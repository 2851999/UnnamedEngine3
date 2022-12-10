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

        /* Returns whether the all of the indices have been assigned */
        inline bool isComplete() { return graphicsFamily.has_value(); }
    };

    /* Rates the suitability of physical device for the engine - Higher values
       means more suitable, 0 means not suitable */
    static int rateSuitability(VkPhysicalDevice physicalDevice, const VulkanExtensions& extensions);

    /* Returns the queue family indices for a particular physical device */
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
};