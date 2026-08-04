// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "Geometry/Geometry2D.hpp"
#include "Geometry/Location.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "UniqueID.hpp"
#include "Util.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <set>
#include <unordered_set>
#include <variant>
#include <vector>

class Simulation;

class BaseStage;

enum class WaitingSetState {
    Active,
    Inactive,
};

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

class NotifiableWaitingSetProxy : public BaseProxy
{
public:
    NotifiableWaitingSetProxy(Simulation* simulation_, BaseStage* stage_)
        : BaseProxy(simulation_, stage_)
    {
    }
    void State(WaitingSetState newState);
    WaitingSetState State() const;
    size_t CountWaiting() const;
    const std::vector<GenericAgent::ID>& Waiting() const;
};

class NotifiableQueueProxy : public BaseProxy
{
public:
    NotifiableQueueProxy(Simulation* simulation_, BaseStage* stage_)
        : BaseProxy(simulation_, stage_)
    {
    }

    size_t CountEnqueued() const;
    const std::vector<GenericAgent::ID>& Enqueued() const;
    void Pop(size_t count);
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

using StageProxy = std::variant<
    WaypointProxy,
    NotifiableWaitingSetProxy,
    NotifiableQueueProxy,
    ExitProxy,
    DirectSteeringProxy>;

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
    virtual Point Target(const GenericAgent& agent) = 0;
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
    Location position;
    double distance;

public:
    Waypoint(Location position_, double distance_);
    ~Waypoint() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    Point Position() const { return position.xy(); };
};

/// Notifies simulation of all agents that need to be removed at the beginning of the next iteration
class Exit : public BaseStage
{
    Polygon area;
    Location centroid;
    std::vector<GenericAgent::ID>& toRemove;

public:
    Exit(Polygon area, Location centroid_, std::vector<GenericAgent::ID>& toRemove_);
    ~Exit() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    Polygon Position() const { return area; };
};

class NotifiableWaitingSet : public BaseStage
{
    std::vector<Location> slots;
    std::vector<GenericAgent::ID> occupants{};
    WaitingSetState state{WaitingSetState::Active};

public:
    NotifiableWaitingSet(std::vector<Location> slots_);
    ~NotifiableWaitingSet() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    void State(WaitingSetState s);
    WaitingSetState State() const;
    void Update(const EnvironmentQuery& envQuery);
    const std::vector<GenericAgent::ID>& Occupants() const;
    const std::vector<Location>& Slots() const { return slots; };
};

class NotifiableQueue : public BaseStage
{

private:
    std::vector<Location> slots;
    std::vector<GenericAgent::ID> occupants{};
    std::set<GenericAgent::ID> exitingThisUpdate{};

public:
    NotifiableQueue(std::vector<Location> slots_);
    ~NotifiableQueue() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    void Update(const EnvironmentQuery& envQuery);
    void Pop(size_t count);
    const std::vector<GenericAgent::ID>& Occupants() const;
    const std::vector<Location>& Slots() const { return slots; };
};

class DirectSteering : public BaseStage
{
public:
    DirectSteering() = default;
    ~DirectSteering() override = default;
    bool IsCompleted(const GenericAgent&) override { return false; };
    Point Target(const GenericAgent& agent) override { return agent.finalTarget; };
    StageProxy Proxy(Simulation* simulation) override
    {
        return DirectSteeringProxy(simulation, this);
    };
};
