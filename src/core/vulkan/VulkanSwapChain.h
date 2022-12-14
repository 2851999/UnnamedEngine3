#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../Settings.h"

// Forward declarations
class Window;
class VulkanDevice;

/*****************************************************************************
 * VulkanSwapChain class - For handling a swap chain
 *****************************************************************************/

class VulkanSwapChain {
private:
    /* Device used to create this swap chain */
    VulkanDevice* device;

    /* Swap chain instance */
    VkSwapchainKHR instance;

    /* Surface image format */
    VkFormat imageFormat;

    /* Extent */
    VkExtent2D extent;

    /* Images in the swap chain */
    std::vector<VkImage> images;

    /* Image views for the images in the swap chain */
    std::vector<VkImageView> imageViews;

    /* Helper function - returns the corresponding present mode for a given
       VSync setting */
    static inline VkPresentModeKHR vSyncToPresentMode(int vSync) {
        switch (vSync) {
            case 0:
                // ~No buffering
                return VK_PRESENT_MODE_IMMEDIATE_KHR;
            case 1:
                // ~VSync
                return VK_PRESENT_MODE_FIFO_KHR;
            case 2:
                // ~Triple buffering
                return VK_PRESENT_MODE_MAILBOX_KHR;
            case 3:
                return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
            default:
                // Guaranteed to be supported
                return VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    /* Helper function - returns the VSync setting corresponding to  a given
       present mode */
    static inline int presentModeToVSync(VkPresentModeKHR presentMode) {
        switch (presentMode) {
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                // ~No buffering
                return 0;
            case VK_PRESENT_MODE_FIFO_KHR:
                // ~VSync
                return 1;
            case VK_PRESENT_MODE_MAILBOX_KHR:
                // ~Triple buffering
                return 2;
            case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                return 3;
            default:
                return 0;
        }
    }

    /* Helper function that gives a surface format a score based on its value */
    static int scoreSurfaceFormat(const VkSurfaceFormatKHR& surfaceFormat);

    /* Helper function for choosing a swap chain surface format from a list of
       supported formats */
    static VkSurfaceFormatKHR pickSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /* Helper function that returns whether a given present mode is present
       in a given list of modes */
    static bool isPresentModeAvailable(VkPresentModeKHR presentMode, const std::vector<VkPresentModeKHR>& availablePresentModes);

    /* Helper function for choosing a swap chain present mode from a list of
       supported modes - Takes settings to allow preference for current VSync
       setting but may also modify it to what is actually chosen */
    static VkPresentModeKHR pickPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const VideoSettings& videoSettings);

    /* Helper function for choosing a swap extent from the surface capabilities */
    static VkExtent2D pickSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const WindowSettings& windowSettings);

public:
    /* Structure used to return support information about the supported
       swap chain capabilities */
    struct Support {
        // Surface the capabilities were queried for (avoids needing more
        // parameters in the constructor)
        VkSurfaceKHR surface;

        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /* Constructor and destructor - the settings will be modified to reflect
       the actual VSync/video resolution & aspect radio chosen */
    VulkanSwapChain(VulkanDevice* device, Settings& settings);
    virtual ~VulkanSwapChain();

    /* Returns the swap chain's image format & extent */
    inline VkFormat getImageFormat() const { return imageFormat; }
    inline VkExtent2D getExtent() const { return extent; }

    /* Obtains information about the swap chain support of the given device */
    static VulkanSwapChain::Support querySupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface);
};