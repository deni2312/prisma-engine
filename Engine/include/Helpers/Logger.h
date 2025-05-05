#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

#include "../GlobalData/InstanceData.h"

namespace Prisma {
// Enum to represent different log levels (in uppercase)
enum class LogLevel {
    INFO,
    ERRORS,
    WARN,
    GENERIC
};

class Logger : public InstanceData<Logger> {
public:
    Logger();

    // Method to log a message at a specific log level
    void log(LogLevel level, const std::string& message);

    // Method to get logs for a specific level
    const std::string& getLogs(LogLevel level) const;

private:
    // Map to store logs by log level
    std::map<LogLevel, std::string> logMap;

    // Convert LogLevel enum to string representation (uppercase)
    std::string toString(LogLevel level) const;
};
} // namespace Prisma