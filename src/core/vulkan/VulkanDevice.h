#pragma once

#include <optional>

#include "../Settings.h"
#include "VulkanExtensions.h"

/*****************************************************************************
 * VulkanDevice class - Handles physical and logical devices and helps during
 *                      the selection a physical device
 *****************************************************************************/

class VulkanDevice {
private:
    /* Stores indices for queue families for a physical device */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        /* Returns whether the all of the indices have been assigned */
        inline bool isComplete(bool requirePresent) { return graphicsFamily.has_value() && (presentFamily.has_value() || ! requirePresent); }
    };

    /* Physical device (Don't need to destroy as handled by instance) */
    VkPhysicalDevice physicalDevice;

    /* Logical device */
    VkDevice logicalDevice;

    /* Extension support of this device */
    VulkanExtensions::PhysicalDeviceSupport extensionSupport;

    /* Queue families used by this device */
    QueueFamilyIndices queueFamiliyIndices;

    /* Returns the queue family indices for a particular physical device - if
       window surface given is not VK_NULL_HANDLE will also look for a present
       queue family */
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);

public:
    /* Constructor and destructor - if window surface given is
       not VK_NULL_HANDLE will also look for a present queue */
    VulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface, const VulkanExtensions& extensions);
    virtual ~VulkanDevice();

    /* Rates the suitability of physical device for the engine - Higher values
       means more suitable, 0 means not suitable - if window surface given is
       not VK_NULL_HANDLE will also look for a present queue  */
    static int rateSuitability(VkPhysicalDevice physicalDevice, const VulkanExtensions& extensions, VkSurfaceKHR windowSurface);
};