#pragma once

#include "../vulkan/VulkanResource.h"

/*****************************************************************************
 * DescriptorSetLayout class - For handling a descriptor set layout
 *****************************************************************************/

class DescriptorSetLayout : VulkanResource {
public:
    /* Structure for storing binding information for this layout */
    struct BindingInfo {
        /* Binding location - same location needed in shaders */
        uint32_t binding;
        /* Type of descriptor */
        VkDescriptorType descriptorType;
        /* Number of descriptors in this binding (e.g. if an array of textures
           will be > 1)*/
        uint32_t descriptorCount;
        /* Stage flags - specifies which pipeline shader stages can access a
           resource */
        VkShaderStageFlags stageFlags;
    };

private:
    /* Vulkan instance */
    VkDescriptorSetLayout instance = VK_NULL_HANDLE;

    /* Bindings within this layout */
    std::vector<BindingInfo> bindingInfos;

public:
    /* Constructor and destructor - layout not created until create
       called to give time to add any bindings needed */
    DescriptorSetLayout(VulkanDevice* device) : VulkanResource(device) {}
    virtual ~DescriptorSetLayout();

    /* Creates the descriptor set layout with the added bindings */
    void create();

    /* Methods to add various bindings to this layout before creation */
    inline void addBinding(BindingInfo bindingInfo) { bindingInfos.push_back(bindingInfo); }
    inline void addBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags) { bindingInfos.push_back({binding, descriptorType, descriptorCount, stageFlags}); }
    inline void addUBO(uint32_t binding, VkShaderStageFlags stageFlags) { addBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, stageFlags); }

    /* Returns the Vulkan instance */
    VkDescriptorSetLayout getVkLayout() const {
        return instance;
    }
};