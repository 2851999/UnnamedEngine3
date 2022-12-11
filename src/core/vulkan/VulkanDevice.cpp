#include "VulkanDevice.h"

#include <set>

/*****************************************************************************
 * VulkanDevice class
 *****************************************************************************/

VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface, const VulkanExtensions& extensions) : physicalDevice(physicalDevice) {
    // Obtain the extension support
    this->extensionSupport = extensions.checkPhysicalDeviceSupport(physicalDevice);

    // Obtain the device queue families
    this->queueFamiliyIndices = VulkanDevice::findQueueFamilies(physicalDevice, windowSurface);

    // Need to define the queues to create
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // Likely the same index - want to make sure we only use unique ones
    std::set<uint32_t> uniqueQueueFamilyIndices = {queueFamiliyIndices.graphicsFamily.value()};
    if (windowSurface)
        uniqueQueueFamilyIndices.insert(queueFamiliyIndices.presentFamily.value());

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

    //...
}

VulkanDevice::~VulkanDevice() {
}

int VulkanDevice::rateSuitability(VkPhysicalDevice physicalDevice, const VulkanExtensions& extensions, VkSurfaceKHR windowSurface) {
    // Obtain the device properties
    VkPhysicalDeviceProperties physicalDeviceProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);

    // Supported queue families
    VulkanDevice::QueueFamilyIndices queueFamilyIndices = VulkanDevice::findQueueFamilies(physicalDevice, windowSurface);

    // Supported extensions
    VulkanExtensions::PhysicalDeviceSupport extensionSupport = extensions.checkPhysicalDeviceSupport(physicalDevice);

    // Determine if suitable
    bool suitable = queueFamilyIndices.isComplete(windowSurface != VK_NULL_HANDLE) && extensionSupport.required;

    if (suitable) {
        // Derive a score
        // Strongly favor a discrete GPU
        int score = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 8 : 1;

        // If we have multiple discrete GPU's, rate based on optional extensions
        for (auto const& pair : extensionSupport.optionals) {
            if (pair.second)
                score += 1;
        }

        // Rate based on if its a discrete GPU or not
        return score;
    }

    return 0;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface) {
    // Structure
    VulkanDevice::QueueFamilyIndices queueFamiliyIndices;

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
        if (queueFamiliyIndices.isComplete(windowSurface != VK_NULL_HANDLE))
            break;

        ++i;
    }

    return queueFamiliyIndices;
}