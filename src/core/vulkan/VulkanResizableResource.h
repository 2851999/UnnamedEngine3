#pragma once

#include "SwapChain.h"
#include "VulkanResource.h"

/*****************************************************************************
 * VulkanResizableResource class - Used to handle a Vulkan resource that
 *                                 needs recreation when the swap chain is
 *                                 recreated
 *****************************************************************************/

class VulkanResizableResource : public VulkanResource, SwapChainListener {
protected:
    /* Swap chain instance used for this resource (may be nullptr) */
    SwapChain* swapChain;

public:
    /* Constructor and destructor - when swapChain is not nullptr this is added
       as a recreation listener */
    VulkanResizableResource(VulkanDevice* device, SwapChain* swapChain) : VulkanResource(device), swapChain(swapChain) {
        if (swapChain)
            swapChain->addListener(this);
    }
    virtual ~VulkanResizableResource() {
        if (swapChain)
            swapChain->removeListener(this);
    }

    /* Called when the swap chain has just been recreated */
    virtual void onSwapChainRecreation() override {}
};