#include "Mesh.h"

#include "../vulkan/VulkanUtils.h"
#include "ShaderInterface.h"

/*****************************************************************************
 * MeshData class
 *****************************************************************************/

Sphere MeshData::calculateBoundingSphere() {
    // The bounding sphere
    Sphere sphere;
    // Calculate the find the lengths between the mesh, and also find the largest one
    float lengthX       = maxX - minX;
    float lengthY       = maxY - minY;
    float lengthZ       = maxZ - minZ;
    float largestLength = utils_maths::max(utils_maths::max(lengthX, lengthY), lengthZ);

    // Calculate the centre and radius of the bound sphere
    sphere.centre = Vector3f((maxX + minX) / 2.0f, (maxY + minY) / 2.0f, (maxZ + minZ) / 2.0f);
    sphere.radius = largestLength / 2.0f;

    // Return the sphere
    return sphere;
}

void MeshData::addPosition(Vector2f position) {
    // Check to see whether it should be separated
    if (separatePositions()) {
        positions.push_back(position.getX());
        positions.push_back(position.getY());
    } else {
        others.push_back(position.getX());
        others.push_back(position.getY());
    }
}

void MeshData::addPosition(Vector3f position) {
    // Check to see whether it should be separated
    if (separatePositions()) {
        positions.push_back(position.getX());
        positions.push_back(position.getY());
        positions.push_back(position.getZ());
    } else {
        others.push_back(position.getX());
        others.push_back(position.getY());
        others.push_back(position.getZ());
    }

    // Check whether data for a bounding sphere should be calculated
    if (numDimensions == 3) {
        minX = utils_maths::min(minX, position.getX());
        maxX = utils_maths::max(maxX, position.getX());
        minY = utils_maths::min(minY, position.getY());
        maxY = utils_maths::max(maxY, position.getY());
        minZ = utils_maths::min(minZ, position.getZ());
        maxZ = utils_maths::max(maxZ, position.getZ());
    }
}

void MeshData::addColour(Colour colour) {
    // Check to see whether it should be separated
    if (separateColours()) {
        colours.push_back(colour.getR());
        colours.push_back(colour.getG());
        colours.push_back(colour.getB());
        colours.push_back(colour.getA());
    } else {
        others.push_back(colour.getR());
        others.push_back(colour.getG());
        others.push_back(colour.getB());
        others.push_back(colour.getA());
    }
}

void MeshData::addTextureCoord(Vector2f textureCoord) {
    // Check to see whether it should be separated
    if (separateTextureCoords()) {
        textureCoords.push_back(textureCoord.getX());
        textureCoords.push_back(textureCoord.getY());
    } else {
        others.push_back(textureCoord.getX());
        others.push_back(textureCoord.getY());
    }
}

void MeshData::addNormal(Vector3f normal) {
    // Check to see whether it should be separated
    if (separateNormals()) {
        normals.push_back(normal.getX());
        normals.push_back(normal.getY());
        normals.push_back(normal.getZ());
    } else {
        others.push_back(normal.getX());
        others.push_back(normal.getY());
        others.push_back(normal.getZ());
    }
}

void MeshData::addTangent(Vector3f tangent) {
    // Check to see whether it should be separated
    if (separateTangents()) {
        tangents.push_back(tangent.getX());
        tangents.push_back(tangent.getY());
        tangents.push_back(tangent.getZ());
    } else {
        others.push_back(tangent.getX());
        others.push_back(tangent.getY());
        others.push_back(tangent.getZ());
    }
}

void MeshData::addBitangent(Vector3f bitangent) {
    // Check to see whether it should be separated
    if (separateBitangents()) {
        bitangents.push_back(bitangent.getX());
        bitangents.push_back(bitangent.getY());
        bitangents.push_back(bitangent.getZ());
    } else {
        others.push_back(bitangent.getX());
        others.push_back(bitangent.getY());
        others.push_back(bitangent.getZ());
    }
}

