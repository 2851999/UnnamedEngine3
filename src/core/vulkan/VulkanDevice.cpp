#include "VulkanDevice.h"

#include "SwapChain.h"

/*****************************************************************************
 * VulkanDevice class
 *****************************************************************************/

VulkanDevice::VulkanDevice(VulkanDevice::PhysicalDeviceInfo& physicalDeviceInfo) {
    this->physicalDevice = physicalDeviceInfo.device;

    // Obtain the extensions & features and their support
    this->supportedExtensions = physicalDeviceInfo.supportedExtensions;
    this->extensions          = physicalDeviceInfo.extensions;
    this->supportedFeatures   = physicalDeviceInfo.supportedFeatures;
    this->features            = physicalDeviceInfo.features;
    this->swapChainSupport    = physicalDeviceInfo.swapChainSupport;

    // Obtain the device queue families
    this->queueFamiliyIndices = physicalDeviceInfo.queueFamilyIndices;

    // Need to define the queues to create
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    // Obtain the unique queue family indices
    std::set<uint32_t> uniqueQueueFamilyIndices = queueFamiliyIndices.getUniqueRequiredIndices();

    float queuePriority = 1.0f;

    // Assign the queue create infos
    for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndices) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Device extensions to enable
    std::vector<const char*> deviceExtensions = this->extensions->getExtensions(this->supportedExtensions);

    // Logical device create info
    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Assign enabled features
    this->features->assignVkDeviceCreateInfo(createInfo, this->supportedFeatures);

    // Device validation layers are the same as instance ones in recent
    // implementations so ignore here
    createInfo.enabledLayerCount = 0;

    // Create the logical device
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
        Logger::logAndThrowError("Failed to create a logical device", "VulkanDevice");

    // Obtain the device queues requested
    vkGetDeviceQueue(logicalDevice, queueFamiliyIndices.graphicsFamily.value(), 0, &graphicsQueue);
    if (queueFamiliyIndices.presentFamily.has_value())
        vkGetDeviceQueue(logicalDevice, queueFamiliyIndices.presentFamily.value(), 0, &presentQueue);
}

VulkanDevice::~VulkanDevice() {
    // Device queues are cleaned up when the device is destroyed
    vkDestroyDevice(logicalDevice, nullptr);

    // Destroy extensions and features
    delete features;
    delete extensions;
}

VulkanDevice::PhysicalDeviceInfo VulkanDevice::queryDeviceInfo(VkPhysicalDevice physicalDevice, VulkanDeviceExtensions* extensions, VulkanFeatures* features, VkSurfaceKHR windowSurface) {
    // Obtain the device properties
    VkPhysicalDeviceProperties physicalDeviceProps;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProps);

    // Return the info
    return PhysicalDeviceInfo{
        physicalDevice,
        physicalDeviceProps,
        extensions,
        // Supported extensions
        extensions->querySupport(physicalDevice),
        features,
        // Supported features
        features->querySupport(physicalDevice),
        // Supported queue families
        VulkanDevice::findQueueFamilies(physicalDevice, windowSurface),
        // Swap chain support
        SwapChain::querySupport(physicalDevice, windowSurface),
    };
}

int VulkanDevice::rateSuitability(PhysicalDeviceInfo& physicalDeviceInfo) {
    // Determine if suitable
    bool suitable = physicalDeviceInfo.queueFamilyIndices.isValid() && physicalDeviceInfo.supportedExtensions.required;

    // Check swap chain support (if currently suitable and doesn't require a
    // present family then assume we don't need it)
    if (suitable) {
        if (physicalDeviceInfo.queueFamilyIndices.presentFamilyRequired)
            suitable = ! physicalDeviceInfo.swapChainSupport.formats.empty() && ! physicalDeviceInfo.swapChainSupport.presentModes.empty();

        // Derive a score
        // Strongly favor a discrete GPU
        int score = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 8 : 1;

        // If we have multiple discrete GPU's, rate based on optional extensions
        for (auto const& pair : physicalDeviceInfo.supportedExtensions.optionals) {
            // Ensure the required features are also supported for the same extension (if it exists)
            if (physicalDeviceInfo.supportedFeatures.optionals.find(pair.first) == physicalDeviceInfo.supportedFeatures.optionals.end() || physicalDeviceInfo.supportedFeatures.optionals[pair.first]) {
                if (pair.second)
                    score += 1;
            }
        }

        // Rate based on if its a discrete GPU or not
        return score;
    }

    return 0;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR windowSurface) {
    // Structure
    VulkanDevice::QueueFamilyIndices queueFamiliyIndices{};
    queueFamiliyIndices.presentFamilyRequired = windowSurface != VK_NULL_HANDLE;

    // Obtain the queue families supported by the device
    uint32_t availableQueueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &availableQueueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> availableQueueFamilies(availableQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &availableQueueFamilyCount, availableQueueFamilies.data());

    // Go through each and check queue family support
    uint32_t i = 0;
    // TODO: Prefer families where both graphics and present are supported
    //       simultaneously (most cases they will be the same anyway)
    for (const auto& queueFamily : availableQueueFamilies) {
        // Does this family have presentation support
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamiliyIndices.graphicsFamily = i;

        // Check present queue support if needed
        if (windowSurface != VK_NULL_HANDLE) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, windowSurface, &presentSupport);
            if (presentSupport)
                queueFamiliyIndices.presentFamily = i;
        }

        // Stop if all required families have been found
        if (queueFamiliyIndices.isValid())
            break;

        ++i;
    }

    return queueFamiliyIndices;
}

