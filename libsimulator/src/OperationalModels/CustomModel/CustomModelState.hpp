// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "FormatAny.hpp"
#include "Point.hpp"

#include <any>
#include <type_traits>
#include <utility>

/// Type-erased per-agent state for CustomModel implementations.
///
/// CustomModelState lets a custom operational model store model-specific state in
/// GenericAgent::model without adding a new built-in model data type. The payload is stored
/// by value in std::any so custom state follows the same ownership model as built-in agent
/// model state.
///
/// Payload types must be copy-constructible. GenericAgent values are copied by parts of the
/// simulation, for example by neighborhood queries, so custom state must remain valid under
/// normal value-copy semantics.
///
/// Access is runtime-typed: Get<T>() must use the exact stored type T. A type mismatch
/// throws std::bad_any_cast.
///
/// Formatting is best-effort. If the payload has a fmt formatter, that is used. Otherwise,
/// if it provides a public ToString() callable on const T& and returning std::string,
/// std::string_view, or const char*, ToString() is used. If neither is available, formatting
/// falls back to a diagnostic <type@address> representation.
class CustomModelState
{
public:
    /// State position cache, kept outside the type-erased payload so the framework can read
    /// it without touching the payload (for GilSafePyObject payloads: without acquiring the
    /// GIL). The owning model must keep it in sync with the payload's own position state.
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
