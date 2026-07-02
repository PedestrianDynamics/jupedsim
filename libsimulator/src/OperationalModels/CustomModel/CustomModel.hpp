// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModel.hpp"

/// Base class for operational models implemented outside libsimulator.
///
/// Derive from this class when a model is not part of the built-in model set. The derived model
/// still implements the pure virtual interface inherited from OperationalModel:
/// ComputeNewPosition(), ApplyUpdate(), and CheckModelConstraint(). This class only fixes the model
/// type to OperationalModelType::CUSTOM_MODEL so custom models do not need to repeat that
/// boilerplate.
///
/// Per-agent custom state should be stored in GenericAgent::model as CustomModelData. Custom model
/// updates should be returned from ComputeNewPosition() as CustomModelUpdate. Both types store
/// their payload in std::any, so model implementations must agree on the concrete stored types and
/// retrieve them with the exact typed accessors.
///
/// Payload types must be copy-constructible because GenericAgent values are copied during
/// simulation queries, e.g. by NeighborhoodSearch.
///
/// @code
/// class MyModel : public CustomModel
/// {
/// public:
///     OperationalModelUpdate ComputeNewPosition(
///         double dT,
///         const GenericAgent& agent,
///         const CollisionGeometry& geometry,
///         const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const override;
///
///     void ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const override;
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
    CustomModel() = default;
    ~CustomModel() override = default;

    OperationalModelType Type() const override { return OperationalModelType::CUSTOM_MODEL; }
};
