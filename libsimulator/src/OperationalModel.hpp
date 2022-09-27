/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Clonable.hpp"
#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"

#include <optional>

class Agent;

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

    virtual PedestrianUpdate ComputeNewPosition(
        double dT,
        const Agent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch& neighborhoodSearch) const = 0;

    virtual void ApplyUpdate(const PedestrianUpdate& update, Agent& agent) const = 0;
    virtual bool ParameterProfileExists(ParametersID id) const = 0;
};

template <typename ModelData>
class OperationalModelBase : public OperationalModel
{
private:
    std::unordered_map<OperationalModel::ParametersID, ModelData> _parameterProfiles;

public:
    bool ParameterProfileExists(ParametersID id) const override
    {
        return _parameterProfiles.count(id) > 0;
    };

protected:
    OperationalModelBase(const std::vector<ModelData>& parameterProfiles)
    {
        _parameterProfiles.reserve(parameterProfiles.size());
        for(auto&& p : parameterProfiles) {
            auto [_, success] = _parameterProfiles.try_emplace(p.id, p);
            if(!success) {
                throw std::runtime_error(
                    fmt::format("Duplicate agent profile id={} supplied", p.id));
            }
        }
    };

    const ModelData& parameterProfile(OperationalModel::ParametersID id) const
    {
        return _parameterProfiles.at(id);
    };
};
