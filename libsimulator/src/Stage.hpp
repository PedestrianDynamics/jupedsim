// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Logger.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "UniqueID.hpp"
#include "Util.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <unordered_set>
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

using StageProxy =
    std::variant<WaypointProxy, NotifiableWaitingSetProxy, NotifiableQueueProxy, ExitProxy>;

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
    Point position;
    double distance;

public:
    Waypoint(Point position_, double distance_);
    ~Waypoint() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    Point Position() const { return position; };
};

/// Notifies simulation of all agents that need to be removed at the beginning of the next iteration
class Exit : public BaseStage
{
    Polygon area;
    std::vector<GenericAgent::ID>& toRemove;

public:
    Exit(Polygon area, std::vector<GenericAgent::ID>& toRemove_);
    ~Exit() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    Polygon Position() const { return area; };
};

class NotifiableWaitingSet : public BaseStage
{
    std::vector<Point> slots;
    std::vector<GenericAgent::ID> occupants{};
    WaitingSetState state{WaitingSetState::Active};

public:
    NotifiableWaitingSet(std::vector<Point> slots_);
    ~NotifiableWaitingSet() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    void State(WaitingSetState s);
    WaitingSetState State() const;
    template <typename T>
    void Update(const NeighborhoodSearch<T>& neighborhoodSearch);
    const std::vector<GenericAgent::ID>& Occupants() const;
    const std::vector<Point>& Slots() const { return slots; }
};

template <typename T>
void NotifiableWaitingSet::Update(const NeighborhoodSearch<T>& neighborhoodSearch)
{
    if(state == WaitingSetState::Inactive) {
        return;
    }
    const auto count_occupants = occupants.size();
    if(count_occupants == slots.size()) {
        return;
    }

    for(size_t index = count_occupants; index < slots.size(); ++index) {
        const auto candidates = neighborhoodSearch.GetNeighboringAgents(slots[index], 2);
        GenericAgent::ID occupant = GenericAgent::ID::Invalid;
        double min_distance = std::numeric_limits<double>::max();
        for(const auto& agent : candidates) {
            if(agent.stageId == id) {
                if(std::find(std::begin(occupants), std::end(occupants), agent.id) ==
                   std::end(occupants)) {
                    const auto distance = (agent.pos - slots[index]).Norm();
                    if(distance < min_distance) {
                        min_distance = distance;
                        occupant = agent.id;
                    }
                }
            }
        }
        if(occupant != GenericAgent::ID::Invalid) {
            occupants.push_back(occupant);
        } else {
            return;
        }
    }
}

class NotifiableQueue : public BaseStage
{

private:
    std::vector<Point> slots;
    std::vector<GenericAgent::ID> occupants{};
    std::set<GenericAgent::ID> exitingThisUpdate{};

public:
    NotifiableQueue(std::vector<Point> slots_);
    ~NotifiableQueue() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    StageProxy Proxy(Simulation* simulation_) override;
    template <typename T>
    void Update(const NeighborhoodSearch<T>& neighborhoodSearch);
    void Pop(size_t count);
    const std::vector<GenericAgent::ID>& Occupants() const;
    const std::vector<Point>& Slots() const { return slots; }
};

template <typename T>
void NotifiableQueue::Update(const NeighborhoodSearch<T>& neighborhoodSearch)
{
    const auto count_occupants = occupants.size();
    if(count_occupants == slots.size()) {
        return;
    }

    for(size_t index = count_occupants; index < slots.size(); ++index) {
        const auto candidates = neighborhoodSearch.GetNeighboringAgents(slots[index], 2);
        GenericAgent::ID occupant = GenericAgent::ID::Invalid;
        double min_distance = std::numeric_limits<double>::max();
        for(const auto& agent : candidates) {
            if(agent.stageId != id || Contains(occupants, agent.id) ||
               exitingThisUpdate.contains(agent.id)) {
                continue;
            }
            const auto distance = (agent.pos - slots[index]).Norm();
            if(distance < min_distance) {
                min_distance = distance;
                occupant = agent.id;
            }
        }
        if(occupant != GenericAgent::ID::Invalid) {
            occupants.emplace_back(occupant);
        } else {
            return;
        }
    }
}
