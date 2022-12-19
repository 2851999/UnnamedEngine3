#pragma once

#include <optional>
#include <set>

#include "../../utils/Logging.h"
#include "../Settings.h"
#include "SwapChain.h"
#include "VulkanExtensions.h"
#include "VulkanFeatures.h"

/*****************************************************************************
 * VulkanDevice class - Handles physical and logical devices and helps during
 *                      the selection a physical device
 *****************************************************************************/

class VulkanDevice {
public:
    /* Stores indices for queue families for a physical device */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        /* States whether the present family is actually required */
        bool presentFamilyRequired;

        /* Returns whether the all of the required indices have been assigned */
        inline bool isValid() { return graphicsFamily.has_value() && (presentFamily.has_value() || ! presentFamilyRequired); }

        /* Returns an std::set of the required queue families */
        inline std::set<uint32_t> getUniqueRequiredIndices() {
            // Likely the same index - want to make sure we only use unique ones
            std::set<uint32_t> uniqueQueueFamilyIndices = {graphicsFamily.value()};

            // Only add present family if needed
            if (presentFamily.has_value())
                uniqueQueueFamilyIndices.insert(presentFamily.value());

            return uniqueQueueFamilyIndices;
        }
    };

private:
    /* Physical device (Don't need to destroy as handled by instance) */
    VkPhysicalDevice physicalDevice;

    /* Logical device */
    VkDevice logicalDevice;

    /* Extension and feature support of this device */
    VulkanExtensions::Support supportedExtensions;
    VulkanFeatures::Support supportedFeatures;

    /* Extensions & features of this device */
    VulkanDeviceExtensions* extensions;
    VulkanFeatures* features;

    /* Swap chain support of this device */
    SwapChain::Support swapChainSupport;

    /* Queue families used by this device */
    QueueFamilyIndices queueFamiliyIndices;

    /* Obtained queues (if assigned) */
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue  = VK_NULL_HANDLE;

    /* Graphics command pool */
    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;

    /* Structure for returning found memory index and its heap */
    struct FoundMemoryType {
        uint32_t index;
        uint32_t heapIndex;
    };

    /* Looks for a specific memory type and returns its index */
    FoundMemoryType findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags propertyFlags);

    /* Returns the queue family indices for a particular physical device - if
       window surface given is not VK_NULL_HANDLE will also look for a present
       queue family */
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface);

