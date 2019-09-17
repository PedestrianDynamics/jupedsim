#include "Logger.h"

#include <spdlog/spdlog.h>

namespace Logging {

void Setup() {
    spdlog::set_level(spdlog::level::trace);
}

void Teardown() {
    spdlog::shutdown();
}

void Debug(std::string_view msg){
    spdlog::debug(msg);
}

void Info(std::string_view msg){
    spdlog::info(msg);
}

void Warning(std::string_view msg){
    spdlog::warn(msg);
}

void Error(std::string_view msg){
    spdlog::critical(msg);
}

Guard::Guard() {
    Setup();
}

Guard::~Guard() {
    Teardown();
}

}
