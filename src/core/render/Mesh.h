#pragma once

#include <map>

#include "../Sphere.h"
#include "Colour.h"
#include "GraphicsPipeline.h"
#include "VertexBuffer.h"

// Forward declaration
class ShaderInterface;

/*****************************************************************************
 * MeshData class - Stores data required for constructing a mesh and helps
 *                  during construction
 *****************************************************************************/

// TODO: Avoid using push_back where possible - Custom engine format would
//       is one way to do this while still allowing data to be kept
//       separate/interleaved and keeping bounding sphere information
// TODO: Allow 3 colour components instead of 4?

class MeshData {
public:
    /* Stores data for a rendering parts of the stored mesh separately
       due to needing different materials */
    struct SubData {
        // Index of the material
        uint32_t materialIndex;

        // First index - base index within the index buffer to use when
        // rendering
        uint32_t firstIndex;

        // Vertex offset - added to vertex index before indexing into the
        // vertex buffer while rendering
        uint32_t vertexOffset;
    };

    /* Data for a bounding sphere (for frustum culling) */
    struct BoundingSphere {
        Vector3f center;
        float radius;
    };

    /* Various types of data this can store */
    enum DataType {
        POSITION       = 1,
        COLOUR         = 2,
        TEXTURE_COORD  = 3,
        NORMAL         = 4,
        TANGENT        = 5,
        BITANGENT      = 6,
        BONE_INDEX     = 7,
        BONE_WEIGHT    = 8,
        MATERIAL_INDEX = 9,
        VERTEX_OFFSET  = 10,
    };

    /* Flags for splitting up data */
    enum SeparateFlags {
        SEPARATE_NONE           = 0,
        SEPARATE_POSITIONS      = 1 << 0,
        SEPARATE_COLOURS        = 1 << 1,
        SEPARATE_TEXTURE_COORDS = 1 << 2,
        SEPARATE_NORMALS        = 1 << 3,
        SEPARATE_TANGENTS       = 1 << 4,
        SEPARATE_BITANGENTS     = 1 << 5,
        // These are always separate for now
        // SEPARATE_BONE_INDICES     = 1 << 6,
        // SEPARATE_BONE_WEIGHT      = 1 << 7,
        // SEPARATE_MATERIAL_INDICES = 1 << 8,
        // SEPARATE_INDEX_OFFSETS    = 1 << 9,
    };

    /* Info about data types that can be stored in a single buffer */
    struct DataTypeInfo {
        // Flag used to separate the data
        SeparateFlags separateFlag;
        // Size of this data in bytes
        uint32_t size;
        // Format this data takes according to Vulkan
        VkFormat format;
    };

    /* Numbers of dimensions */
    static const unsigned int DIMENSIONS_2D = 2;
    static const unsigned int DIMENSIONS_3D = 3;

private:
    /* Known datatypes and their info */
    static std::map<int, DataTypeInfo> datatypeInfoMaps;

    /* Returns info about a known datatype or errors if not found */
    static DataTypeInfo getDataTypeInfo(unsigned int numDimensions, DataType dataType);

    /* Number of dimensions data will be stored for (effects vertex input
       bindings) */
    unsigned int numDimensions;

    /* Raw data for this mesh - all data may be kept interleaved in 'others'
       unless they are requested to be separate */

    // Standard info
    std::vector<float> positions;
    std::vector<float> colours;
    std::vector<float> textureCoords;
    std::vector<float> normals;
    std::vector<float> tangents;
    std::vector<float> bitangents;
    std::vector<float> others;
    std::vector<uint32_t> indices;

    // Bone info
    std::vector<uint32_t> boneIndices;
    std::vector<float> boneWeights;

    // Info that may be required for ray tracing to give material/sub data
    // information on a per vertex basis
    std::vector<uint32_t> materialIndices;
    std::vector<uint32_t> offsetIndices;  // Contains a primitive offset followed by the vertexOffset

    // SubData instances for any parts with different materials
    std::vector<SubData> subData;

    /* For computing the max/min extents of added vertices (when computing a
       bounding sphere) */
    float minX = 1000000000;
    float maxX = -1000000000;
    float minY = 1000000000;
    float maxY = -1000000000;
    float minZ = 1000000000;
    float maxZ = -1000000000;

