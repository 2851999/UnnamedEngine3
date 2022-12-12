#include "VulkanSwapChain.h"

#include <algorithm>

#include "../../utils/Logging.h"
#include "../Window.h"
#include "VulkanDevice.h"

/*****************************************************************************
 * VulkanSwapChain class
 *****************************************************************************/

VulkanSwapChain::VulkanSwapChain(VulkanDevice* device, Settings& settings) : logicalDevice(device->getVkLogical()) {
    // Obtain the device's swap chain support
    VulkanSwapChain::Support& swapChainSupport = device->getSwapChainSupport();

    // Choose the surface format, present mode and extent
    VkSurfaceFormatKHR surfaceFormat = VulkanSwapChain::pickSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode     = VulkanSwapChain::pickPresentMode(swapChainSupport.presentModes, settings.video);
    VkExtent2D extent                = VulkanSwapChain::pickSwapExtent(swapChainSupport.capabilities, settings.window);

    // Assign the chosen settings
    settings.video.vSync       = VulkanSwapChain::presentModeToVSync(presentMode);
    settings.video.resolution  = Vector2i(extent.width, extent.height);
    settings.video.aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    // Have to decide number of images to have in the swap chain - Recommended
    // to use one more than minimum so always have another to work on if the
    // driver hasn't finished with the last
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // Ensure this number does not exceed the maximum
    // (0 means there is no maximum)
    if (swapChainSupport.capabilities.maxImageCount > 0)
        imageCount = utils_maths::min(imageCount, swapChainSupport.capabilities.maxImageCount);

    // Swap chain create info
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = swapChainSupport.surface;
    createInfo.minImageCount            = imageCount;
    createInfo.imageFormat              = surfaceFormat.format;
    createInfo.imageColorSpace          = surfaceFormat.colorSpace;
    createInfo.imageExtent              = extent;
    createInfo.imageArrayLayers         = 1;  // 1 unless VR/stereoscopic 3D
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Obtain the queue family indices and determine what sharing mode to use
    VulkanDevice::QueueFamilyIndices& indices = device->getQueueFamilyIndices();
    uint32_t queueFamilyIndices[]             = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        // Use concurrent mode - not as fast but saves changing ownership
        // (most hardware will use the same anyway)
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        // Not needed in exclusive sharing mode
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;  // No transform applied to images in the swap chain
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;               // Ignore alpha channel for compositing with other windows
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;  // Don't care about obscured pixels
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    // Create the swap chain
    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &instance) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create swap chain", "VulkanSwapChain");
}

VulkanSwapChain::~VulkanSwapChain() {
    if (instance)
        vkDestroySwapchainKHR(logicalDevice, instance, nullptr);
}

VulkanSwapChain::Support VulkanSwapChain::querySupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface) {
    // Support details
    VulkanSwapChain::Support support = {};
    support.surface                  = windowSurface;

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

int VulkanSwapChain::scoreSurfaceFormat(const VkSurfaceFormatKHR& surfaceFormat) {
    // Prefer SRGB with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    int score = 0;

    if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        score += 8;
    if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
        score += 8;
    return score;
}

VkSurfaceFormatKHR VulkanSwapChain::pickSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // Return the only available one if we dont have a choice
    if (availableFormats.size() == 1)
        return availableFormats[0];
    else {
        // Want to choose out of the available
        int maxScore                   = 0;
        VkSurfaceFormatKHR currentBest = availableFormats[0];

        // Go through and rank, replacing the best only if it surpasses
        for (const auto& availableFormat : availableFormats) {
            int score = VulkanSwapChain::scoreSurfaceFormat(availableFormat);

            if (score > maxScore) {
                maxScore    = score;
                currentBest = availableFormat;
            }
        }

        if (maxScore == 0)
            Logger::log("Picking a non-preferred swap chain surface format", "VulkanSwapChain", LogType::Debug);

        return currentBest;
    }
}

bool VulkanSwapChain::isPresentModeAvailable(VkPresentModeKHR presentMode, const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == presentMode)
            return true;
    }
    return false;
}

VkPresentModeKHR VulkanSwapChain::pickPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const VideoSettings& videoSettings) {
    // Find the current requested mode
    VkPresentModeKHR requestedMode = vSyncToPresentMode(videoSettings.vSync);

    // Check if requested mode is available
    if (isPresentModeAvailable(requestedMode, availablePresentModes))
        return requestedMode;
    else {
        // Requested mode not available, try and choose next best
        if (requestedMode == VK_PRESENT_MODE_FIFO_KHR)
            // Supposed to be guaranteed
            Logger::log("VK_PRESENT_MODE_FIFO_KHR is not available when it should be guaranteed", "VulkanSwapChain", LogType::Error);
        else if (requestedMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            // Try triple buffering instead (since also doesn't limit frame rate)
            if (isPresentModeAvailable(VK_PRESENT_MODE_MAILBOX_KHR, availablePresentModes)) {
                Logger::log("VK_PRESENT_MODE_IMMEDIATE_KHR is not supported so using VK_PRESENT_MODE_MAILBOX_KHR instead", "VulkanSwapChain", LogType::Debug);
                return VK_PRESENT_MODE_MAILBOX_KHR;
            } else {
                Logger::log("Neither VK_PRESENT_MODE_IMMEDIATE_KHR nor VK_PRESENT_MODE_MAILBOX_KHR is not supported so using VK_PRESENT_MODE_FIFO_KHR instead", "VulkanSwapChain", LogType::Debug);
                return VK_PRESENT_MODE_FIFO_KHR;
            }
        } else if (requestedMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            // Fall back to double buffering
            Logger::log("VK_PRESENT_MODE_MAILBOX_KHR is not supported so using VK_PRESENT_MODE_FIFO_KHR instead", "VulkanSwapChain", LogType::Information);
            return VK_PRESENT_MODE_FIFO_KHR;
        } else if (requestedMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            Logger::log("VK_PRESENT_MODE_FIFO_RELAXED_KHR is not supported so using VK_PRESENT_MODE_FIFO_KHR instead", "VulkanSwapChain", LogType::Debug);
            return VK_PRESENT_MODE_FIFO_KHR;
        }
    }
}

VkExtent2D VulkanSwapChain::pickSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const WindowSettings& windowSettings) {
    // If these values are equal then dont have the value is not given and we
    // must choose the value ourselves
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        // Keep same as surface
        return capabilities.currentExtent;
    else {
        // Make as close to window size as possible
        VkExtent2D actualExtent = {static_cast<uint32_t>(windowSettings.width), static_cast<uint32_t>(windowSettings.height)};

        // Clamp values between maximum and minimum supported
        actualExtent.width  = utils_maths::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = utils_maths::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}