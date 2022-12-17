#pragma once

#include "../vulkan/VulkanBuffer.h"

/*****************************************************************************
 * IndexBuffer class - Handles an IndexBuffer
 *****************************************************************************/

class IndexBuffer : public VulkanBuffer {
private:
    /* Index type */
    VkIndexType indexType;

public:
    /* Constructor and destructor (data can be nullptr) - Uses
       VK_SHARING_MODE_EXCLUSIVE here as we assume it will only be used in
       the graphics queue family */
    IndexBuffer(VulkanDevice* device, VkDeviceSize size, void* data, VkIndexType indexType, bool deviceLocal) : VulkanBuffer(device, size, data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, deviceLocal), indexType(indexType) {}
    virtual ~IndexBuffer() {}

    /* Binds this buffer */
    inline void bind(VkCommandBuffer commandBuffer) { vkCmdBindIndexBuffer(commandBuffer, instance, 0, indexType); }
};