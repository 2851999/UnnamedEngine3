#include "VulkanInstance.h"

#include "../Window.h"
#include "VulkanExtensions.h"
#include "VulkanFeatures.h"

/*****************************************************************************
 * VulkanInstance class
 *****************************************************************************/

bool VulkanInstance::create(const Settings& settings) {
    // Application info
    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = settings.window.title.c_str();
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName        = "Unnamed Engine 3";
    appInfo.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    // Check the instance supports the required extensions
    extensions = new VulkanInstanceExtensions();
    extensions->addExtensions(settings);

    // For checking if instance creation was successful
    bool success = false;

    // Check instance support
    if (extensions->checkSupport()) {
        std::vector<const char*>& requiredExtensions = extensions->getRequiredExtensions();

        // Create info
        VkInstanceCreateInfo createInfo    = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount       = 0;
        createInfo.pNext                   = nullptr;

        // Only used if validation layers are enabled
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

        // Add validation layer info if applicable
        if (settings.debug.validationLayers) {
            validationLayers = new VulkanValidationLayers(extensions);

            if (validationLayers->checkInstanceSupport()) {
                std::vector<const char*>& requestedValidationLayers = validationLayers->getRequested();

                createInfo.enabledLayerCount   = static_cast<uint32_t>(requestedValidationLayers.size());
                createInfo.ppEnabledLayerNames = requestedValidationLayers.data();

                // Also ensure we can debug instance creation and deletion
                validationLayers->assignDebugMessengerCreateInfo(debugCreateInfo);

                createInfo.pNext = &debugCreateInfo;
            }
        }

        // Create the instance and return if successful
        success = vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS;
    }

    // If successful load extensions
    if (success) {
        extensions->loadExtensions(this);

        // Create the debug messenger if needed
        if (settings.debug.validationLayers)
            validationLayers->createDebugMessenger();
    }

    return success;
}

VulkanDevice* VulkanInstance::pickPhysicalDevice(const Settings& settings, const Window* window) {
    // Chosen device
    VulkanDevice::PhysicalDeviceInfo chosenPhysicalDeviceInfo;

    // Device extensions - will be handled by VulkanDevice once created
    VulkanDeviceExtensions* deviceExtensions = new VulkanDeviceExtensions();
    deviceExtensions->addExtensions(settings);

    // Device features - will be handled by VulkanDevice once created
    VulkanFeatures* deviceFeatures = new VulkanFeatures();
    deviceFeatures->addFeatures(settings);

    // Obtain a list of the available devices
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount > 0) {
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

        // Suitability value for current chosen device
        int maxSuitability = 0;

        // Find a suitable device
        for (const auto& physicalDevice : physicalDevices) {
            // Query the device info
            VulkanDevice::PhysicalDeviceInfo currentDeviceInfo = VulkanDevice::queryDeviceInfo(physicalDevice, deviceExtensions, deviceFeatures, window ? window->getVkSurface() : VK_NULL_HANDLE);

            // Check suitability and pick the most suitable
            int currentSuitability = VulkanDevice::rateSuitability(currentDeviceInfo);

            if (currentSuitability > maxSuitability) {
                maxSuitability           = currentSuitability;
                chosenPhysicalDeviceInfo = currentDeviceInfo;
            }
        }

        // Check if any device was found
        if (maxSuitability == 0)
            Logger::logAndThrowError("Failed to find a suitable physical device", "VulkanInstance");
    } else
        Logger::logAndThrowError("Failed to find any physical devices with Vulkan support", "VulkanInstance");

    return new VulkanDevice(chosenPhysicalDeviceInfo);
}

void VulkanInstance::destroy() {
    if (validationLayers)
        delete validationLayers;
    // Extensions must be destroyed after validation layers (as they use them)
    if (extensions)
        delete extensions;
    if (instance)
        vkDestroyInstance(instance, nullptr);
}