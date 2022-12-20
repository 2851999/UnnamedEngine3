#pragma once

#include "BufferObject.h"

/*****************************************************************************
 * IBO class - Handles an index buffer object
 *****************************************************************************/

class IBO : public BufferObject {
private:
    /* Index type */
    VkIndexType indexType;

public:
    /* Constructor and destructor (data can be nullptr) - Uses
       VK_SHARING_MODE_EXCLUSIVE here as we assume it will only be used in
       the graphics queue family */
    IBO(Renderer* renderer, VkDeviceSize size, void* data, VkIndexType indexType, bool deviceLocal, bool persistentMapping, bool updatable) : BufferObject(renderer, size, data, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, deviceLocal, persistentMapping, updatable), indexType(indexType) {}
    virtual ~IBO() {}

    /* Binds this buffer */
    inline void bind(VkCommandBuffer commandBuffer) { vkCmdBindIndexBuffer(commandBuffer, getCurrentBuffer()->getVkInstance(), 0, indexType); }
};