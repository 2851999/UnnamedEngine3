#include "RenderData.h"

/*****************************************************************************
 * RenderData class
 *****************************************************************************/

RenderData::RenderData(std::vector<VBO*> vbos, IBO* ibo, uint32_t count) : vbos(vbos), ibo(ibo), count(count) {
    vertexBufferInstances.resize(vbos.size());
    vertexBufferOffsets.resize(vbos.size());
    for (unsigned int i = 0; i < vertexBufferOffsets.size(); ++i)
        vertexBufferOffsets[i] = 0;
}

RenderData::~RenderData() {
    // Destroy all buffers
    for (VBO* vbo : vbos)
        delete vbo;
    if (ibo)
        delete ibo;
}

void RenderData::render(VkCommandBuffer commandBuffer) {
    // Bind the vertex buffers
    // TODO: Use offsets for materials
    // TODO: Allow instances
    // TODO: Stop doing this here (unless need multiple for each frame in flight)
    VkDeviceSize offsets[] = {0};
    for (unsigned int i = 0; i < vbos.size(); ++i)
        vertexBufferInstances[i] = vbos[i]->getCurrentBuffer()->getVkInstance();

    vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(vertexBufferInstances.size()), vertexBufferInstances.data(), offsets);

    // Check if have indices
    if (ibo) {
        // Bind index buffer and draw
        ibo->bind(commandBuffer);

        vkCmdDrawIndexed(commandBuffer, count, instanceCount, 0, 0, 0);
    } else {
        // Draw
        vkCmdDraw(commandBuffer, count, instanceCount, 0, 0);
    }
}
