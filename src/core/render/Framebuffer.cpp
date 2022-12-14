#include "Framebuffer.h"

/*****************************************************************************
 * Framebuffer class
 *****************************************************************************/

Framebuffer::Framebuffer(RenderPass* renderPass, VkExtent2D extent, VkImageView attachment) : VulkanResource(renderPass->getDevice()) {
    // Attachments
    VkImageView attachments[] = {attachment};

    // Framebuffer create info
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass      = renderPass->getVkInstance();
    createInfo.attachmentCount = 1;
    createInfo.pAttachments    = attachments;
    createInfo.width           = extent.width;
    createInfo.height          = extent.height;
    createInfo.layers          = 1;

    // Create
    if (vkCreateFramebuffer(device->getVkLogical(), &createInfo, nullptr, &instance) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create framebuffer", "Framebuffer");
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(device->getVkLogical(), instance, nullptr);
}
