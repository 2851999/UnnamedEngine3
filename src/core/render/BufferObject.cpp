#include "BufferObject.h"

/*****************************************************************************
 * BufferObject class
 *****************************************************************************/

BufferObject::BufferObject(Renderer* renderer, VkDeviceSize size, void* data, VkBufferUsageFlags usage, VkSharingMode sharingMode, bool deviceLocal, bool persistentMapping, bool updatable) : RendererResource(renderer), updatable(updatable) {
    // If updatable want one per frame in flight to avoid any sync issues
    unsigned int numBuffers = updatable ? Renderer::MAX_FRAMES_IN_FLIGHT : 1;
    buffers.resize(numBuffers);

    for (unsigned int i = 0; i < numBuffers; ++i)
        // Create a buffer
        buffers[i] = new VulkanBuffer(renderer->getDevice(), size, data, usage, sharingMode, deviceLocal, persistentMapping);
}

BufferObject::~BufferObject() {
    // Destroy all buffers
    for (const auto* buffer : buffers)
        delete buffer;
}