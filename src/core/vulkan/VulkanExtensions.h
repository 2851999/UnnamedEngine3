#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../Settings.h"

/*****************************************************************************
 * VulkanExtensions class - For handling Vulkan extensions
 *****************************************************************************/

class VulkanExtensions {
private:
    /* List of required extensions */
    std::vector<const char*> requiredExtensions;

public:
    /* Constructor and destructor */
    VulkanExtensions() {}
    virtual ~VulkanExtensions() {}

    /* Adds the required extensions to the list given the engine settings
       to use */
    void addRequired(const Settings& settings);

    /* Checks the required extensions are supported by the instance */
    bool checkInstanceSupport();

    /* Returns a reference to the required extension names */
    inline std::vector<const char*>& getRequiredExtensions() { return requiredExtensions; }
};