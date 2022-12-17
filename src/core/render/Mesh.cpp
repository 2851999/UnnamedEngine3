#include "Mesh.h"

#include "../vulkan/VulkanUtils.h"
#include "ShaderInterface.h"

/*****************************************************************************
 * MeshData class
 *****************************************************************************/

std::map<int, MeshData::DataTypeInfo> MeshData::datatypeInfoMaps = {
    {POSITION, {SEPARATE_POSITIONS, 3 * sizeof(float), VK_FORMAT_R32G32B32_SFLOAT}},
    {COLOUR, {SEPARATE_COLOURS, 4 * sizeof(float), VK_FORMAT_R32G32B32A32_SFLOAT}},
    {TEXTURE_COORD, {SEPARATE_TEXTURE_COORDS, 2 * sizeof(float), VK_FORMAT_R32G32_SFLOAT}},
    {TANGENT, {SEPARATE_TANGENTS, 3 * sizeof(float), VK_FORMAT_R32G32B32_SFLOAT}},
    {BITANGENT, {SEPARATE_BITANGENTS, 3 * sizeof(float), VK_FORMAT_R32G32B32_SFLOAT}}};

MeshData::DataTypeInfo MeshData::getDataTypeInfo(unsigned int numDimensions, MeshData::DataType dataType) {
    if (datatypeInfoMaps.count(dataType) == 0)
        Logger::logAndThrowError("Failed to obtain data type info for datatype " + utils_string::str(dataType), "MeshData");

    // Special case
    if (dataType == DataType::POSITION) {
        DataTypeInfo info = datatypeInfoMaps.at(dataType);
        info.size         = numDimensions == 3 ? 3 * sizeof(float) : 2 * sizeof(float);
        return info;
    } else
        return datatypeInfoMaps.at(dataType);
}

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

GraphicsPipeline::VertexInputDescription MeshData::computeVertexInputDescription(unsigned int numDimensions, std::vector<DataType> requiredData, SeparateFlags flags, ShaderInterface shaderInterface) {
    // The output data
    GraphicsPipeline::VertexInputDescription description;
    description.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    unsigned int currentBinding = 0;

    std::vector<DataType> otherData;

    // Go through the required data
    for (DataType current : requiredData) {
        // Obtain the datatype info
        DataTypeInfo typeInfo = getDataTypeInfo(numDimensions, current);

        if ((flags & typeInfo.separateFlag)) {
            description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(shaderInterface.getAttributeLocation(current), currentBinding, typeInfo.format, 0));
            description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, typeInfo.size, VK_VERTEX_INPUT_RATE_VERTEX));
            ++currentBinding;
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
                case BONE_INDEX:
                case BONE_WEIGHT:
                    hasBones = true;
                    break;
                default:
                    DataTypeInfo typeInfo = getDataTypeInfo(numDimensions, current);
                    description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(shaderInterface.getAttributeLocation(current), currentBinding, typeInfo.format, currentOffset));
                    currentOffset += typeInfo.size;
                    break;
            }
        }
        // Add the other VBO binding
        description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, currentOffset, VK_VERTEX_INPUT_RATE_VERTEX));
        ++currentBinding;
    }

    if (hasBones) {
        // Add bone info
        description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(shaderInterface.getAttributeLocation(MeshData::BONE_WEIGHT), currentBinding, VK_FORMAT_R32G32B32A32_SFLOAT, 0));
        description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 4 * sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX));
        ++currentBinding;
        description.attributes.push_back(utils_vulkan::initVertexAttributeDescription(shaderInterface.getAttributeLocation(MeshData::BONE_INDEX), currentBinding, VK_FORMAT_R32G32B32A32_UINT, 0));
        description.bindings.push_back(utils_vulkan::initVertexInputBindings(currentBinding, 4 * sizeof(uint32_t), VK_VERTEX_INPUT_RATE_VERTEX));
    }

    return description;
}