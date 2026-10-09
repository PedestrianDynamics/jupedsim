// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "logger.hpp"

class LogCallbackOwner
{
public:
    using LogCallback = logging::Logger::LogCallback;

    LogCallback debug{};
    LogCallback info{};
    LogCallback warning{};
    LogCallback error{};

public:
    static LogCallbackOwner& instance();
};
