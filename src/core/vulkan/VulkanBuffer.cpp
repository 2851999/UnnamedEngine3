#include "VulkanBuffer.h"

/*****************************************************************************
 * VulkanBuffer class
 *****************************************************************************/

VulkanBuffer::VulkanBuffer(VulkanDevice* device, VkDeviceSize size, void* data, VkBufferUsageFlags usage, VkSharingMode sharingMode, bool deviceLocal) : VulkanResource(device), size(size) {
    // Create the buffer
    // TODO: Try and get rid of need for VK_BUFFER_USAGE_TRANSFER_DST_BIT -
    //       trouble is cant be sure of supported memoryTypeBits until
    //       obtaining memory requirements for the buffer
    device->createBuffer(size, deviceLocal ? usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT : usage, sharingMode, &instance);

    // Allocate memory

    // Memory should either be
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    // or
    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    // In the case of resizable bar/smart memory access whole of device memory
    // can be all of the above, so no staging will be necessary
    VkMemoryPropertyFlags chosenFlags = device->allocateBufferMemoryResizableBar(instance, memory, deviceLocal);

    // Check if staging needed (only for when want device local, but not
    // host visible and coherent)
    this->stagingNeeded = deviceLocal && chosenFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // Copy data if given
    if (data)
        copy(data, size);
}

VulkanBuffer::~VulkanBuffer() {
    device->destroyBuffer(instance);
    device->freeMemory(memory);
}

void VulkanBuffer::copy(const void* data, VkDeviceSize size, VkDeviceMemory deviceMemory) {
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
    vkMapMemory(device->getVkLogical(), deviceMemory, 0, size, 0, &mappedMemory);
    memcpy(mappedMemory, data, static_cast<size_t>(size));
    vkUnmapMemory(device->getVkLogical(), deviceMemory);
}

void VulkanBuffer::copy(const void* data, VkDeviceSize size) {
    // Ensure the size is okay
    if (size > this->size)
        Logger::logAndThrowError("Cannot copy of size " + utils_string::str(size) + " into buffer of smaller size " + utils_string::str(this->size), "VulkanBuffer");

    // Check for staging
    if (stagingNeeded) {
        // Create a staging buffer
        VkBuffer stagingBuffer;
        device->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, &stagingBuffer);
        VkDeviceMemory stagingBufferMemory;
        device->allocateBufferMemory(stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory);

        // Copy data into the staging buffer
        copy(data, size, stagingBufferMemory);

        // Copy dta to the actual buffer being used
        device->copyBuffer(stagingBuffer, instance, size);

        // Free staging resources
        device->destroyBuffer(stagingBuffer);
        device->freeMemory(stagingBufferMemory);
    } else
        // Copy directly
        copy(data, size, memory);
}