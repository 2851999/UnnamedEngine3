#include "Shader.h"

#include "../../utils/FileUtils.h"

/*****************************************************************************
 * Shader class - Stores info on a shader module and its usage
 *****************************************************************************/

Shader* Shader::load(VulkanDevice* device, const std::string& path, VkShaderStageFlagBits stage, const char* entrypoint) {
    // Read the file
    std::vector<char> buffer = utils_file::readBinChar(path);

    // Create the shader module
    VkShaderModule shaderModule = device->createShaderModule(buffer);

    // Return the shader
    return new Shader(device, shaderModule, stage, entrypoint);
}

Shader* Shader::load(VulkanDevice* device, const std::string& path, const char* entrypoint) {
    // Obtain the file extension
    std::vector<std::string> split = utils_string::split(path, '.');
    std::string extension          = split[split.size() - 1];
    VkShaderStageFlagBits stage;

    if (extension == ".vert")
        stage = VK_SHADER_STAGE_VERTEX_BIT;
    else if (extension == ".geom")
        stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    else if (extension == ".frag")
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    else if (extension == ".rgen")
        stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    else if (extension == ".rmiss")
        stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    else if (extension == ".rchit")
        stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    else
        Logger::logAndThrowError("Unsupported file extension for '" + path + "'", "Shader");

    // Return the loaded shader
    return Shader::load(device, path, stage, entrypoint);
}