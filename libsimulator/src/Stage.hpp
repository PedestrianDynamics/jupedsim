// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Destination.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "Geometry/Location.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "UniqueID.hpp"
#include "Util.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <unordered_set>
#include <variant>
#include <vector>

class BaseStage;
class Geometry;
class Simulation;

class BaseProxy
{
protected:
    Simulation* simulation;
    BaseStage* stage;

    BaseProxy(Simulation* simulation_, BaseStage* stage_) : simulation(simulation_), stage(stage_)
    {
    }
    virtual ~BaseProxy() = default;

public:
    size_t CountTargeting() const;
};

class WaypointProxy : public BaseProxy
{
public:
    WaypointProxy(Simulation* simulation_, BaseStage* stage_) : BaseProxy(simulation_, stage_) {}
};

class ExitProxy : public BaseProxy
{
public:
    ExitProxy(Simulation* simulation_, BaseStage* stage_) : BaseProxy(simulation_, stage_) {}
};

class DirectSteeringProxy : public BaseProxy
{
public:
    DirectSteeringProxy(Simulation* simulation_, BaseStage* stage_) : BaseProxy(simulation_, stage_)
    {
    }
};

using StageProxy = std::variant<WaypointProxy, ExitProxy, DirectSteeringProxy>;

class BaseStage
{
public:
    using ID = jps::UniqueID<BaseStage>;

protected:
    ID id;
    size_t targeting{0};

public:
    virtual ~BaseStage() = default;
    virtual bool IsCompleted(const GenericAgent& agent) = 0;
    virtual RoutingTarget Target(const GenericAgent& agent) = 0;
    virtual StageProxy Proxy(Simulation* simulation_) = 0;
    ID Id() const { return id; }
    size_t CountTargeting() const { return targeting; }
    void IncreaseTargeting() { targeting = targeting + 1; }
    void DecreaseTargeting()
    {
        assert(targeting >= 1);
        targeting = targeting - 1;
    }
};

template <>
struct fmt::formatter<BaseStage> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const BaseStage& s, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(id={}, targeting={})", s.Id(), s.CountTargeting());
    }
};

class Waypoint : public BaseStage
{
    Destination destination;

public:
    explicit Waypoint(Destination destination_);
    ~Waypoint() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    RoutingTarget Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
};

/// Notifies simulation of all agents that need to be removed at the beginning of the next iteration
class Exit : public BaseStage
{
    Destination destination;
    std::vector<GenericAgent::ID>& toRemove;

public:
    Exit(Destination destination_, std::vector<GenericAgent::ID>& toRemove_);
    ~Exit() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    RoutingTarget Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
};

class DirectSteering : public BaseStage
{
public:
    DirectSteering() = default;
    ~DirectSteering() override = default;
    bool IsCompleted(const GenericAgent&) override { return false; };
    RoutingTarget Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation) override
    {
        return DirectSteeringProxy(simulation, this);
    };
};
