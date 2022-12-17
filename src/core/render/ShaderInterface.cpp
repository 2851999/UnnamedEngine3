#include "ShaderInterface.h"

/*****************************************************************************
 * ShaderInterface class
 *****************************************************************************/

uint32_t ShaderInterface::getAttributeLocation(MeshData::DataType dataType) {
    if (attributeMappings.count(dataType) == 0)
        Logger::logAndThrowError("Failed to obtain attribute mapping for datatype " + utils_string::str(dataType), "ShaderInterface");
    return attributeMappings.at(dataType);
}