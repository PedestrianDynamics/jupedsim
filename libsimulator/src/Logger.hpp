// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <functional>
#include <memory>
#include <string_view>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace Logging
{

class Logger
{
public:
    using LogCalback = std::function<void(const std::string& msg)>;

private:
    LogCalback debug_msg_cb{};
    LogCalback info_msg_cb{};
    LogCalback warning_msg_cb{};
    LogCalback error_msg_cb{};

public:
    static Logger& Instance();
    void SetDebugCallback(LogCalback&& cb);
    void ClearDebugCallback();
    void LogDebugMessage(const std::string& msg);
    void SetInfoCallback(LogCalback&& cb);
    void ClearInfoCallback();
    void LogInfoMessage(const std::string& msg);
    void SetWarningCallback(LogCalback&& cb);
    void ClearWarningCallback();
    void LogWarningMessage(const std::string& msg);
    void SetErrorCallback(LogCalback&& cb);
    void ClearErrorCallback();
    void LogErrorMessage(const std::string& msg);
    void ClearAllCallbacks();

private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger& other) = delete;
    Logger& operator=(const Logger& other) = delete;
    Logger(Logger&& other) = delete;
    Logger& operator=(Logger&& other) = delete;
};

enum class Level { Debug, Info, Warning, Error, Off };

} // namespace Logging

// Convenience macros to add log messages with compile time check to validate
// used format string.I have not been able to get the FMT_STRING macro to work
// in a variadic template so I used variadic macros which now requires to
// silence a modernization warning. Further the macro relies on the clang / gcc
// no standard extension to use ##__VA_ARGS to solve the issue of trailing ','
// if the var args are empty. MSVC eliminates the trailing ',' silently but
// errors out on the clang / gcc variant.

#ifdef _MSC_VER
// NOLINTNEXTLINE
#define __LOG(Level, FormatString, ...)                                                            \
    Logging::Logger::Instance().Log##Level##Message(                                               \
        fmt::format(FMT_STRING(FormatString), __VA_ARGS__))
// NOLINTNEXTLINE
#define LOG_DEBUG(FormatString, ...) __LOG(Debug, FormatString, __VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_INFO(FormatString, ...) __LOG(Info, FormatString, __VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_WARNING(FormatString, ...) __LOG(Warning, FormatString, __VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_ERROR(FormatString, ...) __LOG(Error, FormatString, __VA_ARGS__)
#else
// NOLINTNEXTLINE
#define __LOG(Level, FormatString, ...)                                                            \
    Logging::Logger::Instance().Log##Level##Message(                                               \
        fmt::format(FMT_STRING(FormatString), ##__VA_ARGS__))
// NOLINTNEXTLINE
#define LOG_DEBUG(FormatString, ...) __LOG(Debug, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_INFO(FormatString, ...) __LOG(Info, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_WARNING(FormatString, ...) __LOG(Warning, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_ERROR(FormatString, ...) __LOG(Error, FormatString, ##__VA_ARGS__)
#endif
