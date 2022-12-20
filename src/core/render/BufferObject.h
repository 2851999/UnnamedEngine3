#pragma once

#include "../vulkan/VulkanBuffer.h"
#include "RendererResource.h"

/*****************************************************************************
 * BufferObject class - Handles VulkanBuffer instances allowing them to be
 *                      updated while having multiple frames in flight
 *****************************************************************************/

class BufferObject : RendererResource {
private:
    /* The buffers */
    std::vector<VulkanBuffer*> buffers;

    /* Whether this buffer object is updatable */
    bool updatable;

public:
    /* Constructor (data can be nullptr) */
    BufferObject(Renderer* renderer, VkDeviceSize size, void* data, VkBufferUsageFlags usage, VkSharingMode sharingMode, bool deviceLocal, bool persistentMapping, bool updatable);

    /* Destructor */
    virtual ~BufferObject();

    /* Returns the current buffer to use (when updatable will be specific
       to the current frame, otherwise there will only be one buffer) */
    inline VulkanBuffer* getCurrentBuffer() { return buffers[updatable ? renderer->getCurrentFrame() : 0]; }
};