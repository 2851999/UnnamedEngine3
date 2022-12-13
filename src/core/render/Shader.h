
#pragma once

#include "../vulkan/VulkanResource.h"

/*****************************************************************************
 * Shader class - Stores info on a shader module and its usage
 *****************************************************************************/

class Shader : VulkanResource {
private:
    VkShaderModule module;
    VkShaderStageFlagBits stage;
    const char* entrypoint;

public:
    Shader(VulkanDevice* device, VkShaderModule module, VkShaderStageFlagBits stage, const char* entrypoint) : VulkanResource(device), module(module), stage(stage), entrypoint(entrypoint) {}
    virtual ~Shader() { device->destroyShaderModule(module); }

    /* Populates a shader stage create info object with the information for
       this module and returns it */
    inline VkPipelineShaderStageCreateInfo getShaderStageCreateInfo() {
        VkPipelineShaderStageCreateInfo createInfo = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage                           = stage;
        createInfo.module                          = module;
        createInfo.pName                           = entrypoint;

        return createInfo;
    }

    /* Loads a shader from a file */
    static Shader* load(VulkanDevice* device, const std::string& path, VkShaderStageFlagBits stage, const char* entrypoint);

    /* Loads a shader from a file - interpreting the stage from the file
       extension */
    static Shader* load(VulkanDevice* device, const std::string& path, const char* entrypoint = "main");
};
