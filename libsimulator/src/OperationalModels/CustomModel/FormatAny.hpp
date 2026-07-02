// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

/// @file FormatAny.hpp
/// Helpers for formatting values stored in std::any.

#include <fmt/core.h>

#include <any>
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

using FormatFn = fmt::format_context::iterator (*)(const std::any& value, fmt::format_context& ctx);

template <typename T>
concept ToStringResult = std::same_as<std::remove_cvref_t<T>, std::string> ||
                         std::same_as<std::remove_cvref_t<T>, std::string_view> ||
                         std::same_as<std::remove_cvref_t<T>, const char*>;

template <typename T>
concept HasToString = requires(const T& value) {
    { value.ToString() } -> ToStringResult;
};

template <typename T>
auto makeFormatFn()
{
    using Stored = std::decay_t<T>;
    if constexpr(fmt::is_formattable<Stored, char>::value) {
        return [](const std::any& any, fmt::format_context& ctx) {
            return fmt::format_to(ctx.out(), "{}", std::any_cast<const Stored&>(any));
        };
    } else if constexpr(HasToString<Stored>) {
        return [](const std::any& any, fmt::format_context& ctx) {
            const auto& value = std::any_cast<const Stored&>(any);
            return fmt::format_to(ctx.out(), "{}", value.ToString());
        };
    } else {
        return [](const std::any& any, fmt::format_context& ctx) {
            const auto& value = std::any_cast<const Stored&>(any);
            const auto addr = static_cast<const void*>(std::addressof(value));
            return fmt::format_to(ctx.out(), "<{}@{}>", any.type().name(), addr);
        };
    }
}
