#pragma once

#include "Mesh.h"

/*****************************************************************************
 * ShaderInterface class - Stores data about the interface to engine shaders
 *****************************************************************************/

class ShaderInterface {
private:
    /* Mapping for attribute values from the engine to the shader */
    std::map<int, uint32_t> attributeMappings;

public:
    /* Constructor and destructor */
    ShaderInterface() {}
    virtual ~ShaderInterface() {}

    /* Method to add an attribute mapping to this interface */
    inline void addAttributeLocation(MeshData::DataType dataType, uint32_t location) { attributeMappings.insert(std::pair<int, uint32_t>(dataType, location)); }

    /* Returns a mapping (or errors if it has not been assigned) */
    uint32_t getAttributeLocation(MeshData::DataType dataType);
};