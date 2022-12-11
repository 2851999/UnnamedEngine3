#include "VulkanFeatures.h"

/*****************************************************************************
 * VulkanFeatures class
 *****************************************************************************/

const std::string VulkanFeatures::RAY_TRACING = "ray_tracing";

void* VulkanFeatures::setupPNext(std::vector<void*>& selectedFeatures) const {
    // Structure to help linking the pNext of features
    struct FeatureHeader {
        VkStructureType sType;
        void* pNext;
    };

    for (unsigned int i = 0; i < selectedFeatures.size(); ++i) {
        auto* header = reinterpret_cast<FeatureHeader*>(selectedFeatures[i]);
        // Assign the pNext to the next feature in the list provided it is not the last
        header->pNext = i < selectedFeatures.size() - 1 ? selectedFeatures[i + 1] : nullptr;
    }
    return selectedFeatures[0];
}

void VulkanFeatures::addFeatures(const Settings& settings) {
    rayTracing = settings.video.rayTracing;
}

VulkanFeatures::Support VulkanFeatures::querySupport(VkPhysicalDevice device) const {
    // Structure to hold the feature support info
    VulkanFeatures::Support supportedFeatures;

    // Obtain the supported features
    VkPhysicalDeviceFeatures supportedDeviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedDeviceFeatures);

    // Check the standard required features
    supportedFeatures.required = supportedDeviceFeatures.samplerAnisotropy && supportedDeviceFeatures.geometryShader;

    // Is raytracing supported (if requested)
    bool supportsRayTracing = false;

    // Query features that require use of pNext
    if (rayTracing) {
        VkPhysicalDeviceVulkan11Features supportedVulkan11Features                                       = {};
        VkPhysicalDeviceBufferDeviceAddressFeatures supportedDeviceBufferAddressFeatures                 = {};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR supportedDeviceRayTracingPipelineFeaturesKHR       = {};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR supportedDeviceAccelerationStructureFeaturesKHR = {};
        VkPhysicalDeviceShaderClockFeaturesKHR supportedDeviceShaderClockFeaturesKHR                     = {};
        VkPhysicalDeviceDescriptorIndexingFeatures supportedDeviceDescriptorIndexingFeatures             = {};
        VkPhysicalDeviceHostQueryResetFeatures supportedDeviceHostQueryResetFeatures                     = {};

        supportedVulkan11Features.sType                       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        supportedDeviceBufferAddressFeatures.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        supportedDeviceRayTracingPipelineFeaturesKHR.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        supportedDeviceAccelerationStructureFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        supportedDeviceShaderClockFeaturesKHR.sType           = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR;
        supportedDeviceDescriptorIndexingFeatures.sType       = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        supportedDeviceHostQueryResetFeatures.sType           = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;

        // Required device features
        std::vector<void*> requiredDeviceFeatures = {
            &supportedVulkan11Features,
            &supportedDeviceBufferAddressFeatures,
            &supportedDeviceRayTracingPipelineFeaturesKHR,
            &supportedDeviceAccelerationStructureFeaturesKHR,
            &supportedDeviceShaderClockFeaturesKHR,
            &supportedDeviceDescriptorIndexingFeatures,
            &supportedDeviceHostQueryResetFeatures,
        };

        VkPhysicalDeviceFeatures2 supportedFeatures2 = {};
        supportedFeatures2.sType                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        supportedFeatures2.pNext                     = setupPNext(requiredDeviceFeatures);
        vkGetPhysicalDeviceFeatures2(device, &supportedFeatures2);

        supportsRayTracing = supportedDeviceFeatures.shaderInt64 &&
                             supportedDeviceBufferAddressFeatures.bufferDeviceAddress &&
                             supportedDeviceRayTracingPipelineFeaturesKHR.rayTracingPipeline &&
                             supportedDeviceAccelerationStructureFeaturesKHR.accelerationStructure &&
                             supportedDeviceShaderClockFeaturesKHR.shaderSubgroupClock &&
                             supportedDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing &&
                             supportedDeviceDescriptorIndexingFeatures.runtimeDescriptorArray &&
                             supportedDeviceHostQueryResetFeatures.hostQueryReset;
    }

    supportedFeatures.optionals.insert(std::pair<std::string, bool>(VulkanFeatures::RAY_TRACING, supportsRayTracing));

    return supportedFeatures;
}

void VulkanFeatures::assignVkDeviceCreateInfo(VkDeviceCreateInfo& createInfo, VulkanFeatures::Support& supportedFeatures) {
    // Request anisotropic filtering/geometry shaders
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.geometryShader    = VK_TRUE;

    // Extra features for ray tracing (Only if needed)
    if (supportedFeatures.get(VulkanFeatures::RAY_TRACING)) {
        // Required for using GL_EXT_shader_explicit_arithmetic_types_int64 in shaders
        deviceFeatures.shaderInt64 = VK_TRUE;

        deviceBufferDeviceAddressFeatures.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        deviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
        selectedFeatures.push_back(&deviceBufferDeviceAddressFeatures);

        deviceRayTracingPipelineFeaturesKHR.sType              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        deviceRayTracingPipelineFeaturesKHR.rayTracingPipeline = VK_TRUE;
        selectedFeatures.push_back(&deviceRayTracingPipelineFeaturesKHR);

        deviceAccelerationStructureFeaturesKHR.sType                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        deviceAccelerationStructureFeaturesKHR.accelerationStructure = VK_TRUE;
        selectedFeatures.push_back(&deviceAccelerationStructureFeaturesKHR);

        deviceShaderClockFeaturesKHR.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR;
        deviceShaderClockFeaturesKHR.shaderSubgroupClock = VK_TRUE;
        selectedFeatures.push_back(&deviceShaderClockFeaturesKHR);

        deviceDescriptorIndexingFeatures.sType                                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        deviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        deviceDescriptorIndexingFeatures.runtimeDescriptorArray                    = VK_TRUE;
        selectedFeatures.push_back(&deviceDescriptorIndexingFeatures);

        deviceHostQueryResetFeatures.sType          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES;
        deviceHostQueryResetFeatures.hostQueryReset = VK_TRUE;
        selectedFeatures.push_back(&deviceHostQueryResetFeatures);
    }

    // Setup pNext values for the features
    if (selectedFeatures.size() > 0) {
        deviceFeatures2.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.features = deviceFeatures;
        deviceFeatures2.pNext    = setupPNext(selectedFeatures);
    }

    // Assign the values depending on whether we need VkPhysicalDeviceFeatures2
    if (selectedFeatures.size() > 0) {
        createInfo.pEnabledFeatures = nullptr;  // Assigned in deviceFeatures2 instead
        createInfo.pNext            = &deviceFeatures2;
    } else
        createInfo.pEnabledFeatures = &deviceFeatures;
}