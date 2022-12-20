#pragma once

#include "BufferObject.h"

/*****************************************************************************
 * VBO class - Handles a vertex buffer object
 *****************************************************************************/

class VBO : public BufferObject {
public:
    /* Constructor and destructor (data can be nullptr) - Uses
       VK_SHARING_MODE_EXCLUSIVE here as we assume it will only be used in
       the graphics queue family */
    VBO(Renderer* renderer, VkDeviceSize size, void* data, bool deviceLocal, bool persistentMapping, bool updatable) : BufferObject(renderer, size, data, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, deviceLocal, persistentMapping, updatable) {}
    virtual ~VBO() {}
};