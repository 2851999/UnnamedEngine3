#include "RenderPass.h"

#include "Framebuffer.h"

/*****************************************************************************
 * RenderPass class
 *****************************************************************************/

RenderPass::RenderPass(VulkanDevice* device, VulkanSwapChain* swapChain) : VulkanResource(device) {
    // Attachment description for the colour buffer
    VkAttachmentDescription colourAttachmentDescription{};
    colourAttachmentDescription.format         = swapChain->getImageFormat();
    colourAttachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
    colourAttachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colourAttachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Attachment reference
    VkAttachmentReference colourAttachmentReference{};
    colourAttachmentReference.attachment = 0;
    colourAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass description
    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments    = &colourAttachmentReference;

    // Render pass create info
    VkRenderPassCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments    = &colourAttachmentDescription;
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = &subpassDescription;

    // Create
    if (vkCreateRenderPass(device->getVkLogical(), &createInfo, nullptr, &instance) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create render pass", "RenderPass");
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(device->getVkLogical(), instance, nullptr);
}

void RenderPass::begin(VkCommandBuffer commandBuffer, Framebuffer* framebuffer, VkExtent2D extent) {
    // Render pass begin info
    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass        = instance;
    beginInfo.framebuffer       = framebuffer->getVkInstance();
    beginInfo.renderArea.offset = {0, 0};
    beginInfo.renderArea.extent = extent;

    VkClearValue clearColor   = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues    = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(VkCommandBuffer commandBuffer) {
    vkCmdEndRenderPass(commandBuffer);
}