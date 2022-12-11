#pragma once

#include <optional>
#include <set>

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

    /* Extension support of this device */
    VulkanExtensions::Support supportedExtensions;

    /* the extensions of this device */
    VulkanDeviceExtensions* extensions;

    /* Queue families used by this device */
    QueueFamilyIndices queueFamiliyIndices;

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

        /* Physical device support */
        VulkanExtensions::Support supportedExtensions;

        /* Queue family indices */
        QueueFamilyIndices queueFamilyIndices;
    };

    /* Constructor and destructor */
    VulkanDevice(PhysicalDeviceInfo& physicalDeviceInfo);
    virtual ~VulkanDevice();

    /* Obtains device info given a physical device instance */
    static PhysicalDeviceInfo queryDeviceInfo(VkPhysicalDevice physicalDevice, VulkanDeviceExtensions* extensions, VkSurfaceKHR windowSurface);

    /* Rates the suitability of physical device for the engine - Higher values
       means more suitable, 0 means not suitable - if window surface given is
       not VK_NULL_HANDLE will also look for a present queue  */
    static int rateSuitability(PhysicalDeviceInfo& physicalDeviceInfo);
};