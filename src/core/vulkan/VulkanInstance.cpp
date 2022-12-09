#include "VulkanInstance.h"

#include "VulkanExtensions.h"

/*****************************************************************************
 * VulkanInstance class
 *****************************************************************************/

bool VulkanInstance::create(const Settings& settings) {
    // Application info
    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = settings.window.title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Unnamed Engine 3";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    // Check the instance supports the required extensions
    extensions.addRequired(settings);

    // Check instance support
    if (extensions.checkInstanceSupport()) {
        std::vector<const char*>& requiredExtensions = extensions.getRequiredExtensions();

        // Create info
        VkInstanceCreateInfo createInfo    = {};
        createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo        = &appInfo;
        createInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        // Add validation layer info if applicable
        if (settings.debug.validationLayers) {
            std::vector<const char*>& requestedValidationLayers = validationLayers.getRequested();

            createInfo.enabledLayerCount   = static_cast<uint32_t>(requestedValidationLayers.size());
            createInfo.ppEnabledLayerNames = requiredExtensions.data();
        } else
            createInfo.enabledLayerCount = 0;

        // Create the instance and return if successful
        return vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS;
    }

    return false;
}

void VulkanInstance::destroy() {
    if (instance)
        vkDestroyInstance(instance, nullptr);
}