// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>

#include <string>

/// __str__ / __repr__ implementation for binding classes, reusing the wrapped
/// type's C++ fmt::formatter.
template <typename T>
std::string formatViaFmt(const T& value)
{
    return fmt::format("{}", value);
}
