#include "VulkanDevice.h"

/*****************************************************************************
 * VulkanDevice class
 *****************************************************************************/

int VulkanDevice::rateSuitability(VkPhysicalDevice physicalDevice, const VulkanExtensions& extensions, VkSurfaceKHR windowSurface) {
    // Obtain the device properties
    VkPhysicalDeviceProperties physicalDeviceProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);

    // Supported queue families
    VulkanDevice::QueueFamilyIndices queueFamilyIndices = VulkanDevice::findQueueFamilies(physicalDevice, windowSurface);

    // Determine if suitable
    bool suitable = queueFamilyIndices.isComplete(windowSurface != VK_NULL_HANDLE) && extensions.checkPhysicalDeviceSupport(physicalDevice);

    // Rate based on if its a discrete GPU or not
    if (suitable)
        return physicalDeviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 8 : 1;

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