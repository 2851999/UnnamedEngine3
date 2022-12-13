#pragma once

#include "../vulkan/VulkanResource.h"

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
    GraphicsPipeline(VulkanDevice* device, VkExtent2D swapChainExtent);
    virtual ~GraphicsPipeline();
};