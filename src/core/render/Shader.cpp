#include "Shader.h"

#include <boost/filesystem.hpp>

#include "../../utils/FileUtils.h"

/*****************************************************************************
 * Shader class - Stores info on a shader module and its usage
 *****************************************************************************/

const std::map<std::string, std::string> Shader::SHADER_EXTENSIONS = {
    {"vert", "_vert.spv"},
    {"geom", "_geom.spv"},
    {"frag", "_frag.spv"},
    {"rgen", "_rgen.spv"},
    {"rmiss", "_rmiss.spv"},
    {"rhit", "_rhit.spv"},
};

const std::map<std::string, VkShaderStageFlagBits> Shader::COMPILED_SHADER_EXTENSIONS = {
    {"vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
    {"geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT},
    {"frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT},
    {"rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR},
    {"rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR},
    {"rhit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR},
};

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
    std::vector<std::string> split = utils_string::split(path, '_');
    std::string extension          = split[split.size() - 1];
    VkShaderStageFlagBits stage;

    if (COMPILED_SHADER_EXTENSIONS.count(extension) > 0)
        stage = COMPILED_SHADER_EXTENSIONS.at(extension);
    else
        Logger::logAndThrowError("Unsupported file extension for '" + path + "'", "Shader");

    // Return the loaded shader
    return Shader::load(device, path, stage, entrypoint);
}

void Shader::compile(const std::string& inputPath, const std::string& outputPath, const std::string& filePath, const std::string& glslangValidatorPath) {
    // Split by extension
    std::vector<std::string> split = utils_string::splitLast(filePath, ".");
    std::string extension          = split[1];
    
    if (SHADER_EXTENSIONS.count(extension) > 0)
        std::system((glslangValidatorPath + " --target-env vulkan1.2 -V " + inputPath + filePath + " -o " + outputPath + split[0] + SHADER_EXTENSIONS.at(split[1])).c_str());
    else
        Logger::log("Unsupported file extension '" + extension + "'", "Shader", LogType::Error);
}

/*****************************************************************************
 * ShaderGroup class
 *****************************************************************************/

ShaderGroup::~ShaderGroup() {
    // Destroy the attached shaders
    for (const auto* shader : shaders)
        delete shader;
}

ShaderGroup* ShaderGroup::load(VulkanDevice* device, const std::string& path) {
    // Find the files in that have the same path and name but different file
    // extensions and load them
    std::vector<Shader*> foundShaders;

    for (auto& pair : Shader::COMPILED_SHADER_EXTENSIONS) {
        std::string currentPath = path + "_" + pair.first;

        if (utils_file::isFile(currentPath))
            foundShaders.push_back(Shader::load(device, currentPath));
    }

    return new ShaderGroup(foundShaders);
}

void ShaderGroup::compile(const std::string& inputPath, const std::string& outputPath, const std::string& fileName, const std::string& glslangValidatorPath) {
    // Find the files in that have the same path and name but different file
    // extensions and compile them

    for (auto& pair : Shader::SHADER_EXTENSIONS) {
        std::string currentPath = inputPath + fileName + "." + pair.first;

        if (utils_file::isFile(currentPath))
            Shader::compile(inputPath, outputPath, fileName + "." + pair.first, glslangValidatorPath);
    }
}
