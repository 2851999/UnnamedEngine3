#pragma once

#include <optional>

#include "../Settings.h"
#include "VulkanExtensions.h"

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

        /* Returns whether the all of the indices have been assigned */
        inline bool isComplete(bool requirePresent) { return graphicsFamily.has_value() && (presentFamily.has_value() || ! requirePresent); }
    };

    /* Rates the suitability of physical device for the engine - Higher values
       means more suitable, 0 means not suitable - if window surface given is
       not VK_NULL_HANDLE will also look for a present queue  */
    static int rateSuitability(VkPhysicalDevice physicalDevice, const VulkanExtensions& extensions, VkSurfaceKHR windowSurface);

    /* Returns the queue family indices for a particular physical device - if
       window surface given is not VK_NULL_HANDLE will also look for a present
       queue family */
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);
};