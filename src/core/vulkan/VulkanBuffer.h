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

    /* States whether we need staging for copying data */
    bool stagingNeeded;

    /* Copies data into some device memory */
    void copy(const void* data, VkDeviceSize size, VkDeviceMemory deviceMemory);

public:
    /* Constructor and destructor (data can be nullptr)*/
    VulkanBuffer(VulkanDevice* device, VkDeviceSize size, void* data, VkBufferUsageFlags usage, VkSharingMode sharingMode, bool deviceLocal);
    virtual ~VulkanBuffer();

    /* Copies data into the buffer */
    void copy(const void* data, VkDeviceSize size);

    /* Returns the Vulkan instance of this buffer */
    inline VkBuffer getVkInstance() { return instance; }
};