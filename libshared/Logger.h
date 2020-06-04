#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string_view>

namespace Logging
{
enum class Level { Debug, Info, Warning, Error, Off };

void Setup();
void Teardown();
void Debug(std::string_view msg);
void Info(std::string_view msg);
void Warning(std::string_view msg);
void Error(std::string_view msg);
void SetLogLevel(Level level);

struct Guard {
    Guard();
    ~Guard();
};
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
    Logging::Level(fmt::format(FMT_STRING(FormatString), __VA_ARGS__))
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
    Logging::Level(fmt::format(FMT_STRING(FormatString), ##__VA_ARGS__))
// NOLINTNEXTLINE
#define LOG_DEBUG(FormatString, ...) __LOG(Debug, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_INFO(FormatString, ...) __LOG(Info, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_WARNING(FormatString, ...) __LOG(Warning, FormatString, ##__VA_ARGS__)
// NOLINTNEXTLINE
#define LOG_ERROR(FormatString, ...) __LOG(Error, FormatString, ##__VA_ARGS__)
#endif
