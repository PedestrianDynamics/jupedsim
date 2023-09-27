// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Clonable.hpp"
#include "CollisionGeometry.hpp"
#include "GeneralizedCentrifugalForceModelData.hpp"
#include "OperationalModelType.hpp"
#include "OperationalModelUpdate.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "UniqueID.hpp"

#include <optional>
#include <unordered_map>

template <typename T>
class NeighborhoodSearch;

struct GenericAgent;

struct PedestrianUpdate {
    std::optional<Point> position{};
    std::optional<Point> velocity{};
    Point e0{}; // desired direction
    bool resetTurning{false};
};

template <>
struct fmt::formatter<PedestrianUpdate> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const PedestrianUpdate& u, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(),
            "Upd[position={}, velocity={}, e0={}, resetTurning={}]",
            u.position.value_or(Point{}),
            u.velocity.value_or(Point{}),
            u.e0,
            u.resetTurning);
    }
};

class OperationalModel : public Clonable<OperationalModel>
{
public:
    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    virtual OperationalModelType Type() const = 0;
    virtual OperationalModelUpdate ComputeNewPosition(
        double dT,
        const GenericAgent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const = 0;

    virtual void ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const = 0;
    virtual void CheckDistanceConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const = 0;
};
