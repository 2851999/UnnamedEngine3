#pragma once

#include <iostream>
#include <fstream>

/* Log types */
enum LogType {
    Debug = 1,
    Information = 2,
    Warning = 4,
    Error = 8
};

/* Provides bitmask functionality for the log levels above */
inline LogType operator|(LogType a, LogType b) {
    return static_cast<LogType>(static_cast<int>(a) | static_cast<int>(b));
}

/*****************************************************************************
 * Logger class - Used for logging
 *****************************************************************************/

class Logger {
private:
    /* Used to get a string representation of a log type */
    static inline std::string logTypeString(LogType type) {
        if (type & LogType::Debug)
            return "DEBUG";
        else if (type & LogType::Information)
            return "INFO";
        else if (type & LogType::Warning)
            return "WARNING";
        else if (type & LogType::Error)
            return "ERROR";
        return "DEBUG";
    }

    /* States whether logs should be saved to a file */
    static bool saveLogs;

    /* File ouput stream to put logs into */
    static std::ofstream fileOutputStream;

    /* Determines whether a log should be printed based on the current log level */
    static inline bool shouldLog(LogType type) { return logLevel & type; }

public:
    /* Current log level */
    static LogType logLevel;

    /* Should include timestamp in logs? */
    static bool includeTimeStamp;

    /* Starts saving the logs */
    static void startFileOutput(const std::string& path);

    /* Stops saving the logs */
    static void stopFileOutput();

    /* Various log functions */
    static void log(const std::string& message, LogType type);
    static inline void log(const std::string& message) { log(message, LogType::Debug); }
    static inline void log(const std::string& message, const std::string& source, LogType type) { log("[" + source + "]" + message, type); }
};