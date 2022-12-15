#pragma once

#include "../vulkan/VulkanResource.h"

// Forward declaration
class Framebuffer;

/*****************************************************************************
 * RenderPass class - Handles a render pass
 *****************************************************************************/

class RenderPass : public VulkanResource {
private:
    /* Render pass instance */
    VkRenderPass instance;

    /* Creates this render pass */
    void create(SwapChain* swapChain);

    /* Destroys this render pass */
    void destroy();

public:
    /* Constructor and destructor */
    RenderPass(VulkanDevice* device, SwapChain* swapChain);
    virtual ~RenderPass() { destroy(); }

    /* Recreates this render pass */
    inline void recreate(SwapChain* swapChain) {
        destroy();
        create(swapChain);
    }

    /* Begins/ends this render pass given the command buffer to submit the
       commands to */
    void begin(VkCommandBuffer commandBuffer, Framebuffer* framebuffer, VkExtent2D extent);
    void end(VkCommandBuffer commandBuffer);

    /* Returns the Vulkan instance */
    inline VkRenderPass getVkInstance() const { return instance; }
};