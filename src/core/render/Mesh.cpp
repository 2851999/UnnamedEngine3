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

    ++vertexCount;
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

    ++vertexCount;
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

/*****************************************************************************
 * MeshRenderData class
 *****************************************************************************/

MeshRenderData::MeshRenderData(Renderer* renderer, MeshData* data) {
    // TODO: Don't hard code this
    bool deviceLocal       = true;
    bool persistentMapping = false;

    // For now will also assume if separated then want to be updatable
    // as this is main reason you might want to separate them
    // TODO: ^ Don't assume this?

    // Vertex buffers
    std::vector<VBO*> vertexBuffers;

    // Create the buffers as needed
    if (data->hasPositions() && data->separatePositions()) {
        vboPositions = new VBO(renderer, data->getPositions().size() * sizeof(data->getPositions()[0]), data->getPositions().data(), deviceLocal, persistentMapping, true);
        vertexBuffers.push_back(vboPositions);
    }

    if (data->hasColours() && data->separateColours()) {
        vboColours = new VBO(renderer, data->getColours().size() * sizeof(data->getColours()[0]), data->getColours().data(), deviceLocal, persistentMapping, true);
        vertexBuffers.push_back(vboColours);
    }

    if (data->hasTextureCoords() && data->separateTextureCoords()) {
        vboTextureCoords = new VBO(renderer, data->getTextureCoords().size() * sizeof(data->getTextureCoords()[0]), data->getTextureCoords().data(), deviceLocal, persistentMapping, true);
        vertexBuffers.push_back(vboTextureCoords);
    }

    if (data->hasNormals() && data->separateNormals()) {
        vboNormals = new VBO(renderer, data->getNormals().size() * sizeof(data->getNormals()[0]), data->getNormals().data(), deviceLocal, persistentMapping, true);
        vertexBuffers.push_back(vboNormals);
    }

    if (data->hasTangents() && data->separateTangents()) {
        vboTangents = new VBO(renderer, data->getTangents().size() * sizeof(data->getTangents()[0]), data->getTangents().data(), deviceLocal, persistentMapping, true);
        vertexBuffers.push_back(vboTangents);
    }

    if (data->hasBitangents() && data->separateBitangents()) {
        vboBitangents = new VBO(renderer, data->getBitangents().size() * sizeof(data->getBitangents()[0]), data->getBitangents().data(), deviceLocal, persistentMapping, true);
        vertexBuffers.push_back(vboBitangents);
    }

    if (data->hasOthers()) {
        vboOthers = new VBO(renderer, data->getOthers().size() * sizeof(data->getOthers()[0]), data->getOthers().data(), deviceLocal, persistentMapping, false);
        vertexBuffers.push_back(vboOthers);
    }

    // Setup bones
    if (data->hasBones()) {
        vboBoneIndices = new VBO(renderer, data->getBoneIndices().size() * sizeof(data->getBoneIndices()[0]), data->getBoneIndices().data(), deviceLocal, persistentMapping, false);
        vertexBuffers.push_back(vboBoneIndices);

        vboBoneWeights = new VBO(renderer, data->getBoneWeights().size() * sizeof(data->getBoneWeights()[0]), data->getOthers().data(), deviceLocal, persistentMapping, false);
        vertexBuffers.push_back(vboBoneWeights);
    }

    // Setup material and offset indices only if assigned
    if (data->hasMaterialIndices())
        bufferMaterialIndices = new VBO(renderer, data->getMaterialIndices().size() * sizeof(data->getMaterialIndices()[0]), data->getMaterialIndices().data(), deviceLocal, persistentMapping, false);
    if (data->hasOffsetIndices())
        bufferOffsetIndices = new VBO(renderer, data->getOffsetIndices().size() * sizeof(data->getOffsetIndices()[0]), data->getOffsetIndices().data(), deviceLocal, persistentMapping, false);

    // Setup indices
    if (data->hasIndices())
        ibo = new IBO(renderer, data->getIndices().size() * sizeof(data->getIndices()[0]), data->getIndices().data(), VK_INDEX_TYPE_UINT32, deviceLocal, persistentMapping, false);

    renderData = new RenderData(vertexBuffers, ibo, data->getCount());
}

MeshRenderData::~MeshRenderData() {
    delete renderData;
}

/*****************************************************************************
 * MeshBuilder class
 *****************************************************************************/

/* 2D Stuff */

