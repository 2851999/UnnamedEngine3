#pragma once

#include "../vulkan/VulkanResizableResource.h"
#include "RenderPass.h"
#include "Shader.h"

/*****************************************************************************
 * GraphicsPipelineLayout class - Handles a graphics pipeline layout
 *****************************************************************************/

class GraphicsPipelineLayout : VulkanResource {
private:
    /* Pipeline layout instance */
    VkPipelineLayout instance;

public:
    /* Constructor and destructor */
    GraphicsPipelineLayout(VulkanDevice* device);
    virtual ~GraphicsPipelineLayout();

    /* Returns the Vulkan instance */
    inline VkPipelineLayout getVkInstance() { return instance; }
};

/*****************************************************************************
 * GraphicsPipeline class - Handles a graphics pipeline
 *****************************************************************************/

class GraphicsPipeline : VulkanResizableResource {
private:
    /* Pipeline instance */
    VkPipeline instance;

    /* Things used to create the pipeline (Needed for recreation) */
    GraphicsPipelineLayout* layout;
    RenderPass* renderPass;
    ShaderGroup* shaderGroup;

    /* Function to create the pipeline */
    void create();

    /* Function to destroy this pipeline */
    void destroy();

public:
    /* Constructor and destructor */
    GraphicsPipeline(GraphicsPipelineLayout* layout, RenderPass* renderPass, ShaderGroup* shaderGroup, SwapChain* swapChain);
    virtual ~GraphicsPipeline() { destroy(); }

    /* Binds this pipeline given the command buffer to record the command to */
    inline void bind(VkCommandBuffer commandBuffer) { vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, instance); }

    /* Called when the swap chain has just been recreated */
    virtual void onSwapChainRecreation() override {
        // Recreate
        destroy();
        create();
    }
};