bool VulkanDevice::isSupported(std::string key) {
    // Look for the name in the extension and features
    bool supportedInExtensions = supportedExtensions.get(key);
    bool supportedInFeatures   = supportedFeatures.get(key);

    // If present in both - both must be true, otherwise just one does
    if (supportedExtensions.has(key) && supportedFeatures.has(key))
        return supportedInExtensions && supportedInFeatures;
    else
        return supportedInExtensions || supportedInFeatures;
}

std::string VulkanDevice::listLimits() {
    // Obtain the device properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // Obtain all of the properties and return them as a string
    return "maxImageDimension1D: " + utils_string::str(deviceProperties.limits.maxImageDimension1D) + "\n" +
           "maxImageDimension2D: " + utils_string::str(deviceProperties.limits.maxImageDimension2D) + "\n" +
           "maxImageDimension3D: " + utils_string::str(deviceProperties.limits.maxImageDimension3D) + "\n" +
           "maxImageDimensionCube: " + utils_string::str(deviceProperties.limits.maxImageDimensionCube) + "\n" +
           "maxImageArrayLayers: " + utils_string::str(deviceProperties.limits.maxImageArrayLayers) + "\n" +
           "maxTexelBufferElements: " + utils_string::str(deviceProperties.limits.maxTexelBufferElements) + "\n" +
           "maxUniformBufferRange: " + utils_string::str(deviceProperties.limits.maxUniformBufferRange) + "\n" +
           "maxStorageBufferRange: " + utils_string::str(deviceProperties.limits.maxStorageBufferRange) + "\n" +
           "maxPushConstantsSize: " + utils_string::str(deviceProperties.limits.maxPushConstantsSize) + "\n" +
           "maxMemoryAllocationCount: " + utils_string::str(deviceProperties.limits.maxMemoryAllocationCount) + "\n" +
           "maxSamplerAllocationCount: " + utils_string::str(deviceProperties.limits.maxSamplerAllocationCount) + "\n" +
           "bufferImageGranularity: " + utils_string::str(deviceProperties.limits.bufferImageGranularity) + "\n" +
           "sparseAddressSpaceSize: " + utils_string::str(deviceProperties.limits.sparseAddressSpaceSize) + "\n" +
           "maxBoundDescriptorSets: " + utils_string::str(deviceProperties.limits.maxBoundDescriptorSets) + "\n" +
           "maxPerStageDescriptorSamplers: " + utils_string::str(deviceProperties.limits.maxPerStageDescriptorSamplers) + "\n" +
           "maxPerStageDescriptorUniformBuffers: " + utils_string::str(deviceProperties.limits.maxPerStageDescriptorUniformBuffers) + "\n" +
           "maxPerStageDescriptorStorageBuffers: " + utils_string::str(deviceProperties.limits.maxPerStageDescriptorStorageBuffers) + "\n" +
           "maxPerStageDescriptorSampledImages: " + utils_string::str(deviceProperties.limits.maxPerStageDescriptorSampledImages) + "\n" +
           "maxPerStageDescriptorStorageImages: " + utils_string::str(deviceProperties.limits.maxPerStageDescriptorStorageImages) + "\n" +
           "maxPerStageDescriptorInputAttachments: " + utils_string::str(deviceProperties.limits.maxPerStageDescriptorInputAttachments) + "\n" +
           "maxPerStageResources: " + utils_string::str(deviceProperties.limits.maxPerStageResources) + "\n" +
           "maxDescriptorSetSamplers: " + utils_string::str(deviceProperties.limits.maxDescriptorSetSamplers) + "\n" +
           "maxDescriptorSetUniformBuffers: " + utils_string::str(deviceProperties.limits.maxDescriptorSetUniformBuffers) + "\n" +
           "maxDescriptorSetUniformBuffersDynamic: " + utils_string::str(deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic) + "\n" +
           "maxDescriptorSetStorageBuffers: " + utils_string::str(deviceProperties.limits.maxDescriptorSetStorageBuffers) + "\n" +
           "maxDescriptorSetStorageBuffersDynamic: " + utils_string::str(deviceProperties.limits.maxDescriptorSetStorageBuffersDynamic) + "\n" +
           "maxDescriptorSetSampledImages: " + utils_string::str(deviceProperties.limits.maxDescriptorSetSampledImages) + "\n" +
           "maxDescriptorSetStorageImages: " + utils_string::str(deviceProperties.limits.maxDescriptorSetStorageImages) + "\n" +
           "maxDescriptorSetInputAttachments: " + utils_string::str(deviceProperties.limits.maxDescriptorSetInputAttachments) + "\n" +
           "maxVertexInputAttributes: " + utils_string::str(deviceProperties.limits.maxVertexInputAttributes) + "\n" +
           "maxVertexInputBindings: " + utils_string::str(deviceProperties.limits.maxVertexInputBindings) + "\n" +
           "maxVertexInputAttributeOffset: " + utils_string::str(deviceProperties.limits.maxVertexInputAttributeOffset) + "\n" +
           "maxVertexInputBindingStride: " + utils_string::str(deviceProperties.limits.maxVertexInputBindingStride) + "\n" +
           "maxVertexOutputComponents: " + utils_string::str(deviceProperties.limits.maxVertexOutputComponents) + "\n" +
           "maxTessellationGenerationLevel: " + utils_string::str(deviceProperties.limits.maxTessellationGenerationLevel) + "\n" +
           "maxTessellationPatchSize: " + utils_string::str(deviceProperties.limits.maxTessellationPatchSize) + "\n" +
           "maxTessellationControlPerVertexInputComponents: " + utils_string::str(deviceProperties.limits.maxTessellationControlPerVertexInputComponents) + "\n" +
           "maxTessellationControlPerVertexOutputComponents: " + utils_string::str(deviceProperties.limits.maxTessellationControlPerVertexOutputComponents) + "\n" +
           "maxTessellationControlPerPatchOutputComponents: " + utils_string::str(deviceProperties.limits.maxTessellationControlPerPatchOutputComponents) + "\n" +
           "maxTessellationControlTotalOutputComponents: " + utils_string::str(deviceProperties.limits.maxTessellationControlTotalOutputComponents) + "\n" +
           "maxTessellationEvaluationInputComponents: " + utils_string::str(deviceProperties.limits.maxTessellationEvaluationInputComponents) + "\n" +
           "maxTessellationEvaluationOutputComponents: " + utils_string::str(deviceProperties.limits.maxTessellationEvaluationOutputComponents) + "\n" +
           "maxGeometryShaderInvocations: " + utils_string::str(deviceProperties.limits.maxGeometryShaderInvocations) + "\n" +
           "maxGeometryInputComponents: " + utils_string::str(deviceProperties.limits.maxGeometryInputComponents) + "\n" +
           "maxGeometryOutputComponents: " + utils_string::str(deviceProperties.limits.maxGeometryOutputComponents) + "\n" +
           "maxGeometryOutputVertices: " + utils_string::str(deviceProperties.limits.maxGeometryOutputVertices) + "\n" +
           "maxGeometryTotalOutputComponents: " + utils_string::str(deviceProperties.limits.maxGeometryTotalOutputComponents) + "\n" +
           "maxFragmentInputComponents: " + utils_string::str(deviceProperties.limits.maxFragmentInputComponents) + "\n" +
           "maxFragmentOutputAttachments: " + utils_string::str(deviceProperties.limits.maxFragmentOutputAttachments) + "\n" +
           "maxFragmentDualSrcAttachments: " + utils_string::str(deviceProperties.limits.maxFragmentDualSrcAttachments) + "\n" +
           "maxFragmentCombinedOutputResources: " + utils_string::str(deviceProperties.limits.maxFragmentCombinedOutputResources) + "\n" +
           "maxComputeSharedMemorySize: " + utils_string::str(deviceProperties.limits.maxComputeSharedMemorySize) + "\n" +
           "maxComputeWorkGroupCount[0]: " + utils_string::str(deviceProperties.limits.maxComputeWorkGroupCount[0]) + "\n" +
           "maxComputeWorkGroupCount[1]: " + utils_string::str(deviceProperties.limits.maxComputeWorkGroupCount[1]) + "\n" +
           "maxComputeWorkGroupCount[2]: " + utils_string::str(deviceProperties.limits.maxComputeWorkGroupCount[2]) + "\n" +
           "maxComputeWorkGroupInvocations: " + utils_string::str(deviceProperties.limits.maxComputeWorkGroupInvocations) + "\n" +
           "maxComputeWorkGroupSize[0]: " + utils_string::str(deviceProperties.limits.maxComputeWorkGroupSize[0]) + "\n" +
           "maxComputeWorkGroupSize[1]: " + utils_string::str(deviceProperties.limits.maxComputeWorkGroupSize[1]) + "\n" +
           "maxComputeWorkGroupSize[2]: " + utils_string::str(deviceProperties.limits.maxComputeWorkGroupSize[2]) + "\n" +
           "subPixelPrecisionBits: " + utils_string::str(deviceProperties.limits.subPixelPrecisionBits) + "\n" +
           "subTexelPrecisionBits: " + utils_string::str(deviceProperties.limits.subTexelPrecisionBits) + "\n" +
           "mipmapPrecisionBits: " + utils_string::str(deviceProperties.limits.mipmapPrecisionBits) + "\n" +
           "maxDrawIndexedIndexValue: " + utils_string::str(deviceProperties.limits.maxDrawIndexedIndexValue) + "\n" +
           "maxDrawIndirectCount: " + utils_string::str(deviceProperties.limits.maxDrawIndirectCount) + "\n" +
           "maxSamplerLodBias: " + utils_string::str(deviceProperties.limits.maxSamplerLodBias) + "\n" +
           "maxSamplerAnisotropy: " + utils_string::str(deviceProperties.limits.maxSamplerAnisotropy) + "\n" +
           "maxViewports: " + utils_string::str(deviceProperties.limits.maxViewports) + "\n" +
           "maxViewportDimensions[0]: " + utils_string::str(deviceProperties.limits.maxViewportDimensions[0]) + "\n" +
           "maxViewportDimensions[1]: " + utils_string::str(deviceProperties.limits.maxViewportDimensions[1]) + "\n" +
           "viewportBoundsRange[0]: " + utils_string::str(deviceProperties.limits.viewportBoundsRange[0]) + "\n" +
           "viewportBoundsRange[1]: " + utils_string::str(deviceProperties.limits.viewportBoundsRange[1]) + "\n" +
           "viewportSubPixelBits: " + utils_string::str(deviceProperties.limits.viewportSubPixelBits) + "\n" +
           "minMemoryMapAlignment: " + utils_string::str(deviceProperties.limits.minMemoryMapAlignment) + "\n" +
           "minTexelBufferOffsetAlignment: " + utils_string::str(deviceProperties.limits.minTexelBufferOffsetAlignment) + "\n" +
           "minUniformBufferOffsetAlignment: " + utils_string::str(deviceProperties.limits.minUniformBufferOffsetAlignment) + "\n" +
           "minStorageBufferOffsetAlignment: " + utils_string::str(deviceProperties.limits.minStorageBufferOffsetAlignment) + "\n" +
           "minTexelOffset: " + utils_string::str(deviceProperties.limits.minTexelOffset) + "\n" +
           "maxTexelOffset: " + utils_string::str(deviceProperties.limits.maxTexelOffset) + "\n" +
           "minTexelGatherOffset: " + utils_string::str(deviceProperties.limits.minTexelGatherOffset) + "\n" +
           "maxTexelGatherOffset: " + utils_string::str(deviceProperties.limits.maxTexelGatherOffset) + "\n" +
           "minInterpolationOffset: " + utils_string::str(deviceProperties.limits.minInterpolationOffset) + "\n" +
           "maxInterpolationOffset: " + utils_string::str(deviceProperties.limits.maxInterpolationOffset) + "\n" +
           "subPixelInterpolationOffsetBits: " + utils_string::str(deviceProperties.limits.subPixelInterpolationOffsetBits) + "\n" +
           "maxFramebufferWidth: " + utils_string::str(deviceProperties.limits.maxFramebufferWidth) + "\n" +
           "maxFramebufferHeight: " + utils_string::str(deviceProperties.limits.maxFramebufferHeight) + "\n" +
           "maxFramebufferLayers: " + utils_string::str(deviceProperties.limits.maxFramebufferLayers) + "\n" +
           "framebufferColorSampleCounts: " + utils_string::str(deviceProperties.limits.framebufferColorSampleCounts) + "\n" +
           "framebufferDepthSampleCounts: " + utils_string::str(deviceProperties.limits.framebufferDepthSampleCounts) + "\n" +
           "framebufferStencilSampleCounts: " + utils_string::str(deviceProperties.limits.framebufferStencilSampleCounts) + "\n" +
           "framebufferNoAttachmentsSampleCounts: " + utils_string::str(deviceProperties.limits.framebufferNoAttachmentsSampleCounts) + "\n" +
           "maxColorAttachments: " + utils_string::str(deviceProperties.limits.maxColorAttachments) + "\n" +
           "sampledImageColorSampleCounts: " + utils_string::str(deviceProperties.limits.sampledImageColorSampleCounts) + "\n" +
           "sampledImageIntegerSampleCounts: " + utils_string::str(deviceProperties.limits.sampledImageIntegerSampleCounts) + "\n" +
           "sampledImageDepthSampleCounts: " + utils_string::str(deviceProperties.limits.sampledImageDepthSampleCounts) + "\n" +
           "sampledImageStencilSampleCounts: " + utils_string::str(deviceProperties.limits.sampledImageStencilSampleCounts) + "\n" +
           "storageImageSampleCounts: " + utils_string::str(deviceProperties.limits.storageImageSampleCounts) + "\n" +
           "maxSampleMaskWords: " + utils_string::str(deviceProperties.limits.maxSampleMaskWords) + "\n" +
           "timestampComputeAndGraphics: " + utils_string::str(deviceProperties.limits.timestampComputeAndGraphics) + "\n" +
           "timestampPeriod: " + utils_string::str(deviceProperties.limits.timestampPeriod) + "\n" +
           "maxClipDistances: " + utils_string::str(deviceProperties.limits.maxClipDistances) + "\n" +
           "maxCullDistances: " + utils_string::str(deviceProperties.limits.maxCullDistances) + "\n" +
           "maxCombinedClipAndCullDistances: " + utils_string::str(deviceProperties.limits.maxCombinedClipAndCullDistances) + "\n" +
           "discreteQueuePriorities: " + utils_string::str(deviceProperties.limits.discreteQueuePriorities) + "\n" +
           "pointSizeRange[0]: " + utils_string::str(deviceProperties.limits.pointSizeRange[0]) + "\n" +
           "pointSizeRange[1]: " + utils_string::str(deviceProperties.limits.pointSizeRange[1]) + "\n" +
           "lineWidthRange[0]: " + utils_string::str(deviceProperties.limits.lineWidthRange[2]) + "\n" +
           "lineWidthRange[1]: " + utils_string::str(deviceProperties.limits.lineWidthRange[2]) + "\n" +
           "pointSizeGranularity: " + utils_string::str(deviceProperties.limits.pointSizeGranularity) + "\n" +
           "lineWidthGranularity: " + utils_string::str(deviceProperties.limits.lineWidthGranularity) + "\n" +
           "strictLines: " + utils_string::str(deviceProperties.limits.strictLines) + "\n" +
           "standardSampleLocations: " + utils_string::str(deviceProperties.limits.standardSampleLocations) + "\n" +
           "optimalBufferCopyOffsetAlignment: " + utils_string::str(deviceProperties.limits.optimalBufferCopyOffsetAlignment) + "\n" +
           "optimalBufferCopyRowPitchAlignment: " + utils_string::str(deviceProperties.limits.optimalBufferCopyRowPitchAlignment) + "\n" +
           "nonCoherentAtomSize: " + utils_string::str(deviceProperties.limits.nonCoherentAtomSize) + "\n";
}