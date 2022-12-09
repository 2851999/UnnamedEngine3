#pragma once

#include "../Settings.h"
#include "VulkanExtensions.h"
#include "VulkanValidationLayers.h"

/*****************************************************************************
 * VulkanInstance class - For handling a Vulkan instance
 *****************************************************************************/

class VulkanInstance {
private:
    /* The actual instance*/
    VkInstance instance = nullptr;

    /* VulkanExtensions instance */
    VulkanExtensions extensions;

    /* VulkanValidationLayers instance */
    VulkanValidationLayers validationLayers;

public:
    /* Constructor and destructor */
    VulkanInstance() {}
    virtual ~VulkanInstance() { destroy(); }

    /* Attempts to create the VkInstance and returns whether successful */
    bool create(const Settings& settings);

    /* Destroys the VkInstance */
    void destroy();
};