#pragma once

/*****************************************************************************
 * ShaderInterface class - Stores data about the interface to engine shaders
 *****************************************************************************/

class ShaderInterface {
public:
    /* The locations for attributes in the shaders */
    static const unsigned int ATTRIBUTE_LOCATION_POSITION      = 0;
    static const unsigned int ATTRIBUTE_LOCATION_COLOUR        = 1;  // Needs to be the same for now
    static const unsigned int ATTRIBUTE_LOCATION_TEXTURE_COORD = 1;
    static const unsigned int ATTRIBUTE_LOCATION_NORMAL        = 2;
    static const unsigned int ATTRIBUTE_LOCATION_TANGENT       = 3;
    static const unsigned int ATTRIBUTE_LOCATION_BITANGENT     = 4;
    static const unsigned int ATTRIBUTE_LOCATION_BONE_INDICES  = 5;
    static const unsigned int ATTRIBUTE_LOCATION_BONE_WEIGHTS  = 6;
};