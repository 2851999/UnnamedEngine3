#include "VulkanDevice.h"

/*****************************************************************************
 * VulkanDevice class
 *****************************************************************************/

int VulkanDevice::rateSuitability(VkPhysicalDevice physicalDevice, const VulkanExtensions& extensions) {
    // Obtain the device properties
    VkPhysicalDeviceProperties physicalDeviceProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);

    // Supported queue families
    VulkanDevice::QueueFamilyIndices queueFamilyIndices = VulkanDevice::findQueueFamilies(physicalDevice);

    // Determine if suitable
    bool suitable = queueFamilyIndices.isComplete() && extensions.checkPhysicalDeviceSupport(physicalDevice);

    // Rate based on if its a discrete GPU or not
    if (suitable)
        return physicalDeviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 8 : 1;

    return 0;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice physicalDevice) {
    // Structure
    VulkanDevice::QueueFamilyIndices queueFamiliyIndices;

    // Obtain the queue families supported by the device
    uint32_t availableQueueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &availableQueueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> availableQueueFamilies(availableQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &availableQueueFamilyCount, availableQueueFamilies.data());

    // Go through each and check queue family support
    uint32_t i = 0;
    for (const auto& queueFamily : availableQueueFamilies) {
        // Does this family have presentation support
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamiliyIndices.graphicsFamily = i;
        ++i;
    }

    return queueFamiliyIndices;
}