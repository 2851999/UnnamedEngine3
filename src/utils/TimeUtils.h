#pragma once

#include <GLFW/glfw3.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>

#include "StringUtils.h"

/*****************************************************************************
 * utils_time namespace - Various time utilities
 *****************************************************************************/

namespace utils_time {
    /* Returns the current time as a string */
    std::string getTimeAsString();

    /* Returns the time since GLFW was initialised in seconds */
    inline double getSeconds() { return glfwGetTime(); }

    /* Returns the time since GLFW was initialised in milliseconds */
    inline double getMilliseconds() { return getSeconds() * 1000.0; }

    /* Pauses thread for some time in seconds */
    void wait(double seconds);
}  // namespace utils_time