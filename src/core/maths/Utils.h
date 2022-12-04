#pragma once

/*****************************************************************************
 * Various maths utilities
 *****************************************************************************/

#include <cmath>

namespace utils_maths {
    const float PI = 3.14159265f;

    /* Angle conversions */
    inline float toRadians(float degrees) { return degrees * (PI / 180.0f); }
    inline float toDegrees(float radians) { return radians * (180.0f / PI); }

    /* Clamp a value between a given minimum and maximum */
    template <typename T>
    inline T clamp(T value, T min, T max) {
        return value < min ? min : value > max ? max
                                               : value;
    }

    /* Find min/max of two values */
    template <typename T>
    inline T min(T value1, T value2) {
        return value1 < value2 ? value1 : value2;
    }

    template <typename T>
    inline T max(T value1, T value2) {
        return value1 > value2 ? value1 : value2;
    }

    /* Absolute value of a value */
    template <typename T>
    inline T abs(T value) {
        return value < 0 ? -value : value;
    }

    /* Linear interpolation between two values */
    template <typename T>
    inline T lerp(T valueA, T valueB, T factor) { return (valueA + ((valueB - valueA) * factor)); }
}  // namespace utils_maths