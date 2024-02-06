// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <fmt/core.h>
#include <fmt/format.h>
#include <stdexcept>

class SimulationError : public std::runtime_error
{
public:
    template <typename... Args>
    SimulationError(const char* msg, const Args&... args)
        : std::runtime_error(fmt::format(fmt::runtime(msg), args...))
    {
    }
};