MeshData* MeshBuilder::createTriangle(Vector2f v1, Vector2f v2, Vector2f v3, MeshData::SeparateFlags flags) {
    MeshData* data = new MeshData(2, flags);

    data->addPosition(v1);
    data->addPosition(v2);
    data->addPosition(v3);

    return data;
}

MeshData* MeshBuilder::createQuad(Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4, MeshData::SeparateFlags flags) {
    MeshData* data = new MeshData(2, flags);
    addQuadData(data, v1, v2, v3, v4);
    data->addTextureCoord(Vector2f(0, 0));
    data->addTextureCoord(Vector2f(0, 0));
    data->addTextureCoord(Vector2f(0, 0));
    data->addTextureCoord(Vector2f(0, 0));  // Vulkan expects all GUI to have texture coord data
    addQuadI(data);
    return data;
}

// MeshData* MeshBuilder::createQuad(Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4, Texture* texture, MeshData::SeparateFlags flags) {
//     MeshData* data = new MeshData(2, flags);
//     addQuadData(data, v1, v2, v3, v4, texture);
//     addQuadI(data);
//     return data;
// }

MeshData* MeshBuilder::createQuad(float width, float height, MeshData::SeparateFlags flags) {
    MeshData* data = new MeshData(2, flags);
    addQuadData(data, Vector2f(0, 0), Vector2f(width, 0), Vector2f(width, height), Vector2f(0, height));
    data->addTextureCoord(Vector2f(0, 0));
    data->addTextureCoord(Vector2f(0, 0));
    data->addTextureCoord(Vector2f(0, 0));
    data->addTextureCoord(Vector2f(0, 0));  // Vulkan expects all GUI to have texture coord data
    addQuadI(data);
    return data;
}

// MeshData* MeshBuilder::createQuad(float width, float height, Texture* texture, MeshData::SeparateFlags flags) {
//     MeshData* data = new MeshData(2, flags);
//     addQuadData(data, Vector2f(0, 0), Vector2f(width, 0), Vector2f(width, height), Vector2f(0, height), texture);
//     addQuadI(data);
//     return data;
// }

void MeshBuilder::addQuadData(MeshData* data, Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4) {
    data->addPosition(v1);
    data->addPosition(v2);
    data->addPosition(v3);
    data->addPosition(v4);
}

// void MeshBuilder::addQuadData(MeshData* data, Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4, Texture* texture) {
//     data->addPosition(v1);
//     data->addTextureCoord(Vector2f(0, 0));
//     data->addPosition(v2);
//     data->addTextureCoord(Vector2f(1, 0));
//     data->addPosition(v3);
//     data->addTextureCoord(Vector2f(1, 1));
//     data->addPosition(v4);
//     data->addTextureCoord(Vector2f(0, 1));
// }

void MeshBuilder::addQuadI(MeshData* data) {
    data->addIndex(0);
    data->addIndex(1);
    data->addIndex(2);
    data->addIndex(3);
    data->addIndex(0);
    data->addIndex(2);
}

void MeshBuilder::addQuadT(MeshData* data, float top, float left, float bottom, float right) {
    data->addTextureCoord(Vector2f(left, top));
    data->addTextureCoord(Vector2f(right, top));
    data->addTextureCoord(Vector2f(right, bottom));
    data->addTextureCoord(Vector2f(left, bottom));
}

/* 3D Stuff */

MeshData* MeshBuilder::createQuad3D(Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4, MeshData::SeparateFlags flags) {
    MeshData* data = new MeshData(3, flags);
    addQuadData3D(data, v1, v2, v3, v4);
    addQuadI(data);
    return data;
}

// MeshData* MeshBuilder::createQuad3D(Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4, Texture* texture, MeshData::SeparateFlags flags) {
//     MeshData* data = new MeshData(3, flags);
//     addQuadData3D(data, v1, v2, v3, v4, texture);
//     addQuadI(data);
//     return data;
// }

MeshData* MeshBuilder::createQuad3D(float width, float height, MeshData::SeparateFlags flags) {
    MeshData* data = new MeshData(3, flags);
    addQuadData3D(data, Vector2f(-width / 2, -height / 2), Vector2f(width / 2, -height / 2), Vector2f(width / 2, height / 2), Vector2f(-width / 2, height / 2));
    addQuadI(data);
    return data;
}

