#pragma once

#include "VulkanResource.h"

/*****************************************************************************
 * VulkanBuffer class - Handles a Vulkan buffer instance and its memory
 *                      allocation
 *****************************************************************************/

class VulkanBuffer : VulkanResource {
private:
    /* Vulkan buffer instance*/
    VkBuffer instance;

    /* Allocated device memory for the buffer */
    VkDeviceMemory memory;

    /* Size of this buffer */
    VkDeviceSize size;

public:
    /* Constructor and destructor */
    VulkanBuffer(VulkanDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);
    virtual ~VulkanBuffer();

    /* Copies data into the buffer */
    void copy(const void* data, VkDeviceSize size);

    /* Returns the Vulkan instance of this buffer */
    inline VkBuffer getVkInstance() { return instance; }
};