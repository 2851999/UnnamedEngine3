#include "FileUtils.h"

#include <fstream>

#include "Logging.h"

/*****************************************************************************
 * utils_file namespace
 *****************************************************************************/

std::vector<char> utils_file::readBinChar(const std::string& path) {
    // Attempt to open the file (Start at end as can tell size of file from
    // position)
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (! file.is_open())
        Logger::logAndThrowError("Failed to open file '" + path + "'", "utils_file");

    // Obtain file size and allocate buffer
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    // Now read from beginning
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}