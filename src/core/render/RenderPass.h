#pragma once

#include "../vulkan/VulkanResource.h"

/*****************************************************************************
 * RenderPass class - Handles a render pass
 *****************************************************************************/

class RenderPass : public VulkanResource {
private:
    /* Render pass instance */
    VkRenderPass instance;

public:
    /* Constructor and destructor */
    RenderPass(VulkanDevice* device, VulkanSwapChain* swapChain);
    virtual ~RenderPass();

    /* Returns the Vulkan instance */
    inline VkRenderPass getVkInstance() const { return instance; }
};