    /* Flags specifying whether certain data should be separated */
    SeparateFlags separateFlags;

public:
    /* Constructor and destructor  */
    MeshData(unsigned int numDimensions, SeparateFlags separateFlags = SEPARATE_NONE) : numDimensions(numDimensions), separateFlags(separateFlags) {}
    virtual ~MeshData() {}

    /* Calculates and returns a sphere bounding this mesh */
    // TODO: Allow computation for sub mesh's
    Sphere calculateBoundingSphere();

    /* Methods to add data */
    void addPosition(Vector2f position);
    void addPosition(Vector3f position);
    void addColour(Colour colour);
    void addTextureCoord(Vector2f textureCoord);
    void addNormal(Vector3f normal);
    void addTangent(Vector3f tangent);
    void addBitangent(Vector3f bitangent);
    void addIndex(uint32_t index);

    void addBoneData(uint32_t boneIndex, float boneWeight) {
        boneIndices.push_back(boneIndex);
        boneWeights.push_back(boneWeight);
    }

    inline void addMaterialIndex(uint32_t materialIndex) { materialIndices.push_back(materialIndex); }

    inline void addOffsetIndex(uint32_t indexOffset, uint32_t vertexOffset) {
        // Want to store offset in primitives (triangles in this case)
        offsetIndices.push_back(indexOffset / 3);
        offsetIndices.push_back(vertexOffset);
    }

    inline void addSubData(SubData& data) { subData.push_back(data); }
    inline void addSubData(uint32_t materialIndex, uint32_t firstIndex, uint32_t vertexOffset) { subData.push_back({materialIndex, firstIndex, vertexOffset}); }

    /* Methods to check whether certain data should be separated */
    inline bool separatePositions() { return separateFlags & SeparateFlags::SEPARATE_POSITIONS; }
    inline bool separateColours() { return separateFlags & SeparateFlags::SEPARATE_COLOURS; }
    inline bool separateTextureCoords() { return separateFlags & SeparateFlags::SEPARATE_TEXTURE_COORDS; }
    inline bool separateNormals() { return separateFlags & SeparateFlags::SEPARATE_NORMALS; }
    inline bool separateTangents() { return separateFlags & SeparateFlags::SEPARATE_TANGENTS; }
    inline bool separateBitangents() { return separateFlags & SeparateFlags::SEPARATE_BITANGENTS; }

    /* Methods to check whether data has been provided */
    inline bool hasPositions() { return positions.size() > 0; }
    inline bool hasColours() { return colours.size() > 0; }
    inline bool hasTextureCoords() { return textureCoords.size() > 0; }
    inline bool hasNormals() { return normals.size() > 0; }
    inline bool hasTangents() { return tangents.size() > 0; }
    inline bool hasBitangents() { return bitangents.size() > 0; }
    inline bool hasOthers() { return others.size() > 0; }
    inline bool hasIndices() { return indices.size() > 0; }
    inline bool hasBones() { return boneIndices.size() > 0; }
    inline bool hasMaterialIndices() { return materialIndices.size() > 0; }
    inline bool hasOffsetIndices() { return offsetIndices.size() > 0; }

    std::vector<float>& getPositions() { return positions; }
    std::vector<float>& getColours() { return colours; }
    std::vector<float>& getTextureCoords() { return textureCoords; }
    std::vector<float>& getNormals() { return normals; }
    std::vector<float>& getTangents() { return tangents; }
    std::vector<float>& getBitangents() { return bitangents; }
    std::vector<float>& getOthers() { return others; }
    std::vector<uint32_t>& getIndices() { return indices; }
    std::vector<uint32_t>& getBoneIndices() { return boneIndices; }
    std::vector<float>& getBoneWeights() { return boneWeights; }
    std::vector<uint32_t>& getMaterialIndices() { return materialIndices; }
    std::vector<uint32_t>& getOffsetIndices() { return offsetIndices; }
    inline bool hasSubData() { return subData.size() > 0; }
    inline size_t getSubDataCount() { return subData.size(); }
    inline SubData& getSubData(unsigned int index) { return subData[index]; }

    /* Static method to construct vertex input bindings and attributes given the
       required data and whether they should be separated from the others */
    static GraphicsPipeline::VertexInputDescription computeVertexInputDescription(unsigned int numDimensions, std::vector<DataType> requiredData, SeparateFlags flags, ShaderInterface shaderInterface);
};