
#include "Logging.h"

#include "TimeUtils.h"

/*****************************************************************************
 * Logger class
 *****************************************************************************/

bool Logger::saveLogs = false;

std::ofstream Logger::fileOutputStream;

LogType Logger::logLevel = LogType::Debug | LogType::Information | LogType::Warning | LogType::Error;

bool Logger::includeTimeStamp = true;

void Logger::startFileOutput(const std::string& path) {
    // Attempt to open the output stream
    fileOutputStream.open(path.c_str(), std::ofstream::out | std::ofstream::app);
    // Check if success full and act appropriately
    if (fileOutputStream.is_open())
        saveLogs = true;
    else {
        saveLogs = false;
        log("Could not open file: " + path, "Logger", LogType::Error);
    }
}

/* Stops saving the logs */
void Logger::stopFileOutput() {
    // Stop file output if open
    if (fileOutputStream.is_open())
        fileOutputStream.close();
    saveLogs = false;
}

/* Various log functions */
void Logger::log(const std::string& message, const std::string& source, LogType type) {
    // Log only if within the log level
    if (shouldLog(type)) {
        // Build the message
        std::string fullMessage = "[" + logTypeString(type) + "]";
        if (includeTimeStamp)
            fullMessage = "[" + utils_time::getTimeAsString() + "]" + fullMessage;
        if (source != "")
            fullMessage = fullMessage + "[" + source + "]";
        fullMessage += " " + message;
        // Output
        std::cout << fullMessage << std::endl;
        // Output to a file as well if needed
        if (saveLogs) {
            fileOutputStream << fullMessage << std::endl;
            fileOutputStream.flush();
        }
    }
}