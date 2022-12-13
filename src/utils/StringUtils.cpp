#pragma once

#include "StringUtils.h"

#include <stdexcept>

#include "Logging.h"

/*****************************************************************************
 * utils_string namespace
 *****************************************************************************/

std::vector<std::string> utils_string::split(const std::string& strValue, char delimeter) {
    std::stringstream stringStream(strValue);
    std::string item;
    std::vector<std::string> split;
    while (std::getline(stringStream, item, delimeter))
        split.push_back(item);
    return split;
}

std::vector<std::string> utils_string::splitLast(const std::string& strValue, const std::string& delimeter) {
    std::vector<std::string> split;

    // Find last occurrence
    size_t foundPos = strValue.find_last_of(delimeter);

    if (foundPos != std::string::npos)
        split = {strValue.substr(0, foundPos), strValue.substr(foundPos + 1)};
    else
        split = {strValue};
    return split;
}

/* Helper for handling errors with conversion functions */
template <typename T>
inline T convertToNumeric(const std::string& value, const std::string& errorMessage, T (*conversionFunc)(const std::string&, size_t*)) {
    std::size_t pos;
    T result   = 0;
    bool valid = true;

    try {
        result = conversionFunc(value, &pos);
        valid  = pos == value.length();
    } catch (const std::invalid_argument& e) {
        // Trick usage
        e;
        valid = false;
    } catch (const std::out_of_range& e) {
        // Trick usage
        e;
        valid = false;
    }
    if (! valid)
        Logger::logAndThrowError(errorMessage, "Logger");

    return result;
}

int utils_string::toInt(const std::string& value) {
    return convertToNumeric<int>(value, "Cannot convert the string '" + value + "' to an integer", [](const std::string& strValue, size_t* pos) { return std::stoi(strValue, pos); });
}

unsigned int utils_string::toUInt(const std::string& value) {
    // No stou so convert from next closest
    return convertToNumeric<unsigned int>(value, "Cannot convert the string '" + value + "' to an unsigned integer", [](const std::string& strValue, size_t* pos) { return static_cast<unsigned int>(std::stoul(strValue, pos)); });
}

float utils_string::toFloat(const std::string& value) {
    return convertToNumeric<float>(value, "Cannot convert the string '" + value + "' to a float", [](const std::string& strValue, size_t* pos) { return std::stof(strValue, pos); });
}

double utils_string::toDouble(const std::string& value) {
    return convertToNumeric<double>(value, "Cannot convert the string '" + value + "' to a double", [](const std::string& strValue, size_t* pos) { return std::stod(strValue, pos); });
}