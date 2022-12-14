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
    Framebuffer(RenderPass* renderPass, VkExtent2D extent, VkImageView attachment);
    virtual ~Framebuffer();
};