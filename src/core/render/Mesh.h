#pragma once

#include "IndexBuffer.h"
#include "VertexBuffer.h"

/*****************************************************************************
 * Mesh class - Handles the data required for rendering a mesh
 *****************************************************************************/

class Mesh {
private:
    /* Vertex buffers */
    std::vector<VertexBuffer*> vertexBuffers;

    /* Index buffer */
    IndexBuffer* indexBuffer;

    /* Actual instance identifiers for vertex buffer */
    std::vector<VkBuffer> vertexBufferInstances;

    /* Offsets for the vertex buffers */
    std::vector<VkDeviceSize> vertexBufferOffsets;

    /* Vertex/Index count */
    uint32_t count;

public:
    /* Constructor and destructor */
    Mesh(std::vector<VertexBuffer*> vertexBuffers, IndexBuffer* indexBuffer, uint32_t count);
    virtual ~Mesh();

    /* Issues the command to render this mesh */
    void render(VkCommandBuffer commandBuffer);
};