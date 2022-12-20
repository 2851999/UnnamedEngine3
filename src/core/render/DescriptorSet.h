#pragma once

#include "../vulkan/VulkanResource.h"
#include "RendererResource.h"

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
    inline VkDescriptorSetLayout getVkInstance() const { return instance; }

    /* Returns the binding infos */
    inline std::vector<BindingInfo>& getBindingInfos() { return bindingInfos; }
};

/*****************************************************************************
 * DescriptorSetResource class - Base class for resources that can be added
 *                               to a descriptor set
 *****************************************************************************/

class DescriptorSetResource {
public:
    /* Should be implemented for use when setting up and updating a descriptor
       set */
    virtual VkWriteDescriptorSet initWriteDescriptorSet(unsigned int frame, VkDescriptorSet dstSet, uint32_t binding, uint32_t descriptorCount) = 0;
};

/*****************************************************************************
 * DescriptorSet class - Handles a descriptor set
 *****************************************************************************/

class DescriptorSet : RendererResource {
private:
    /* Layout of this descriptor set */
    DescriptorSetLayout* layout;

    /* Descriptor pool */
    // TODO: Move this out of here
    VkDescriptorPool pool;

    /* Descriptor set instances (may have one per maximum frame in flight) */
    std::vector<VkDescriptorSet> instances;

    /* States whether this descriptor set is updatable (i.e. has one set per
       maximum frame in flight) */
    bool updatable;

    /* Resource in this descriptor set */
    std::vector<DescriptorSetResource*> resources;

public:
    /* Constructor */
    DescriptorSet(Renderer* renderer, DescriptorSetLayout* layout, bool updatable);

    /* Destructor */
    virtual ~DescriptorSet();

    /* Set's up this descriptor set with the resources it describes */
    void setup(std::vector<DescriptorSetResource*> resources);

    /* Updates this descriptor set only for the current frame */
    void updateCurrentFrame();

    /* Binds this descriptor set using a given command buffer */
    // TODO: Simplify this?
    inline void bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout pipelineLayout, uint32_t firstSet) { vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, pipelineLayout, firstSet, 1, updatable ? &instances[renderer->getCurrentFrame()] : &instances[0], 0, nullptr); }
};
