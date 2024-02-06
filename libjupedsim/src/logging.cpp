// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/logging.h"

#include <Logger.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Logging
////////////////////////////////////////////////////////////////////////////////////////////////////
JUPEDSIM_API void JPS_Logging_SetDebugCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetDebugCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearDebugCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetInfoCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetInfoCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearInfoCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetWarningCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetWarningCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearWarningCallback();
    }
}

JUPEDSIM_API void JPS_Logging_SetErrorCallback(JPS_LoggingCallBack callback, void* userdata)
{
    if(callback) {
        Logging::Logger::Instance().SetErrorCallback(
            [callback, userdata](const std::string& msg) { callback(msg.c_str(), userdata); });
    } else {
        Logging::Logger::Instance().ClearErrorCallback();
    }
}
