#pragma once

#include <string>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class Logger {
    public:
        static void log(LogLevel level, const std::string& message);
};