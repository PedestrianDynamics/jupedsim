// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Logger.hpp"

class LogCallbackOwner
{
public:
    using LogCallback = Logging::Logger::LogCallback;

    LogCallback debug{};
    LogCallback info{};
    LogCallback warning{};
    LogCallback error{};

public:
    static LogCallbackOwner& Instance();
};
