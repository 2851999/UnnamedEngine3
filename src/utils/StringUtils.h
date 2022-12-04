#pragma once

#include <sstream>
#include <string>

/*****************************************************************************
 * Various string utilities
 *****************************************************************************/

namespace utils_string {
    /* Converts some data to a string */
    template <typename T>
    inline std::string str(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
}  // namespace utils_string