// MeshData* MeshBuilder::createQuad3D(float width, float height, Texture* texture, MeshData::SeparateFlags flags) {
//     MeshData* data = new MeshData(3, flags);
//     addQuadData3D(data, Vector2f(-width / 2, -height / 2), Vector2f(width / 2, -height / 2), Vector2f(width / 2, height / 2), Vector2f(-width / 2, height / 2), texture);
//     addQuadI(data);
//     return data;
// }

void MeshBuilder::addQuadData3D(MeshData* data, Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4) {
    data->addPosition(Vector3f(v1, 0.0f));
    data->addPosition(Vector3f(v2, 0.0f));
    data->addPosition(Vector3f(v3, 0.0f));
    data->addPosition(Vector3f(v4, 0.0f));
}

// void MeshBuilder::addQuadData3D(MeshData* data, Vector2f v1, Vector2f v2, Vector2f v3, Vector2f v4, Texture* texture) {
//     data->addPosition(Vector3f(v1));
//     data->addTextureCoord(Vector2f(0, 0));
//     data->addPosition(Vector3f(v2));
//     data->addTextureCoord(Vector2f(1, 0));
//     data->addPosition(Vector3f(v3));
//     data->addTextureCoord(Vector2f(1, 1));
//     data->addPosition(Vector3f(v4));
//     data->addTextureCoord(Vector2f(0, 1));
// }

MeshData* MeshBuilder::createCube(float width, float height, float depth, MeshData::SeparateFlags flags) {
    MeshData* data = new MeshData(3, flags);
    addCubeData(data, width, height, depth);
    addCubeI(data);
    return data;
}

void MeshBuilder::addCubeData(MeshData* data, float width, float height, float depth) {
    float w = width / 2;
    float h = height / 2;
    float d = depth / 2;
    // Front face
    data->addPosition(Vector3f(-w, h, d));
    data->addPosition(Vector3f(w, h, d));
    data->addPosition(Vector3f(w, -h, d));
    data->addPosition(Vector3f(-w, -h, d));

    // Left face
    data->addPosition(Vector3f(-w, -h, d));
    data->addPosition(Vector3f(-w, -h, -d));
    data->addPosition(Vector3f(-w, h, -d));
    data->addPosition(Vector3f(-w, h, d));

    // Back face
    data->addPosition(Vector3f(-w, h, -d));
    data->addPosition(Vector3f(w, h, -d));
    data->addPosition(Vector3f(w, -h, -d));
    data->addPosition(Vector3f(-w, -h, -d));

    // Bottom face
    data->addPosition(Vector3f(w, -h, -d));
    data->addPosition(Vector3f(w, -h, d));
    data->addPosition(Vector3f(-w, -h, d));
    data->addPosition(Vector3f(-w, -h, -d));

    // Right face
    data->addPosition(Vector3f(w, -h, -d));
    data->addPosition(Vector3f(w, -h, d));
    data->addPosition(Vector3f(w, h, d));
    data->addPosition(Vector3f(w, h, -d));

    // Top face
    data->addPosition(Vector3f(-w, h, -d));
    data->addPosition(Vector3f(-w, h, d));
    data->addPosition(Vector3f(w, h, d));
    data->addPosition(Vector3f(w, h, -d));
}

void MeshBuilder::addCubeI(MeshData* data) {
    // Front face
    // B-L triangle
    data->addIndex(0);
    data->addIndex(1);
    data->addIndex(2);
    // T-R
    data->addIndex(2);
    data->addIndex(3);
    data->addIndex(0);

    // Left face
    // B-L triangle
    data->addIndex(4);
    data->addIndex(5);
    data->addIndex(6);
    // T-R
    data->addIndex(6);
    data->addIndex(7);
    data->addIndex(4);

    // Back face
    // B-L triangle
    data->addIndex(8);
    data->addIndex(9);
    data->addIndex(10);
    // T-R
    data->addIndex(10);
    data->addIndex(11);
    data->addIndex(8);

    // Bottom face
    // B-L triangle
    data->addIndex(12);
    data->addIndex(13);
    data->addIndex(14);
    // T-R
    data->addIndex(14);
    data->addIndex(15);
    data->addIndex(12);

    // Right face
    // B-L triangle
    data->addIndex(16);
    data->addIndex(17);
    data->addIndex(18);
    // T-R
    data->addIndex(18);
    data->addIndex(19);
    data->addIndex(16);

    // Top face
    // B-L triangle
    data->addIndex(20);
    data->addIndex(21);
    data->addIndex(22);
    // T-R
    data->addIndex(22);
    data->addIndex(23);
    data->addIndex(20);
}