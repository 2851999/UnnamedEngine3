#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/*****************************************************************************
 * utils_vulkan namespace - Contains various Vulkan helper functions
 *****************************************************************************/

namespace utils_vulkan {
    inline VkVertexInputBindingDescription initVertexInputBindings(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate) {
        VkVertexInputBindingDescription description;
        description.binding   = binding;
        description.stride    = stride;
        description.inputRate = inputRate;
        return description;
    }

    inline VkVertexInputAttributeDescription initVertexAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset) {
        VkVertexInputAttributeDescription description;
        description.location = location;
        description.binding  = binding;
        description.format   = format;
        description.offset   = offset;
        return description;
    }
};  // namespace utils_vulkan