public:
    /* Stores info about a physical device - helps with creation and allows us
       to avoid querying extensions and queue families more than once */
    struct PhysicalDeviceInfo {
        /* Vulkan pointer */
        VkPhysicalDevice device;

        /* Device properties */
        VkPhysicalDeviceProperties properties;

        /* Extensions
           Note: Once a VulkanDevice is created, it should have complete ownership
                 of this - it should not be deleted elsewhere */
        VulkanDeviceExtensions* extensions;

        /* Physical device extension support */
        VulkanExtensions::Support supportedExtensions;

        /* Extensions
           Note: Once a VulkanDevice is created, it should have complete ownership
                 of this - it should not be deleted elsewhere */
        VulkanFeatures* features;

        /* Physical device feature support */
        VulkanFeatures::Support supportedFeatures;

        /* Queue family indices */
        QueueFamilyIndices queueFamilyIndices;

        /* Swap chain support of the device */
        SwapChain::Support swapChainSupport;
    };

    /* Constructor and destructor (validationLayers may be nullptr) */
    VulkanDevice(PhysicalDeviceInfo& physicalDeviceInfo);
    virtual ~VulkanDevice();

    /* Returns whether a set of extensions/features is supported given the
       key */
    bool isSupported(std::string key);

    /* Lists the limits of this device - for debugging purposes */
    std::string listLimits();

    /* Various methods to create resources using this device */
    inline void createImageView(VkImage image, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layerCount, VkImageView* pImageView) {
        // Create info
        VkImageViewCreateInfo createInfo         = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        createInfo.image                         = image;
        createInfo.viewType                      = viewType;
        createInfo.format                        = format;
        createInfo.subresourceRange.aspectMask   = aspectMask;
        createInfo.subresourceRange.levelCount   = mipLevels;
        createInfo.subresourceRange.baseMipLevel = baseMipLevel;
        createInfo.subresourceRange.layerCount   = layerCount;

        // Attempt creation
        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, pImageView) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to create image view", "VulkanDevice");
    }

    inline void createShaderModule(const std::vector<char>& code, VkShaderModule* pShaderModule) {
        // Create info
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

        // Attempt creation
        if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, pShaderModule) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to create shader module", "VulkanDevice");
    }

    inline void createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags, VkCommandPool* pCommandPool) {
        // Create info
        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        createInfo.flags            = flags;

        // Attempt creation
        if (vkCreateCommandPool(logicalDevice, &createInfo, nullptr, pCommandPool) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to create command pool", "VulkanDevice");
    }

    inline void createGraphicsCommandBuffers(VkCommandBufferLevel level, uint32_t commandBufferCount, VkCommandBuffer* pCommandBuffers) {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool                 = graphicsCommandPool;
        allocInfo.level                       = level;
        allocInfo.commandBufferCount          = commandBufferCount;

        if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, pCommandBuffers) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to allocate command buffers", "VulkanDevice");
    }

    inline void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkBuffer* pBuffer) {
        // Create info
        VkBufferCreateInfo createInfo{};
        createInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size        = size;
        createInfo.usage       = usage;
        createInfo.sharingMode = sharingMode;

        // Attempt creation
        if (vkCreateBuffer(logicalDevice, &createInfo, nullptr, pBuffer) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to create buffer", "VulkanDevice");
    }

    inline void createDescriptorSetLayout(uint32_t bindingCount, const VkDescriptorSetLayoutBinding* pBindings, VkDescriptorSetLayout* pSetLayout) {
        // Create info
        VkDescriptorSetLayoutCreateInfo createInfo{};
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext        = nullptr;
        createInfo.flags        = 0;
        createInfo.bindingCount = bindingCount;
        createInfo.pBindings    = pBindings;

        // Attempt creation
        if (vkCreateDescriptorSetLayout(logicalDevice, &createInfo, nullptr, pSetLayout) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to create descriptor set layout", "VulkanDevice");
    }

    /* Various methods to destroy resources using this device */
    inline void destroyImageView(VkImageView imageView) {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }

    inline void destroyShaderModule(VkShaderModule shaderModule) {
        vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
    }

    inline void destroyCommandPool(VkCommandPool commandPool) {
        vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    }

    inline void destroyBuffer(VkBuffer buffer) {
        vkDestroyBuffer(logicalDevice, buffer, nullptr);
    }

    inline void destroyDescriptorSetLayout(VkDescriptorSetLayout desctriptorSetLayout) {
        vkDestroyDescriptorSetLayout(logicalDevice, desctriptorSetLayout, nullptr);
    }

    /* Allocates some device memory for a buffer - also binds its use to the
       given buffer */
    inline void allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags propertyFlags, VkDeviceMemory& memory) {
        // Obtain the buffer's memory requirements
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, buffer, &memoryRequirements);

        // Memory allocation info
        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;

        FoundMemoryType chosenMemoryType = findMemoryType(memoryRequirements.memoryTypeBits, propertyFlags);

        memoryAllocateInfo.memoryTypeIndex = chosenMemoryType.index;

        // Attempt allocation
        if (vkAllocateMemory(logicalDevice, &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS)
            Logger::logAndThrowError("Failed to allocate buffer memory", "VulkanDevice");

        // Associate memory with the buffer
        vkBindBufferMemory(logicalDevice, buffer, memory, 0);
    }

    /* Allocates some device memory for a buffer - When deviceLocal is true
       will return memory with the property flags
       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT or VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
       if both are found in the same heap (indicating resizable bar/smart
       access memory is supported)
       When false it instead returns
       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT or VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
       under the same conditions
    */
    VkMemoryPropertyFlags allocateBufferMemoryResizableBar(VkBuffer buffer, VkDeviceMemory& memory, bool deviceLocal);

    /* Frees some allocated memory */
    inline void freeMemory(VkDeviceMemory memory) {
        vkFreeMemory(logicalDevice, memory, nullptr);
    }

    /* Waits until this device finishes whatever it's doing */
    inline void waitIdle() {
        vkDeviceWaitIdle(logicalDevice);
    }

    /* Allocates and returns a command buffer for recording a set of commands
       that will be submitted to the graphics queue immediately after
       finishing via endSingleTimeGraphicsCommands */
    VkCommandBuffer beginSingleTimeGraphicsCommands();

    /* Stops recording a command buffer allocated using
       beginSingleTimeGraphicsCommands and submits it to the graphics queue
       before waiting for the queue to become idle */
    void endSingleTimeGraphicsCommands(VkCommandBuffer commandBuffer);

    /* Uses the graphics queue to copy one buffer into another */
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /* Re-queries swap chain support and updates the support info */
    inline void requerySwapChainSupport(VkSurfaceKHR windowSurface) {
        this->swapChainSupport = SwapChain::querySupport(physicalDevice, windowSurface);
    }

    /* Returns the swap chain support */
    inline SwapChain::Support& getSwapChainSupport() { return swapChainSupport; }

    /* Returns the Vulkan handles */
    inline VkPhysicalDevice& getVkPhysical() { return physicalDevice; }
    inline VkDevice& getVkLogical() { return logicalDevice; }
    inline VkCommandPool& getVkGraphicsCommandPool() { return graphicsCommandPool; }

    /* Returns the queue indices/queues */
    inline QueueFamilyIndices& getQueueFamilyIndices() { return queueFamiliyIndices; }
    inline VkQueue& getVkGraphicsQueue() { return graphicsQueue; }
    inline VkQueue& getVkPresentQueue() { return presentQueue; }

    /* Obtains device info given a physical device instance */
    static PhysicalDeviceInfo queryDeviceInfo(VkPhysicalDevice physicalDevice, VulkanDeviceExtensions* extensions, VulkanFeatures* features, VkSurfaceKHR windowSurface);

    /* Rates the suitability of physical device for the engine - Higher values
       means more suitable, 0 means not suitable - if window surface given is
       not VK_NULL_HANDLE will also look for a present queue  */
    static int rateSuitability(PhysicalDeviceInfo& physicalDeviceInfo);
};