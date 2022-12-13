#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <map>
#include <vector>

#include "../Settings.h"

/*****************************************************************************
 * VulkanFeatures class - For handling Vulkan device features
 *****************************************************************************/

class VulkanFeatures {
private:
    /* Various structures defining required device features */
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    VkPhysicalDeviceVulkan11Features deviceVulkan11Features{};
    VkPhysicalDeviceBufferDeviceAddressFeatures deviceBufferDeviceAddressFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR deviceRayTracingPipelineFeaturesKHR{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR deviceAccelerationStructureFeaturesKHR{};
    VkPhysicalDeviceShaderClockFeaturesKHR deviceShaderClockFeaturesKHR{};
    VkPhysicalDeviceDescriptorIndexingFeatures deviceDescriptorIndexingFeatures{};
    VkPhysicalDeviceHostQueryResetFeatures deviceHostQueryResetFeatures{};

    /* Selected features */
    std::vector<void*> selectedFeatures;

    /* Helper for setting up the pNext parameters of the structs */
    void* setupPNext(std::vector<void*>& selectedFeatures) const;

public:
    /* Structure used to return support information about supported
       features */
    struct Support {
        /* States whether the required extensions are supported */
        bool required = false;

        /* Map of optional extensions and whether they are supported by the
           device - will only be added here if they have been requested e.g.
           if ray tracing is disabled - it wont be present */
        std::map<std::string, bool> optionals{};

        /* Returns whether the optional features has a particular key */
        inline bool has(std::string key) {
            return optionals.find(key) != optionals.end();
        }

        /* Returns if a particular optional feature is present and supported */
        inline bool get(std::string key) {
            return optionals.find(key) != optionals.end() && optionals[key];
        }
    };

    /* Names for optional extensions */
    static const std::string RAY_TRACING;

    /* States whether ray tracing features are required */
    bool rayTracing = false;

    /* Constructor and destructor */
    VulkanFeatures() {}
    virtual ~VulkanFeatures() {}

    /* Adds the required features given the engine settings to use */
    void addFeatures(const Settings& settings);

    /* Checks the required/optional features are supported by the given device
       (returns false regardless for features that are not needed) */
    VulkanFeatures::Support querySupport(VkPhysicalDevice device) const;

    /* Assigns the appropriate parts of a VkDeviceCreateInfo to enable the
       features described in this class - will assign the pNext for
       VkPhysicalDeviceFeatures2 if needed*/
    void assignVkDeviceCreateInfo(VkDeviceCreateInfo& createInfo, VulkanFeatures::Support& supportedFeatures);
};