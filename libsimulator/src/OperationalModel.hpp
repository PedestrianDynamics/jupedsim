/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Clonable.hpp"
#include "CollisionGeometry.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "UniqueID.hpp"

#include <optional>
#include <unordered_map>

template <typename T>
class NeighborhoodSearch;

struct PedestrianUpdate {
    std::optional<Point> position{};
    std::optional<Point> velocity{};
    Point e0{}; // desired direction
    bool resetTurning{false};
};

class OperationalModel : public Clonable<OperationalModel>
{
    struct Parameters {
    };

public:
    using ParametersID = jps::UniqueID<OperationalModel::Parameters>;
    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    // virtual PedestrianUpdate ComputeNewPosition(
    //     double dT,
    //     const GenericAgent& ped,
    //     const CollisionGeometry& geometry,
    //     const NeighborhoodSearch<GenericAgent*>& neighborhoodSearch) const = 0;

    // virtual void ApplyUpdate(const PedestrianUpdate& update, GenericAgent& agent) const = 0;
    virtual bool ParameterProfileExists(ParametersID id) const = 0;
};

template <typename Params>
class OperationalModelBase : public OperationalModel
{
private:
    std::unordered_map<OperationalModel::ParametersID, Params> _parameterProfiles;

public:
    bool ParameterProfileExists(ParametersID id) const override
    {
        return _parameterProfiles.count(id) > 0;
    };

protected:
    OperationalModelBase(const std::vector<Params>& parameterProfiles)
    {
        _parameterProfiles.reserve(parameterProfiles.size());
        for(auto&& p : parameterProfiles) {
            auto [_, success] = _parameterProfiles.try_emplace(p.id, p);
            if(!success) {
                throw SimulationError("Duplicate agent profile id={} supplied", p.id);
            }
        }
    };

    const Params& parameterProfile(OperationalModel::ParametersID id) const
    {
        return _parameterProfiles.at(id);
    };
};
