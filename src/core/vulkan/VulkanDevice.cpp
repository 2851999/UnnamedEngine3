#include "VulkanDevice.h"

#include "../../utils/Logging.h"

/*****************************************************************************
 * VulkanDevice class
 *****************************************************************************/

VulkanDevice::VulkanDevice(VulkanDevice::PhysicalDeviceInfo& physicalDeviceInfo) {
    this->physicalDevice = physicalDeviceInfo.device;

    // Obtain the extensions & features and their support
    this->supportedExtensions = physicalDeviceInfo.supportedExtensions;
    this->extensions          = physicalDeviceInfo.extensions;
    this->supportedFeatures   = physicalDeviceInfo.supportedFeatures;
    this->features            = physicalDeviceInfo.features;

    // Obtain the device queue families
    this->queueFamiliyIndices = physicalDeviceInfo.queueFamilyIndices;

    // Need to define the queues to create
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // Obtain the unique queue family indices
    std::set<uint32_t> uniqueQueueFamilyIndices = queueFamiliyIndices.getUniqueRequiredIndices();

    float queuePriority = 1.0f;

    // Assign the queue create infos
    for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndices) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = queueFamilyIndex;
        queueCreateInfo.queueCount              = 1;
        queueCreateInfo.pQueuePriorities        = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Device extensions to enable
    std::vector<const char*> deviceExtensions = this->extensions->getExtensions(this->supportedExtensions);

    // Logical device create info
    VkDeviceCreateInfo createInfo      = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Assign enabled features
    this->features->assignVkDeviceCreateInfo(createInfo, this->supportedFeatures);

    // Device validation layers are the same as instance ones in recent
    // implementations so ignore here
    createInfo.enabledLayerCount = 0;

    // Create the logical device
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create a logical device", "VulkanDevice");

    // Obtain the device queues requested
    vkGetDeviceQueue(logicalDevice, queueFamiliyIndices.graphicsFamily.value(), 0, &graphicsQueue);
    if (queueFamiliyIndices.presentFamily.has_value())
        vkGetDeviceQueue(logicalDevice, queueFamiliyIndices.presentFamily.value(), 0, &presentQueue);
}

VulkanDevice::~VulkanDevice() {
    // Device queues are cleaned up when the device is destroyed
    vkDestroyDevice(logicalDevice, nullptr);

    // Destroy extensions and features
    delete features;
    delete extensions;
}

VulkanDevice::PhysicalDeviceInfo VulkanDevice::queryDeviceInfo(VkPhysicalDevice physicalDevice, VulkanDeviceExtensions* extensions, VulkanFeatures* features, VkSurfaceKHR windowSurface) {
    // Obtain the device properties
    VkPhysicalDeviceProperties physicalDeviceProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);

    // Return the info
    return PhysicalDeviceInfo{
        physicalDevice,
        physicalDeviceProps,
        extensions,
        // Supported extensions
        extensions->querySupport(physicalDevice),
        features,
        // Supported features
        features->querySupport(physicalDevice),
        // Supported queue families
        VulkanDevice::findQueueFamilies(physicalDevice, windowSurface),
    };
}

int VulkanDevice::rateSuitability(PhysicalDeviceInfo& physicalDeviceInfo) {
    // Determine if suitable
    bool suitable = physicalDeviceInfo.queueFamilyIndices.isValid() && physicalDeviceInfo.supportedExtensions.required;

    if (suitable) {
        // Derive a score
        // Strongly favor a discrete GPU
        int score = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 8 : 1;

        // If we have multiple discrete GPU's, rate based on optional extensions
        for (auto const& pair : physicalDeviceInfo.supportedExtensions.optionals) {
            // Ensure the required features are also supported for the same extension (if it exists)
            if (physicalDeviceInfo.supportedFeatures.optionals.find(pair.first) == physicalDeviceInfo.supportedFeatures.optionals.end() || physicalDeviceInfo.supportedFeatures.optionals[pair.first]) {
                if (pair.second)
                    score += 1;
            }
        }

        // Rate based on if its a discrete GPU or not
        return score;
    }

    return 0;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface) {
    // Structure
    VulkanDevice::QueueFamilyIndices queueFamiliyIndices = {};
    queueFamiliyIndices.presentFamilyRequired            = windowSurface != VK_NULL_HANDLE;

    // Obtain the queue families supported by the device
    uint32_t availableQueueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &availableQueueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> availableQueueFamilies(availableQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &availableQueueFamilyCount, availableQueueFamilies.data());

    // Go through each and check queue family support
    uint32_t i = 0;
    // TODO: Prefer families where both graphics and present are supported
    //       simultaneously (most cases they will be the same anyway)
    for (const auto& queueFamily : availableQueueFamilies) {
        // Does this family have presentation support
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamiliyIndices.graphicsFamily = i;

        // Check present queue support if needed
        if (windowSurface != VK_NULL_HANDLE) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, windowSurface, &presentSupport);
            if (presentSupport)
                queueFamiliyIndices.presentFamily = i;
        }

        // Stop if all required families have been found
        if (queueFamiliyIndices.isValid())
            break;

        ++i;
    }

    return queueFamiliyIndices;
}

bool VulkanDevice::isSupported(std::string key) {
    // Look for the name in the extension and features
    bool supportedInExtensions = supportedExtensions.get(key);
    bool supportedInFeatures   = supportedFeatures.get(key);

    // If present in both - both must be true, otherwise just one does
    if (supportedExtensions.has(key) && supportedFeatures.has(key))
        return supportedInExtensions && supportedInFeatures;
    else
        return supportedInExtensions || supportedInFeatures;
}