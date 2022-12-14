#pragma once

#include "RenderPass.h"

/*****************************************************************************
 * Framebuffer class - Handles a framebuffer
 *****************************************************************************/

class Framebuffer : VulkanResource {
private:
    /* Instance */
    VkFramebuffer instance;

public:
    /* Constructor and destructor */
    Framebuffer(RenderPass* renderPass, std::vector<VkImageView> attachments, uint32_t width, uint32_t height, uint32_t layers);
    virtual ~Framebuffer();

    /* Returns the Vulkan instance */
    inline VkFramebuffer getVkInstance() const { return instance; }
};