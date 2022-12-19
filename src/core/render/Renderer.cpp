#include "Renderer.h"

#include "../vulkan/VulkanDevice.h"
#include "RenderPass.h"

/*****************************************************************************
 * Renderer class
 *****************************************************************************/

Renderer::Renderer(VulkanDevice* device, Window* window, Settings& settings) : device(device) {
    // Create the swap chain
    swapChain = new SwapChain(device, window, settings);

    // Listen for recreation events
    swapChain->addListener(this);

    // Create command buffers
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    device->createGraphicsCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    // Create synchronisation objects
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Start signalled as default is not and would cause 'vkWaitForFences called for fence 0x11 which has not been submitted on a Queue or during acquire next image'

    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(device->getVkLogical(), &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device->getVkLogical(), &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device->getVkLogical(), &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            Logger::logAndThrowError("Failed to create synchronisation objects for a frame", "BaseEngine");
        }
    }

    // Current frame
    currentFrame = 0;

    // Create the default render pass
    defaultRenderPass = new RenderPass(device, swapChain);

    // Obtain framebuffers for rendering directly to the swap chain images
    defaultFramebufers = swapChain->createFramebuffers(defaultRenderPass);
}

Renderer::~Renderer() {
    // Default render pass
    delete defaultRenderPass;

    // Destroy synchronisation objects
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device->getVkLogical(), imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device->getVkLogical(), renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device->getVkLogical(), inFlightFences[i], nullptr);
    }

    // Destroy default framebuffers and swap chain
    for (unsigned int i = 0; i < defaultFramebufers.size(); ++i)
        delete defaultFramebufers[i];
    delete swapChain;
}

bool Renderer::beginFrame() {
    // Wait for all fences (VK_TRUE)
    vkWaitForFences(device->getVkLogical(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire the next swap chain image (don't render if recreating swap chain)
    if (! swapChain->acquireNextImage(imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE))
        return false;

    // Only reset if actually submitting work
    vkResetFences(device->getVkLogical(), 1, &inFlightFences[currentFrame]);  // Unlike semaphores have to reset after use

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    // Begin recording to command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to start recording to command buffer", "BaseEngine");

    return true;
}

bool Renderer::endFrame() {
    // Stop recording to command buffer
    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to stop recording to command buffer", "BaseEngine");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[]      = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount     = 1;
    submitInfo.pWaitSemaphores        = waitSemaphores;
    submitInfo.pWaitDstStageMask      = waitStages;
    submitInfo.commandBufferCount     = 1;
    submitInfo.pCommandBuffers        = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[]  = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    // Fence added here to be signalled when command buffer finishes executing
    if (vkQueueSubmit(device->getVkGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to submit draw command buffer", "BaseEngine");

    // Present the next image in the swap chain
    if (! swapChain->presentImage(1, signalSemaphores))
        return false;

    // vkAcquireNextImageKHR semaphore signalled will be the one with this index (so must increase before it is called again)
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return true;
}

void Renderer::beginDefaultRenderPass() {
    defaultRenderPass->begin(commandBuffers[currentFrame], defaultFramebufers[swapChain->getCurrentImageIndex()], swapChain->getExtent());
}

void Renderer::endDefaultRenderPass() {
    defaultRenderPass->end(commandBuffers[currentFrame]);
}

void Renderer::onSwapChainRecreation(float scaleX, float scaleY) {
    // Delete all the framebuffers as need to recreate
    for (unsigned int i = 0; i < defaultFramebufers.size(); ++i)
        delete defaultFramebufers[i];

    // Render pass only needs destroying due to format changing e.g. going from non-hdr to hdr monitor
    defaultRenderPass->recreate(swapChain);
    defaultFramebufers = swapChain->createFramebuffers(defaultRenderPass);
}