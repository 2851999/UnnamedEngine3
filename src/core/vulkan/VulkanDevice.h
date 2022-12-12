#pragma once

#include <optional>
#include <set>

#include "../../utils/Logging.h"
#include "../Settings.h"
#include "VulkanExtensions.h"
#include "VulkanFeatures.h"
#include "VulkanSwapChain.h"

/*****************************************************************************
 * VulkanDevice class - Handles physical and logical devices and helps during
 *                      the selection a physical device
 *****************************************************************************/

class VulkanDevice {
public:
    /* Stores indices for queue families for a physical device */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        /* States whether the present family is actually required */
        bool presentFamilyRequired;

        /* Returns whether the all of the required indices have been assigned */
        inline bool isValid() { return graphicsFamily.has_value() && (presentFamily.has_value() || ! presentFamilyRequired); }

        /* Returns an std::set of the required queue families */
        inline std::set<uint32_t> getUniqueRequiredIndices() {
            // Likely the same index - want to make sure we only use unique ones
            std::set<uint32_t> uniqueQueueFamilyIndices = {graphicsFamily.value()};

            // Only add present family if needed
            if (presentFamily.has_value())
                uniqueQueueFamilyIndices.insert(presentFamily.value());

            return uniqueQueueFamilyIndices;
        }
    };

private:
    /* Physical device (Don't need to destroy as handled by instance) */
    VkPhysicalDevice physicalDevice;

    /* Logical device */
    VkDevice logicalDevice;

    /* Extension and feature support of this device */
    VulkanExtensions::Support supportedExtensions;
    VulkanFeatures::Support supportedFeatures;

    /* Extensions & features of this device */
    VulkanDeviceExtensions* extensions;
    VulkanFeatures* features;

    /* Swap chain support of this device */
    VulkanSwapChain::Support swapChainSupport;

    /* Queue families used by this device */
    QueueFamilyIndices queueFamiliyIndices;

    /* Obtained queues (if assigned) */
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue  = VK_NULL_HANDLE;

    /* Returns the queue family indices for a particular physical device - if
       window surface given is not VK_NULL_HANDLE will also look for a present
       queue family */
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);

public:
    /* Stores info about a physical device - helps with creation and allows us
       to avoid querying extensions and queue families more than once */
    struct PhysicalDeviceInfo {
        /* Vulkan pointer */
        VkPhysicalDevice device;

        /* Device properties */
        VkPhysicalDeviceProperties properties;

        /* Extensions
           Note: Once a VulkanDevice is created, it should have complete ownership
                 of this - it should not be deleted elsewhere */
        VulkanDeviceExtensions* extensions;

        /* Physical device extension support */
        VulkanExtensions::Support supportedExtensions;

        /* Extensions
           Note: Once a VulkanDevice is created, it should have complete ownership
                 of this - it should not be deleted elsewhere */
        VulkanFeatures* features;

        /* Physical device feature support */
        VulkanFeatures::Support supportedFeatures;

        /* Queue family indices */
        QueueFamilyIndices queueFamilyIndices;

        /* Swap chain support of the device */
        VulkanSwapChain::Support swapChainSupport;
    };

    /* Constructor and destructor (validationLayers may be nullptr) */
    VulkanDevice(PhysicalDeviceInfo& physicalDeviceInfo);
    virtual ~VulkanDevice();

    /* Returns whether a set of extensions/features is supported given the
       key */
    bool isSupported(std::string key);

    /* Lists the limits of this device - for debugging purposes */
    std::string listLimits();

    /* Various methods to create resources using this device */
    inline VkImageView createImageView(VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layerCount) {
        // Create info
        VkImageViewCreateInfo createInfo         = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        createInfo.image                         = image;
        createInfo.viewType                      = viewType;
        createInfo.format                        = format;
        createInfo.subresourceRange.aspectMask   = aspectMask;
        createInfo.subresourceRange.levelCount   = mipLevels;
        createInfo.subresourceRange.baseMipLevel = baseMipLevel;
        createInfo.subresourceRange.layerCount   = layerCount;

        // Attempt creation
        VkImageView imageView;
        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to create image view", "VulkanDevice");

        return imageView;
    }

    /* Various methods to destroy resources using this device */
    inline void destroyImageView(VkImageView imageView) {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }

    /* Returns the swap chain support */
    inline VulkanSwapChain::Support& getSwapChainSupport() { return swapChainSupport; }

    /* Returns the Vulkan device handles */
    inline VkPhysicalDevice& getVkPhysical() { return physicalDevice; }
    inline VkDevice& getVkLogical() { return logicalDevice; }

    /* Returns the queue indices/queues */
    inline QueueFamilyIndices& getQueueFamilyIndices() { return queueFamiliyIndices; }
    inline VkQueue& getVkGraphicsQueue() { return graphicsQueue; }
    inline VkQueue& getVkPresentQueue() { return presentQueue; }

    /* Obtains device info given a physical device instance */
    static PhysicalDeviceInfo queryDeviceInfo(VkPhysicalDevice physicalDevice, VulkanDeviceExtensions* extensions, VulkanFeatures* features, VkSurfaceKHR windowSurface);

    /* Rates the suitability of physical device for the engine - Higher values
       means more suitable, 0 means not suitable - if window surface given is
       not VK_NULL_HANDLE will also look for a present queue  */
    static int rateSuitability(PhysicalDeviceInfo& physicalDeviceInfo);
};