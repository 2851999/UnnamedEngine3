
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
    static const std::map<std::string, std::string> SHADER_EXTENSIONS;
    static const std::map<std::string, VkShaderStageFlagBits> COMPILED_SHADER_EXTENSIONS;

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

    /* Compiles a shader module */
    static void compile(const std::string& inputPath, const std::string& outputPath, const std::string& filePath, const std::string& glslangValidatorPath);
};

/*****************************************************************************
 * ShaderGroup class - Handles a group of shaders (That will be combined into
 *                     a single pipeline)
 *****************************************************************************/

class ShaderGroup {
private:
    /* Shaders in this group */
    std::vector<Shader*> shaders;

public:
    /* Constructor and destructor - assumes ownership of the given shaders */
    ShaderGroup(std::vector<Shader*> shaders) : shaders(shaders) {};
    virtual ~ShaderGroup();

    /* Loads a group of shaders given its common path (without a file extension) */
    static ShaderGroup* load(VulkanDevice* device, const std::string& path);

    /* Compiles a group of shaders given its common name (without a file extension) */
    static void compile(const std::string& inputPath, const std::string& outputPath, const std::string& fileName, const std::string& glslangValidatorPath);
};
