// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "FormatAny.hpp"

#include <any>
#include <type_traits>
#include <utility>

/// Type-erased update payload for CustomModel implementations.
///
/// CustomModelUpdate is returned from CustomModel::ComputeNewPosition and later consumed by
/// CustomModel::ApplyUpdate. Keeping update data separate from CustomModelData preserves JuPedSim's
/// existing two-phase operational update flow: first compute all updates from the current state,
/// then apply them.
///
/// Like CustomModelData, the payload is stored by value in std::any. Payload types must be
/// copy-constructible, access through Get<T>() requires the exact stored type, and mismatches throw
/// std::bad_any_cast.
///
/// Formatting follows the same rules as CustomModelData: fmt formatter first, then public
/// const-callable ToString() returning std::string, std::string_view, or const char*, then a
/// diagnostic <type@address> fallback.
class CustomModelUpdate
{
private:
    std::any value{};
    FormatFn format{};

public:
    template <typename T>
        requires(!std::is_same_v<std::decay_t<T>, CustomModelUpdate>)
    CustomModelUpdate(T&& value) : value(std::forward<T>(value)), format(makeFormatFn<T>())
    {
        using Stored = std::decay_t<T>;
        static_assert(
            std::is_copy_constructible_v<Stored>,
            "CustomModelUpdate payloads must be copy-constructible");
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

    friend struct fmt::formatter<CustomModelUpdate>;
};

template <>
struct fmt::formatter<CustomModelUpdate> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const CustomModelUpdate& value, fmt::format_context& ctx) const
    {
        return value.format(value.value, ctx);
    }
};
