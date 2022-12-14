#pragma once

#include "VulkanDevice.h"

/*****************************************************************************
 * VulkanResource class - Used to handle a Vulkan resource (that requires a
 *                        device to create and destroy)
 *****************************************************************************/

class VulkanResource {
protected:
    VulkanDevice* device;

public:
    /* Constructor and destructor */
    VulkanResource(VulkanDevice* device) : device(device) {}
    virtual ~VulkanResource() {}

    /* Returns the device this resource was created using */
    VulkanDevice* getDevice() { return device; }
};