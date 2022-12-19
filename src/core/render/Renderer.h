#pragma once

#include "../vulkan/SwapChain.h"
#include "Framebuffer.h"

/*****************************************************************************
 * Renderer class - Handles rendering with multiple frames in flight, also
 *                  manages the swap chain and its framebuffers
 *****************************************************************************/

class Renderer : SwapChainListener {
private:
    /* Device used for rendering */
    VulkanDevice* device;

    /* Swap chain used for rendering */
    SwapChain* swapChain;

    /* Command buffers used for rendering */
    std::vector<VkCommandBuffer> commandBuffers;

    /* Synchronisation objects */
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    /* Current frame (out of those that can be in flight) */
    unsigned int currentFrame;

    /* Default render pass (Renders directly to swap chain) */
    RenderPass* defaultRenderPass;

    /* Default framebuffers to render to */
    std::vector<Framebuffer*> defaultFramebufers;

public:
    /* Maximum number of frames that can be recorded while others have
       not finished rendering */
    static const unsigned int MAX_FRAMES_IN_FLIGHT = 2;

    /* Constructor */
    Renderer(VulkanDevice* device, Window* window, Settings& settings);

    /* Destructor */
    virtual ~Renderer();

    /* Performs necessary synchronisation, obtains the next swap chain image,
       and begins recording to a command buffer - If the return value is
       true then rendering can begin, otherwise it cannot as the swap chain
       needs recreation */
    bool beginFrame();

    /* Stops recording to the command buffer begun with beginFrame and
       submits and presents the next swap chain image while performing the
       necessary synchronisation - If the return value is false then
       presentation failed and the swap chain needs recreation */
    bool endFrame();

    /* Starts the default render pass */
    void beginDefaultRenderPass();

    /* Ends the default render pass */
    void endDefaultRenderPass();

    /* Called when the swap chain has just been recreated */
    void onSwapChainRecreation(float scaleX, float scaleY) override;

    /* Returns the current command buffer being recorded to (when called
       between begin & endFrame)*/
    inline VkCommandBuffer getCurrentCommandBuffer() { return commandBuffers[currentFrame]; }

    /* Returns other things */
    inline SwapChain* getSwapChain() { return swapChain; }
    inline RenderPass* getDefaultRenderPass() { return defaultRenderPass; }
};