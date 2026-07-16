// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "FormatAny.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"

#include <any>
#include <type_traits>
#include <utility>

/// Base class for operational models implemented outside libsimulator.
///
/// Derive from this class when a model is not part of the built-in model set. The derived model
/// still implements the pure virtual interface inherited from OperationalModel:
/// ComputeNextState() and CheckModelConstraint(). This class only fixes the model
/// type to OperationalModelType::CUSTOM_MODEL so custom models do not need to repeat that
/// boilerplate.
///
/// Per-agent custom state should be stored in GenericAgent::model as CustomModel::State. In
/// ComputeNextState(), "next" arrives as an exact copy of "current"; the model overwrites only the
/// fields it changes. The agent's new position must be written to CustomModel::State::position of
/// "next". CustomModel::State stores its payload in std::any, so model implementations must agree
/// on the concrete stored type and retrieve it with the exact typed accessors.
///
/// Payload types must be copy-constructible because GenericAgent values are copied during
/// simulation queries, e.g. by NeighborhoodSearch.
///
/// @code
/// class MyModel : public CustomModel
/// {
/// public:
///     void ComputeNextState(
///         double dT,
///         const GenericAgent& current,
///         GenericAgent& next,
///         const Geometry2D& geometry,
///         const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const override;
///
///     void CheckModelConstraint(
///         const GenericAgent& agent,
///         const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
///         const Geometry2D& geometry) const override;
/// };
/// @endcode
///
/// @note CustomModel is still abstract. It cannot be instantiated directly.
/// @warning std::any payloads are type-erased. A mismatched accessor type throws std::bad_any_cast.
class CustomModel : public OperationalModel
{
public:
    /// Type-erased per-agent state for CustomModel implementations.
    ///
    /// CustomModel::State lets a custom operational model store model-specific state in
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
    class State
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
            requires(!std::is_same_v<std::decay_t<T>, State>)
        State(T&& value) : value(std::forward<T>(value)), format(makeFormatFn<T>())
        {
            using Stored = std::decay_t<T>;
            static_assert(
                std::is_copy_constructible_v<Stored>,
                "CustomModel::State payloads must be copy-constructible");
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

        friend struct fmt::formatter<State>;
    };

    CustomModel() = default;
    ~CustomModel() override = default;

    OperationalModelType Type() const override { return OperationalModelType::CUSTOM_MODEL; }
};

template <>
struct fmt::formatter<CustomModel::State> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const CustomModel::State& value, fmt::format_context& ctx) const
    {
        return value.format(value.value, ctx);
    }
};
