#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

/*****************************************************************************
 * VulkanValidationLayers class - For handling Vulkan validation layers
 *****************************************************************************/

class VulkanValidationLayers {
private:
    /* Stores the requested validation layers */
    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

public:
    /* Constructor and destructor */
    VulkanValidationLayers() {}
    virtual ~VulkanValidationLayers() {}

    /* Returns a reference the requested validation layer names */
    inline std::vector<const char*>& getRequested() { return validationLayers; }
};