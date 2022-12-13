#pragma once

#include <sstream>
#include <string>
#include <vector>

/*****************************************************************************
 * utils_string namespace - Various string utilities
 *****************************************************************************/

namespace utils_string {
    /* Converts some data to a string */
    template <typename T>
    inline std::string str(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    /* Splits a string based on a given delimeter */
    std::vector<std::string> split(const std::string& strValue, char delimeter);

    /* Splits a string based on the last occurrence of a delimeter - if
       delimeter not present, the full string is returned as the only
       element  */
    std::vector<std::string> splitLast(const std::string& strValue, const std::string& delimeter);

    /* Converts a string to a numeric value */
    int toInt(const std::string& value);
    unsigned int toUInt(const std::string& value);
    float toFloat(const std::string& value);
    double toDouble(const std::string& value);
}  // namespace utils_string
