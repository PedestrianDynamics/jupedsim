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
    Simulation* _simulation;
    BaseStage* _stage;

    BaseProxy(Simulation* simulation, BaseStage* stage) : _simulation(simulation), _stage(stage) {}
    virtual ~BaseProxy() = default;

public:
    size_t count_targeting() const;
};

class WaypointProxy : public BaseProxy
{
public:
    WaypointProxy(Simulation* simulation, BaseStage* stage) : BaseProxy(simulation, stage) {}
};

class ExitProxy : public BaseProxy
{
public:
    ExitProxy(Simulation* simulation, BaseStage* stage) : BaseProxy(simulation, stage) {}
};

class DirectSteeringProxy : public BaseProxy
{
public:
    DirectSteeringProxy(Simulation* simulation, BaseStage* stage) : BaseProxy(simulation, stage) {}
};

using StageProxy = std::variant<WaypointProxy, ExitProxy, DirectSteeringProxy>;

class BaseStage
{
public:
    using ID = jps::UniqueID<BaseStage>;

protected:
    ID _id;
    size_t _targeting{0};

public:
    virtual ~BaseStage() = default;
    virtual bool is_completed(const GenericAgent& agent) = 0;
    virtual RoutingTarget target(const GenericAgent& agent) = 0;
    virtual StageProxy proxy(Simulation* simulation) = 0;
    ID id() const { return _id; }
    size_t count_targeting() const { return _targeting; }
    void increase_targeting() { _targeting = _targeting + 1; }
    void decrease_targeting()
    {
        assert(_targeting >= 1);
        _targeting = _targeting - 1;
    }
};

template <>
struct fmt::formatter<BaseStage> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const BaseStage& s, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(id={}, targeting={})", s.id(), s.count_targeting());
    }
};

class Waypoint : public BaseStage
{
    Destination _destination;

public:
    explicit Waypoint(Destination destination);
    ~Waypoint() override = default;
    bool is_completed(const GenericAgent& agent) override;
    RoutingTarget target(const GenericAgent& agent) override;
    StageProxy proxy(Simulation* simulation) override;
};

/// Notifies simulation of all agents that need to be removed at the beginning of the next iteration
class Exit : public BaseStage
{
    Destination _destination;
    std::vector<GenericAgent::ID>& _to_remove;

public:
    Exit(Destination destination, std::vector<GenericAgent::ID>& to_remove);
    ~Exit() override = default;
    bool is_completed(const GenericAgent& agent) override;
    RoutingTarget target(const GenericAgent& agent) override;
    StageProxy proxy(Simulation* simulation) override;
};

class DirectSteering : public BaseStage
{
public:
    DirectSteering() = default;
    ~DirectSteering() override = default;
    bool is_completed(const GenericAgent&) override { return false; };
    RoutingTarget target(const GenericAgent& agent) override;
    StageProxy proxy(Simulation* simulation) override
    {
        return DirectSteeringProxy(simulation, this);
    };
};