// TODO: Take in RenderShader or something here and use it for shader locations only - that way can avoid having issues with ordering when certain parameters
//       are not assigned
GraphicsPipeline::VertexInputDescription MeshData::computeVertexInputDescription(unsigned int numDimensions, std::vector<DataType> requiredData, SeparateFlags flags) {
    // The output data
    GraphicsPipeline::VertexInputDescription description;
    description.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    unsigned int currentBinding = 0;

    std::vector<DataType> otherData;

    // Go through the required data
    for (DataType current : requiredData) {
        if (current == POSITION) {
            if ((flags & SEPARATE_POSITIONS)) {
                description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_POSITION, currentBinding, numDimensions == 3 ? VK_FORMAT_R32G32B32_SFLOAT : VK_FORMAT_R32G32_SFLOAT, 0));
                description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, numDimensions * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
                ++currentBinding;
            } else
                otherData.push_back(current);
        } else if (current == COLOUR) {
            if ((flags & SEPARATE_COLOURS)) {
                description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_POSITION, currentBinding, VK_FORMAT_R32G32B32A32_SFLOAT, 0));
                description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 4 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
                ++currentBinding;
            } else
                otherData.push_back(current);
        } else if (current == TEXTURE_COORD) {
            if ((flags & SEPARATE_TEXTURE_COORDS)) {
                description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_TEXTURE_COORD, currentBinding, VK_FORMAT_R32G32_SFLOAT, 0));
                description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 2 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
                ++currentBinding;
            } else
                otherData.push_back(current);
        } else if (current == NORMAL) {
            if ((flags & SEPARATE_NORMALS)) {
                description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_NORMAL, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, 0));
                description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
                ++currentBinding;
            } else
                otherData.push_back(current);
        } else if (current == TANGENT) {
            if ((flags & SEPARATE_TANGENTS)) {
                description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_TANGENT, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, 0));
                description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
                ++currentBinding;
            } else
                otherData.push_back(current);
        } else if (current == BITANGENT) {
            if ((flags & SEPARATE_BITANGENTS)) {
                description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_BITANGENT, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, 0));
                description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 3 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
                ++currentBinding;
            } else
                otherData.push_back(current);
        } else
            otherData.push_back(current);
    }
    // States whether bone data is included
    bool hasBones = false;
    if (otherData.size() > 0) {
        // The current offset
        unsigned int currentOffset = 0;
        // Add others data if necessary
        for (DataType current : otherData) {
            switch (current) {
                case POSITION:
                    description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_POSITION, currentBinding, numDimensions == 3 ? VK_FORMAT_R32G32B32_SFLOAT : VK_FORMAT_R32G32_SFLOAT, currentOffset));
                    currentOffset += sizeof(float) * numDimensions;
                    break;
                case COLOUR:
                    description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_COLOUR, currentBinding, VK_FORMAT_R32G32B32A32_SFLOAT, currentOffset));
                    currentOffset += sizeof(float) * 4;
                    break;
                case TEXTURE_COORD:
                    description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_TEXTURE_COORD, currentBinding, VK_FORMAT_R32G32_SFLOAT, currentOffset));
                    currentOffset += sizeof(float) * 2;
                    break;
                case NORMAL:
                    description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_NORMAL, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, currentOffset));
                    currentOffset += sizeof(float) * 3;
                    break;
                case TANGENT:
                    description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_TANGENT, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, currentOffset));
                    currentOffset += sizeof(float) * 3;
                    break;
                case BITANGENT:
                    description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_BITANGENT, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, currentOffset));
                    currentOffset += sizeof(float) * 3;
                    break;
                case BONE_INDEX:
                case BONE_WEIGHT:
                    hasBones = true;
                    break;
            }
        }
        // Add the other VBO binding
        description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, currentOffset, VK_VERTEX_INPUT_RATE_VERTEX));
        ++currentBinding;
    }

    if (hasBones) {
        // Add bone info
        description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_BONE_WEIGHTS, currentBinding, VK_FORMAT_R32G32B32A32_SFLOAT, 0));
        description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 4 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
        ++currentBinding;
        description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(ShaderInterface::ATTRIBUTE_LOCATION_BONE_INDICES, currentBinding, VK_FORMAT_R32G32B32A32_UINT, 0));
        description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 4 * sizeof(uint32_t), VK_VERTEX_INPUT_RATE_VERTEX));
    }

    return description;
}