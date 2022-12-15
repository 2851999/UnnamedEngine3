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
    uint32_t width;
    uint32_t height;
    GraphicsPipelineLayout* layout;
    RenderPass* renderPass;
    ShaderGroup* shaderGroup;

    /* Function to create the pipeline */
    void create();

    /* Function to destroy this pipeline */
    void destroy();

public:
    /* Constructor and destructor - swapChain can be nullptr if autoscaling
       with swapchain size is not needed  */
    GraphicsPipeline(GraphicsPipelineLayout* layout, RenderPass* renderPass, ShaderGroup* shaderGroup, uint32_t width, uint32_t height, SwapChain* swapChain);
    virtual ~GraphicsPipeline() { destroy(); }

    /* Binds this pipeline given the command buffer to record the command to */
    inline void bind(VkCommandBuffer commandBuffer) { vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, instance); }

    /* Called when the swap chain has just been recreated */
    virtual void onSwapChainRecreation(float scaleX, float scaleY) override {
        // Assume if swap chain was given during creation then wan't autoscaling
        width  = static_cast<uint32_t>(scaleX * static_cast<float>(width));
        height = static_cast<uint32_t>(scaleY * static_cast<float>(height));
        // Recreate
        destroy();
        create();
    }
};