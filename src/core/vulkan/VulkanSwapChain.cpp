#include "VulkanSwapChain.h"

/*****************************************************************************
 * VulkanSwapChain class
 *****************************************************************************/

VulkanSwapChain::Support VulkanSwapChain::querySupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface) {
    // Support details
    VulkanSwapChain::Support support = {};

    // No support if there isn't a window surface
    if (windowSurface != VK_NULL_HANDLE) {
        // Obtain the surface capabilities of the device
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, windowSurface, &support.capabilities);

        // Obtain the supported surface formats
        uint32_t count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &count, nullptr);
        if (count > 0) {
            support.formats.resize(count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, windowSurface, &count, support.formats.data());
        }

        // Obtain the supported present modes
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &count, nullptr);
        if (count > 0) {
            support.presentModes.resize(count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, windowSurface, &count, support.presentModes.data());
        }
    }

    return support;
}