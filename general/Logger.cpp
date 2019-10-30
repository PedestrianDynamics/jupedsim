#include "Logger.h"

#include <spdlog/spdlog.h>

namespace Logging
{
void Setup()
{
    spdlog::set_level(spdlog::level::trace);
}

void Teardown()
{
    spdlog::shutdown();
}

void Debug(std::string_view msg)
{
    spdlog::debug(msg);
}

void Info(std::string_view msg)
{
    spdlog::info(msg);
}

void Warning(std::string_view msg)
{
    spdlog::warn(msg);
}

void Error(std::string_view msg)
{
    spdlog::error(msg);
}

void SetLogLevel(Level level)
{
    switch(level) {
        case Level::Debug:
            spdlog::set_level(spdlog::level::debug);
            break;
        case Level::Info:
            spdlog::set_level(spdlog::level::info);
            break;
        case Level::Warning:
            spdlog::set_level(spdlog::level::warn);
            break;
        case Level::Error:
            spdlog::set_level(spdlog::level::err);
            break;
        case Level::Off:
            spdlog::set_level(spdlog::level::off);
            break;
    }
}

Guard::Guard()
{
    Setup();
}

Guard::~Guard()
{
    Teardown();
}

} // namespace Logging
