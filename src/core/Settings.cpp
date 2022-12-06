
#include "Settings.h"

#include "../utils/Logging.h"
#include "../utils/StringUtils.h"

/*****************************************************************************
 * VideoResolution namespace
 *****************************************************************************/

std::string VideoResolution::toString(const Vector2i& resolution) {
    return utils_string::str(resolution.getX()) + "x" + utils_string::str(resolution.getY());
}

Vector2i VideoResolution::fromString(const std::string& resolution) {
    std::vector<std::string> split = utils_string::split(resolution, 'x');

    if (split.size() != 2)
        Logger::logAndThrowError("Invalid resolution '" + resolution + "'", "VideoResolution");

    // Convert the two components
    return Vector2i(utils_string::toInt(split[0]), utils_string::toInt(split[1]));
}