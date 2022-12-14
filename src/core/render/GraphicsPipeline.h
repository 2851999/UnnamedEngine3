#pragma once

#include "../vulkan/VulkanResource.h"
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

class GraphicsPipeline : VulkanResource {
private:
    /* Pipeline instance */
    VkPipeline instance;

public:
    /* Constructor and destructor */
    GraphicsPipeline(GraphicsPipelineLayout* layout, RenderPass* renderPass, ShaderGroup* shaderGroup, VkExtent2D swapChainExtent);
    virtual ~GraphicsPipeline();
};