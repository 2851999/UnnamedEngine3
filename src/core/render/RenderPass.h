#pragma once

#include "../vulkan/VulkanResource.h"

/*****************************************************************************
 * RenderPass class - Handles a render pass
 *****************************************************************************/

class RenderPass : VulkanResource {
private:
    /* Render pass instance */
    VkRenderPass instance;

public:
    /* Constructor and destructor */
    RenderPass(VulkanDevice* device, VulkanSwapChain* swapChain);
    virtual ~RenderPass();
};