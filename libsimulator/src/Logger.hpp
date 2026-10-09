// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/format.h>

#include <functional>
#include <string>

namespace logging
{

class Logger
{
public:
    using LogCallback = std::function<void(const std::string& msg)>;

private:
    LogCallback _debug_msg_cb{};
    LogCallback _info_msg_cb{};
    LogCallback _warning_msg_cb{};
    LogCallback _error_msg_cb{};

public:
    static Logger& instance();
    void set_debug_callback(LogCallback&& cb);
    void clear_debug_callback();
    void log_debug_message(const std::string& msg);
    void set_info_callback(LogCallback&& cb);
    void clear_info_callback();
    void log_info_message(const std::string& msg);
    void set_warning_callback(LogCallback&& cb);
    void clear_warning_callback();
    void log_warning_message(const std::string& msg);
    void set_error_callback(LogCallback&& cb);
    void clear_error_callback();
    void log_error_message(const std::string& msg);
    void clear_all_callbacks();

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger& other) = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger(Logger&& other) = delete;
    Logger& operator=(Logger&& other) = delete;
};

enum class Level { Debug, Info, Warning, Error, Off };

} // namespace logging

// Convenience macros to add log messages with compile time check to validate
// used format string.I have not been able to get the FMT_STRING macro to work
// in a variadic template so I used variadic macros which now requires to
// silence a modernization warning. Further the macro relies on the clang / gcc
// no standard extension to use ##__VA_ARGS to solve the issue of trailing ','
// if the var args are empty. MSVC eliminates the trailing ',' silently but
// errors out on the clang / gcc variant.

#ifdef _MSC_VER
// NOLINTNEXTLINE
#define LOG_AT_LEVEL(level, FormatString, ...)                                                     \
    logging::Logger::instance().log_##level##_message(                                             \
        fmt::format(FMT_STRING(FormatString), __VA_ARGS__))
// NOLINTNEXTLINE
#define LOG_DEBUG(FormatString, ...) LOG_AT_LEVEL(debug, FormatString, __VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_INFO(FormatString, ...) LOG_AT_LEVEL(info, FormatString, __VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_WARNING(FormatString, ...) LOG_AT_LEVEL(warning, FormatString, __VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_ERROR(FormatString, ...) LOG_AT_LEVEL(error, FormatString, __VA_ARGS__)
#else
// NOLINTNEXTLINE
#define LOG_AT_LEVEL(level, FormatString, ...)                                                     \
    logging::Logger::instance().log_##level##_message(                                             \
        fmt::format(FMT_STRING(FormatString), ##__VA_ARGS__))
// NOLINTNEXTLINE
#define LOG_DEBUG(FormatString, ...) LOG_AT_LEVEL(debug, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_INFO(FormatString, ...) LOG_AT_LEVEL(info, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_WARNING(FormatString, ...) LOG_AT_LEVEL(warning, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_ERROR(FormatString, ...) LOG_AT_LEVEL(error, FormatString, ##__VA_ARGS__)
#endif
