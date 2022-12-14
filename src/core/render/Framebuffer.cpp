#include "Framebuffer.h"

/*****************************************************************************
 * Framebuffer class
 *****************************************************************************/

Framebuffer::Framebuffer(RenderPass* renderPass, std::vector<VkImageView> attachments, uint32_t width, uint32_t height, uint32_t layers) : VulkanResource(renderPass->getDevice()) {
    // Framebuffer create info
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass      = renderPass->getVkInstance();
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments    = attachments.data();
    createInfo.width           = width;
    createInfo.height          = height;
    createInfo.layers          = layers;

    // Create
    if (vkCreateFramebuffer(device->getVkLogical(), &createInfo, nullptr, &instance) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create framebuffer", "Framebuffer");
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(device->getVkLogical(), instance, nullptr);
}
