// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "FormatAny.hpp"
#include "GenericAgentState.hpp"
#include "OperationalModel.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

#include <any>
#include <type_traits>
#include <utility>

/// Base class for operational models implemented outside libsimulator.
///
/// Derive from this class when a model is not part of the built-in model set. The derived model
/// still implements the pure virtual interface inherited from OperationalModel:
/// GetNeighbors(), ComputeNextState() and CheckModelConstraint(). This class only fixes the model
/// type to OperationalModelType::CUSTOM_MODEL so custom models do not need to repeat that
/// boilerplate.
///
/// Per-agent custom state should be stored in GenericAgent::state as CustomModel::State. In
/// ComputeNextState(), "next" arrives as an exact copy of "current"; the model overwrites only the
/// fields it changes. The agent's new position must be written to CustomModel::State::position of
/// "next". CustomModel::State stores its payload in std::any, so model implementations must agree
/// on the concrete stored type and retrieve it with the exact typed accessors.
///
/// Payload types must be copy-constructible because per-agent state values are copied during
/// simulation queries, e.g. by NeighborhoodSearch.
///
/// @code
/// class MyModel : public CustomModel
/// {
/// public:
///     void GetNeighbors(
///         const GenericState& current,
///         const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
///         const CollisionGeometry& geometry,
///         StateContainer& neighbor_states) const override;
///
///     void ComputeNextState(
///         double dT,
///         const GenericState& current,
///         GenericState& next,
///         const AgentJourney& journey,
///         const CollisionGeometry& geometry,
///         const StateContainer& neighborStates) const override;
///
///     void CheckModelConstraint(
///         const GenericAgent& agent,
///         const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
///         const CollisionGeometry& geometry) const override;
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

