#pragma once

#include <string>
#include <vector>

/*****************************************************************************
 * utils_file namespace - Various file utilities
 *****************************************************************************/

namespace utils_file {
    /* Reads a file in binary mode into a vector of chars */
    std::vector<char> readBinChar(const std::string& path);
};  // namespace utils_file