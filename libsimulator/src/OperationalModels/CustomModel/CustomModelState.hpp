// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "FormatAny.hpp"
#include "Point.hpp"

#include <fmt/core.h>

#include <any>
#include <type_traits>
#include <utility>

class CustomModelState
{
public:
    /// Position cache kept outside the type-erased payload so the framework can read it
    /// without touching the payload (e.g. without acquiring the GIL for Python objects).
    /// The owning model must keep it in sync with the payload's own position field.
    Point position{};

private:
    std::any value{};
    FormatFn format{};

public:
    template <typename T>
        requires(!std::is_same_v<std::decay_t<T>, CustomModelState>)
    CustomModelState(T&& value) : value(std::forward<T>(value)), format(makeFormatFn<T>())
    {
        using Stored = std::decay_t<T>;
        static_assert(
            std::is_copy_constructible_v<Stored>,
            "CustomModelState payloads must be copy-constructible");
    }

    template <typename T>
    T& Get()
    {
        return std::any_cast<T&>(value);
    }

    template <typename T>
    const T& Get() const
    {
        return std::any_cast<const T&>(value);
    }

    template <typename T>
    void Set(T&& newValue)
    {
        using Stored = std::decay_t<T>;
        std::any_cast<Stored&>(value) = std::forward<T>(newValue);
    }

    friend struct fmt::formatter<CustomModelState>;
};

template <>
struct fmt::formatter<CustomModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    auto format(const CustomModelState& value, fmt::format_context& ctx) const
    {
        return value.format(value.value, ctx);
    }
};
