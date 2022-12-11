#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

/*****************************************************************************
 * VulkanSwapChain class - For handling a swap chain
 *****************************************************************************/

class VulkanSwapChain {
public:
    /* Structure used to return support information about the supported
       swap chain capabilities */
    struct Support {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /* Obtains information about the swap chain support of the given device */
    static VulkanSwapChain::Support querySupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);
};