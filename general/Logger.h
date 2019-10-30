#pragma once

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
