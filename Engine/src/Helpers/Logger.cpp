#include "Helpers/Logger.h"

Prisma::Logger::Logger() {
        // Initialize the map with empty log vectors for each log level
        logMap[LogLevel::INFO] = {""};
        logMap[LogLevel::ERRORS] = {""};
        logMap[LogLevel::WARN] = {""};
        logMap[LogLevel::GENERIC] = {""};
}

void Prisma::Logger::log(LogLevel level, const std::string& message) {
        std::stringstream logEntry;
        logEntry << "[" << toString(level) << "] " << message;
        logMap[level] = logMap[level] + logEntry.str() + "\n";
}

const std::string& Prisma::Logger::getLogs(LogLevel level) const {
        return logMap.at(level);
}

std::string Prisma::Logger::toString(LogLevel level) const {
        switch (level) {
                case LogLevel::INFO:
                        return "INFO";
                case LogLevel::ERRORS:
                        return "ERROR";
                case LogLevel::WARN:
                        return "WARN";
                case LogLevel::GENERIC:
                        return "GENERIC";
                default:
                        return "UNKNOWN";
        }
}