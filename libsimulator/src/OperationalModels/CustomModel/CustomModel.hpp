// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CustomModelState.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"

/// Base class for operational models implemented outside libsimulator.
///
/// Derive from this class when a model is not part of the built-in model set. The derived model
/// still implements the pure virtual interface inherited from OperationalModel:
/// ComputeNextState() and CheckModelConstraint(). This class only fixes the model
/// type to OperationalModelType::CUSTOM_MODEL so custom models do not need to repeat that
/// boilerplate.
///
/// Per-agent custom state is stored as CustomModel::State (alias for CustomModelState) inside the
/// OperationalModelState variant. In ComputeNextState(), "next" arrives as an exact copy of
/// "current"; the model overwrites only the fields it changes. The agent's new position must be
/// written to State::position of "next".
///
/// @code
/// class MyModel : public CustomModel
/// {
/// public:
///     void ComputeNextState(
///         double dT,
///         const OperationalModelState& current,
///         OperationalModelState& next,
///         const Point& destination,
///         const EnvironmentQuery& envQuery) const override;
///
///     void CheckModelConstraint(
///         const OperationalModelState& state,
///         const EnvironmentQuery& envQuery) const override;
/// };
/// @endcode
///
/// @note CustomModel is still abstract. It cannot be instantiated directly.
/// @warning std::any payloads are type-erased. A mismatched accessor type throws std::bad_any_cast.
class CustomModel : public OperationalModel
{
public:
    using State = CustomModelState;

    CustomModel() = default;
    ~CustomModel() override = default;

    OperationalModelType Type() const override { return OperationalModelType::CUSTOM_MODEL; }
};
