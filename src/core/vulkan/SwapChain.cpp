#include "SwapChain.h"

#include <algorithm>

#include "../../utils/Logging.h"
#include "../Window.h"
#include "../render/Framebuffer.h"
#include "VulkanDevice.h"

/*****************************************************************************
 * SwapChain class
 *****************************************************************************/

SwapChain::SwapChain(VulkanDevice* device, Window* window, Settings& settings) : device(device), window(window), settings(settings) {
    // Listen for resize events
    window->addResizeListener(this);

    // Create the swap chain
    create();

    // Assign the initial extent size
    lastExtent = extent;
}

SwapChain::~SwapChain() {
    destroy();
}

void SwapChain::create() {
    // Obtain the device's swap chain support
    SwapChain::Support& swapChainSupport = device->getSwapChainSupport();

    // Choose the surface format, present mode and extent
    VkSurfaceFormatKHR surfaceFormat = SwapChain::pickSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode     = SwapChain::pickPresentMode(swapChainSupport.presentModes, settings.video);
    this->extent                     = SwapChain::pickSwapExtent(swapChainSupport.capabilities, settings.window);

    this->imageFormat = surfaceFormat.format;

    // Assign the chosen settings
    settings.video.vSync       = SwapChain::presentModeToVSync(presentMode);
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
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = swapChainSupport.surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;  // 1 unless VR/stereoscopic 3D
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
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
    if (vkCreateSwapchainKHR(device->getVkLogical(), &createInfo, nullptr, &instance) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create swap chain", "SwapChain");

    // Obtain the swap chain images - may have more than requested as can only
    // specify the minimum number
    vkGetSwapchainImagesKHR(device->getVkLogical(), instance, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device->getVkLogical(), instance, &imageCount, images.data());

    // Create the image views
    imageViews.resize(imageCount);

    // Create image views
    for (unsigned int i = 0; i < imageCount; ++i)
        imageViews[i] = device->createImageView(images[i], VK_IMAGE_VIEW_TYPE_2D, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1);
}

bool SwapChain::acquireNextImage(VkSemaphore semaphore, VkFence fence) {
    // Acquire the next swap chain image
    VkResult result = vkAcquireNextImageKHR(device->getVkLogical(), instance, UINT64_MAX, semaphore, fence, &imageIndex);

    // Check for an out of date swap chain
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        recreate();
        // Should stop rendering this frame - swap chain is being recreated
        return false;
    } else if (result != VK_SUCCESS)
        Logger::logAndThrowError("Failed to acquire swap chain image", "SwapChain");
    return true;
}

bool SwapChain::presentImage(uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = waitSemaphoreCount;
    presentInfo.pWaitSemaphores    = pWaitSemaphores;

    VkSwapchainKHR swapChains[] = {instance};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapChains;
    presentInfo.pImageIndices   = &imageIndex;

    VkResult result = vkQueuePresentKHR(device->getVkPresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        recreate();
        // Should stop rendering this frame - swap chain is being recreated
        return false;
    } else if (result != VK_SUCCESS)
        Logger::logAndThrowError("Failed to present image from queue " + utils_string::str(result), "SwapChain");
    return true;
}

void SwapChain::recreate() {
    // Pause if window is minimised
    int width, height;
    glfwGetFramebufferSize(window->getInstance(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window->getInstance(), &width, &height);
        glfwWaitEvents();
    }

    // Update the window size
    settings.window.width  = static_cast<unsigned int>(width);
    settings.window.height = static_cast<unsigned int>(height);

    // Ensure the device has finished what ever it is doing before destroying
    // resources
    device->waitIdle();

    // Re-query swap chain support
    device->requerySwapChainSupport(window->getVkSurface());

    // Now recreate the swap chain
    destroy();
    create();

    // Work out change in size
    float scaleX = static_cast<float>(extent.width) / static_cast<float>(lastExtent.width);
    float scaleY = static_cast<float>(extent.height) / static_cast<float>(lastExtent.height);

    lastExtent = extent;

    // Trigger recreation events for anything else that needs recreating
    callOnSwapChainRecreation(scaleX, scaleY);

    // Reset in case this triggered it
    framebufferResized = false;
}

