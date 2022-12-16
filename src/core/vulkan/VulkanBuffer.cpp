#include "VulkanBuffer.h"

/*****************************************************************************
 * VulkanBuffer class
 *****************************************************************************/

VulkanBuffer::VulkanBuffer(VulkanDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode) : VulkanResource(device), size(size) {
    // Create the buffer
    device->createBuffer(size, usage, sharingMode, &instance);

    // Allocate memory
    device->allocateBufferMemory(instance, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory);
}

VulkanBuffer::~VulkanBuffer() {
    device->destroyBuffer(instance);
    device->freeMemory(memory);
}

void VulkanBuffer::copy(const void* data, VkDeviceSize size) {
    // Ensure the size is okay
    if (size > this->size)
        Logger::logAndThrowError("Cannot copy of size " + utils_string::str(size) + " into buffer of smaller size " + utils_string::str(this->size), "VulkanBuffer");
    // Map memory and copy
    // This method requires memory with the flag
    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT to ensure it will finish copying
    // before vkQueueSubmit is called later (otherwise need
    // vkFlushMappedMemoryRanges/vkInvalidateMappedMemoryRanges)
    // TODO: Look at these and other types of memory flags
    void* mappedMemory;
    vkMapMemory(device->getVkLogical(), memory, 0, size, 0, &mappedMemory);
    memcpy(mappedMemory, data, static_cast<size_t>(size));
    vkUnmapMemory(device->getVkLogical(), memory);
}