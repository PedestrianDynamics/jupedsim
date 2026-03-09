// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <functional>
#include <string>

class LogCallbackOwner
{
public:
    using LogCallback = std::function<void(const std::string&)>;

    LogCallback debug{};
    LogCallback info{};
    LogCallback warning{};
    LogCallback error{};

public:
    static LogCallbackOwner& Instance();
};
