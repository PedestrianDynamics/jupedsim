// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "format_any.hpp"

#include <any>
#include <type_traits>
#include <utility>

class CustomModelState
{
    std::any _value{};
    FormatFn _format{};

public:
    template <typename T>
        requires(!std::is_same_v<std::decay_t<T>, CustomModelState>)
    explicit CustomModelState(T&& value)
        : _value(std::forward<T>(value)), _format(make_format_fn<T>())
    {
        using Stored = std::decay_t<T>;
        static_assert(
            std::is_copy_constructible_v<Stored>,
            "CustomModelState payloads must be copy-constructible");
    }

    template <typename T>
    T& get()
    {
        return std::any_cast<T&>(_value);
    }

    template <typename T>
    const T& get() const
    {
        return std::any_cast<const T&>(_value);
    }

    template <typename T>
    void set(T&& new_value)
    {
        using Stored = std::decay_t<T>;
        std::any_cast<Stored&>(_value) = std::forward<T>(new_value);
    }

    friend struct fmt::formatter<CustomModelState>;
};

template <>
struct fmt::formatter<CustomModelState> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const CustomModelState& value, fmt::format_context& ctx) const
    {
        return value._format(value._value, ctx);
    }
};
