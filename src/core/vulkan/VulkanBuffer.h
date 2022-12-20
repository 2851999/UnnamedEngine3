#pragma once

#include "VulkanResource.h"

/*****************************************************************************
 * VulkanBuffer class - Handles a Vulkan buffer instance and its memory
 *                      allocation
 *****************************************************************************/

class VulkanBuffer : VulkanResource {
protected:
    /* Vulkan buffer instance*/
    VkBuffer instance;

private:
    /* Allocated device memory for the buffer */
    VkDeviceMemory memory;

    /* Size of this buffer */
    VkDeviceSize size;

    /* States whether we need staging for copying data */
    bool stagingNeeded;

    /* States whether this buffer should use a persistent mapping (when true
       keeps memory mapped and reuses instead of unmapping each time - best
       for when updating frequently - can only be used if staging not needed
       i.e. for memory that is either not device local or can use resizable
       bar) */
    bool persistentMapping;

    /* Pointer to mapped memory (used if above is true) */
    void* mappedMemory;

    /* Descriptor buffer info - only used for descriptor sets */
    VkDescriptorBufferInfo bufferInfo;

    /* Copies data into some device memory */
    void copy(const void* data, VkDeviceSize size, VkDeviceMemory deviceMemory);

public:
    /* Constructor and destructor (data can be nullptr) */
    VulkanBuffer(VulkanDevice* device, VkDeviceSize size, void* data, VkBufferUsageFlags usage, VkSharingMode sharingMode, bool deviceLocal, bool persistentMapping);
    virtual ~VulkanBuffer();

    /* Copies data into the buffer */
    void copy(const void* data, VkDeviceSize size);

    /* Returns the Vulkan instance of this buffer */
    inline VkBuffer getVkInstance() { return instance; }

    /* Returns descriptor info about this buffer (for descriptor sets) */
    inline const VkDescriptorBufferInfo* getVkDescriptorBufferInfo() { return &bufferInfo; }
};