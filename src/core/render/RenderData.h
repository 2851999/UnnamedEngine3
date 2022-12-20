#pragma once

#include "IBO.h"
#include "VBO.h"

/*****************************************************************************
 * RenderData class - Handles the data required for rendering some data
 *****************************************************************************/

class RenderData {
private:
    /* Vertex buffers */
    std::vector<VBO*> vbos;

    /* Index buffer */
    IBO* ibo;

    /* Actual instance identifiers for vertex buffer */
    std::vector<VkBuffer> vertexBufferInstances;

    /* Offsets for the vertex buffers */
    std::vector<VkDeviceSize> vertexBufferOffsets;

    /* Vertex/Index count */
    uint32_t count;

    /* Instance count */
    uint32_t instanceCount = 1;

public:
    /* Constructor and destructor */
    RenderData(std::vector<VBO*> vbos, IBO* ibo, uint32_t count);
    virtual ~RenderData();

    /* Issues the command to render this mesh */
    void render(VkCommandBuffer commandBuffer);

    /* Assigns the number of instances to render */
    inline void setInstanceCount(uint32_t instanceCount) { this->instanceCount = instanceCount; }
};