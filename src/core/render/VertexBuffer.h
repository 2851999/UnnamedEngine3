#pragma once

#include "BufferObject.h"

/*****************************************************************************
 * VertexBuffer class - Handles a vertex buffer
 *****************************************************************************/

class VertexBuffer : public BufferObject {
public:
    /* Constructor and destructor (data can be nullptr) - Uses
       VK_SHARING_MODE_EXCLUSIVE here as we assume it will only be used in
       the graphics queue family */
    VertexBuffer(Renderer* renderer, VkDeviceSize size, void* data, bool deviceLocal, bool persistentMapping, bool updatable) : BufferObject(renderer, size, data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, deviceLocal, persistentMapping, updatable) {}
    virtual ~VertexBuffer() {}
};