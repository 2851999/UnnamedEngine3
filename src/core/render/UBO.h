#pragma once

#include "BufferObject.h"

/*****************************************************************************
 * UBO class - Handles a uniform buffer object
 *****************************************************************************/

class UBO : public BufferObject, public DescriptorSetResource {
public:
    /* Constructor and destructor (data can be nullptr) - Uses
       VK_SHARING_MODE_EXCLUSIVE here as we assume it will only be used in
       the graphics queue family */
    UBO(Renderer* renderer, VkDeviceSize size, void* data, bool deviceLocal, bool persistentMapping, bool updatable) : BufferObject(renderer, size, data, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, deviceLocal, persistentMapping, updatable) {}
    virtual ~UBO() {}

    /* Should be implemented for use when setting up and updating a descriptor set */
    VkWriteDescriptorSet initWriteDescriptorSet(unsigned int frame, VkDescriptorSet dstSet, uint32_t binding, uint32_t descriptorCount) override {
        VkWriteDescriptorSet writeDescriptor{};
        writeDescriptor.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstSet          = dstSet;
        writeDescriptor.dstBinding      = binding;
        writeDescriptor.dstArrayElement = 0;
        writeDescriptor.descriptorCount = descriptorCount;
        writeDescriptor.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptor.pBufferInfo     = getBuffer(frame)->getVkDescriptorBufferInfo();

        return writeDescriptor;
    }
};