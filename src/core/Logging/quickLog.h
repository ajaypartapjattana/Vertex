#pragma once

#include "logger.h"

#ifdef _VALIDATE

#define LOG_INTERNAL(level, fmt, ...)                                             \
    do {                                                                          \
        char buffer[128];                                                         \
        int len = std::snprintf(buffer, sizeof(buffer), fmt, ##__VA_ARGS__);      \
        if (len > 0) {                                                            \
            size_t write_len = std::min((size_t)len, sizeof(buffer) - 1);         \
            sgb::logger::instance().log(level, buffer, write_len);                \
        }                                                                         \
    } while (0)

#define LOG_TRACE(...) LOG_INTERNAL(logLevel::TRACE, __VA_ARGS__)
#define LOG_INFO(...)  LOG_INTERNAL(logLevel::INFO,  __VA_ARGS__)
#define LOG_WARN(...)  LOG_INTERNAL(logLevel::WARN,  __VA_ARGS__)
#define LOG_ERROR(...) LOG_INTERNAL(logLevel::ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG_INTERNAL(logLevel::FATAL, __VA_ARGS__)

#else

#define LOG_TRACE(...)
#define LOG_INFO(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...)

#endif