void SwapChain::destroy() {
    // Destroy image views
    for (const auto& imageView : imageViews)
        device->destroyImageView(imageView);
    // Destroy instance
    if (instance)
        vkDestroySwapchainKHR(device->getVkLogical(), instance, nullptr);
}

std::vector<Framebuffer*> SwapChain::createFramebuffers(RenderPass* renderPass) {
    // Create one for each swap chain image
    std::vector<Framebuffer*> framebuffers(imageViews.size());
    for (unsigned int i = 0; i < framebuffers.size(); ++i)
        framebuffers[i] = new Framebuffer(renderPass, {imageViews[i]}, extent.width, extent.height, 1);

    return framebuffers;
}

SwapChain::Support SwapChain::querySupport(VkPhysicalDevice device, VkSurfaceKHR windowSurface) {
    // Support details
    SwapChain::Support support{};
    support.surface = windowSurface;

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

int SwapChain::scoreSurfaceFormat(const VkSurfaceFormatKHR& surfaceFormat) {
    // Prefer SRGB with VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    int score = 0;

    if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        score += 8;
    if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB)
        score += 8;
    return score;
}

VkSurfaceFormatKHR SwapChain::pickSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    // Return the only available one if we dont have a choice
    if (availableFormats.size() == 1)
        return availableFormats[0];
    else {
        // Want to choose out of the available
        int maxScore                   = 0;
        VkSurfaceFormatKHR currentBest = availableFormats[0];

        // Go through and rank, replacing the best only if it surpasses
        for (const auto& availableFormat : availableFormats) {
            int score = SwapChain::scoreSurfaceFormat(availableFormat);

            if (score > maxScore) {
                maxScore    = score;
                currentBest = availableFormat;
            }
        }

        if (maxScore == 0)
            Logger::log("Picking a non-preferred swap chain surface format", "SwapChain", LogType::Debug);

        return currentBest;
    }
}

bool SwapChain::isPresentModeAvailable(VkPresentModeKHR presentMode, const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == presentMode)
            return true;
    }
    return false;
}

VkPresentModeKHR SwapChain::pickPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, const VideoSettings& videoSettings) {
    // Find the current requested mode
    VkPresentModeKHR requestedMode = vSyncToPresentMode(videoSettings.vSync);

    // Check if requested mode is available
    if (isPresentModeAvailable(requestedMode, availablePresentModes))
        return requestedMode;
    else {
        // Requested mode not available, try and choose next best
        if (requestedMode == VK_PRESENT_MODE_FIFO_KHR)
            // Supposed to be guaranteed
            Logger::log("VK_PRESENT_MODE_FIFO_KHR is not available when it should be guaranteed", "SwapChain", LogType::Error);
        else if (requestedMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            // Try triple buffering instead (since also doesn't limit frame rate)
            if (isPresentModeAvailable(VK_PRESENT_MODE_MAILBOX_KHR, availablePresentModes)) {
                Logger::log("VK_PRESENT_MODE_IMMEDIATE_KHR is not supported so using VK_PRESENT_MODE_MAILBOX_KHR instead", "SwapChain", LogType::Debug);
                return VK_PRESENT_MODE_MAILBOX_KHR;
            } else {
                Logger::log("Neither VK_PRESENT_MODE_IMMEDIATE_KHR nor VK_PRESENT_MODE_MAILBOX_KHR is not supported so using VK_PRESENT_MODE_FIFO_KHR instead", "SwapChain", LogType::Debug);
                return VK_PRESENT_MODE_FIFO_KHR;
            }
        } else if (requestedMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            // Fall back to double buffering
            Logger::log("VK_PRESENT_MODE_MAILBOX_KHR is not supported so using VK_PRESENT_MODE_FIFO_KHR instead", "SwapChain", LogType::Information);
            return VK_PRESENT_MODE_FIFO_KHR;
        } else if (requestedMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
            Logger::log("VK_PRESENT_MODE_FIFO_RELAXED_KHR is not supported so using VK_PRESENT_MODE_FIFO_KHR instead", "SwapChain", LogType::Debug);
            return VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::pickSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const WindowSettings& windowSettings) {
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