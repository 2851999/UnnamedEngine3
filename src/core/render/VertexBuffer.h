#pragma once

#include "../vulkan/VulkanBuffer.h"

/*****************************************************************************
 * VertexBuffer class - Handles a vertex buffer
 *****************************************************************************/

class VertexBuffer : public VulkanBuffer {
public:
    /* Constructor and destructor (data can be nullptr) - Uses
       VK_SHARING_MODE_EXCLUSIVE here as we assume it will only be used in
       the graphics queue family */
    VertexBuffer(VulkanDevice* device, VkDeviceSize size, void* data, bool deviceLocal) : VulkanBuffer(device, size, data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, deviceLocal) {}
    virtual ~VertexBuffer() {}
};