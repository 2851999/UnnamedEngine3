#include "Mesh.h"

/*****************************************************************************
 * Mesh class
 *****************************************************************************/

Mesh::Mesh(std::vector<VertexBuffer*> vertexBuffers, IndexBuffer* indexBuffer, uint32_t count) : vertexBuffers(vertexBuffers), indexBuffer(indexBuffer), count(count) {
    vertexBufferInstances.resize(vertexBuffers.size());
    vertexBufferOffsets.resize(vertexBuffers.size());
    for (unsigned int i = 0; i < vertexBufferOffsets.size(); ++i)
        vertexBufferOffsets[i] = 0;
}

Mesh::~Mesh() {
    // Destroy all buffers
    for (VertexBuffer* vertexBuffer : vertexBuffers)
        delete vertexBuffer;
    if (indexBuffer)
        delete indexBuffer;
}

void Mesh::render(VkCommandBuffer commandBuffer) {
    // Bind the vertex buffers
    // TODO: Use offsets for materials
    // TODO: Allow instances
    // TODO: Stop doing this here (unless need multiple for each frame in flight)
    VkDeviceSize offsets[] = {0};
    for (unsigned int i = 0; i < vertexBuffers.size(); ++i)
        vertexBufferInstances[i] = vertexBuffers[i]->getVkInstance();

    vkCmdBindVertexBuffers(commandBuffer, 0, vertexBufferInstances.size(), vertexBufferInstances.data(), offsets);

    // Check if have indices
    if (indexBuffer) {
        // Bind index buffer and draw
        indexBuffer->bind(commandBuffer);

        vkCmdDrawIndexed(commandBuffer, count, 1, 0, 0, 0);
    } else {
        // Draw
        vkCmdDraw(commandBuffer, count, 1, 0, 0);
    }
}
