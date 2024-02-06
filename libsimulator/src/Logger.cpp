// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Logger.hpp"

#include <memory>

namespace Logging
{

Logger& Logger::Instance()
{
    static Logger logger;
    return logger;
}

void Logger::SetDebugCallback(LogCalback&& cb)
{
    debug_msg_cb = cb;
}

void Logger::ClearDebugCallback()
{
    debug_msg_cb = {};
}

void Logger::LogDebugMessage(const std::string& msg)
{
    if(debug_msg_cb) {
        debug_msg_cb(msg);
    }
}

void Logger::SetInfoCallback(LogCalback&& cb)
{
    info_msg_cb = cb;
}

void Logger::ClearInfoCallback()
{
    info_msg_cb = {};
}

void Logger::LogInfoMessage(const std::string& msg)
{
    if(info_msg_cb) {
        info_msg_cb(msg);
    }
}

void Logger::SetWarningCallback(LogCalback&& cb)
{
    warning_msg_cb = cb;
}

void Logger::ClearWarningCallback()
{
    warning_msg_cb = {};
}

void Logger::LogWarningMessage(const std::string& msg)
{
    if(warning_msg_cb) {
        warning_msg_cb(msg);
    }
}

void Logger::SetErrorCallback(LogCalback&& cb)
{
    error_msg_cb = cb;
}

void Logger::ClearErrorCallback()
{
    error_msg_cb = {};
}

void Logger::LogErrorMessage(const std::string& msg)
{
    if(error_msg_cb) {
        error_msg_cb(msg);
    }
}

void Logger::ClearAllCallbacks()
{
    ClearDebugCallback();
    ClearInfoCallback();
    ClearWarningCallback();
    ClearErrorCallback();
}

} // namespace Logging
