// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include "GenericAgent.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "Simulation.hpp"
#include "SimulationError.hpp"
#include "Util.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <list>
#include <utility>
#include <vector>

namespace
{
/// 3D distance between @p agent and @p where.
double distance_to(const GenericAgent& agent, const Location& where)
{
    const auto in_plan = (agent.Position() - where.xy()).Norm();
    if(!agent.location) {
        return in_plan;
    }
    return std::hypot(in_plan, agent.location->z() - where.z());
}

/// Whether the agent has a straight way to a place.
bool can_walk_straight_to(const GenericAgent& agent, const Location& where)
{
    return !agent.location || agent.location->can_walk_straight_to(where);
}
} // namespace

////////////////////////////////////////////////////////////////////////////////
/// Base Proxy
////////////////////////////////////////////////////////////////////////////////
size_t BaseProxy::CountTargeting() const
{
    return stage->CountTargeting();
}

////////////////////////////////////////////////////////////////////////////////
/// NotifiableQueueProxy
////////////////////////////////////////////////////////////////////////////////

size_t NotifiableQueueProxy::CountEnqueued() const
{
    auto concreteStage = dynamic_cast<NotifiableQueue*>(stage);
    assert(stage);
    return concreteStage->Occupants().size();
}

const std::vector<GenericAgent::ID>& NotifiableQueueProxy::Enqueued() const
{
    const auto concreteStage = dynamic_cast<const NotifiableQueue*>(stage);
    assert(stage);
    return concreteStage->Occupants();
}

void NotifiableQueueProxy::Pop(size_t count)
{
    auto concreteStage = dynamic_cast<NotifiableQueue*>(stage);
    assert(stage);
    return concreteStage->Pop(count);
}

////////////////////////////////////////////////////////////////////////////////
/// NotifiableWaitingSetProxy
////////////////////////////////////////////////////////////////////////////////
void NotifiableWaitingSetProxy::State(WaitingSetState newState)
{
    auto concreteStage = dynamic_cast<NotifiableWaitingSet*>(stage);
    assert(stage);
    concreteStage->State(newState);
}

WaitingSetState NotifiableWaitingSetProxy::State() const
{
    const auto concreteStage = dynamic_cast<const NotifiableWaitingSet*>(stage);
    assert(stage);
    return concreteStage->State();
}

size_t NotifiableWaitingSetProxy::CountWaiting() const
{
    const auto concreteStage = dynamic_cast<const NotifiableWaitingSet*>(stage);
    assert(stage);
    return concreteStage->Occupants().size();
}

const std::vector<GenericAgent::ID>& NotifiableWaitingSetProxy::Waiting() const
{
    auto concreteStage = dynamic_cast<NotifiableWaitingSet*>(stage);
    assert(stage);
    return concreteStage->Occupants();
}

////////////////////////////////////////////////////////////////////////////////
/// Waypoint
////////////////////////////////////////////////////////////////////////////////
Waypoint::Waypoint(Location position_, double distance_) : position(position_), distance(distance_)
{
}

bool Waypoint::IsCompleted(const GenericAgent& agent)
{
    return distance_to(agent, position) <= distance;
}

Point Waypoint::Target(const GenericAgent&)
{
    return position.xy();
}

StageProxy Waypoint::Proxy(Simulation* simulation)
{
    return WaypointProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////////////////////////
Exit::Exit(Polygon area_, Location centroid_, std::vector<GenericAgent::ID>& toRemove_)
    : area(std::move(area_)), centroid(centroid_), toRemove(toRemove_)
{
    if(!area.IsConvex()) {
        throw SimulationError("Exit areas need to be bounded by convex polygons.");
    }
}

bool Exit::IsCompleted(const GenericAgent& agent)
{
    const bool hasReachedExit =
        area.IsInside(agent.Position()) && can_walk_straight_to(agent, centroid);
    if(hasReachedExit) {
        toRemove.push_back(agent.id);
    }
    return hasReachedExit;
}

Point Exit::Target(const GenericAgent&)
{
    return centroid.xy();
}

StageProxy Exit::Proxy(Simulation* simulation)
{
    return ExitProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// NotifiableWaitingSet
////////////////////////////////////////////////////////////////////////////////
NotifiableWaitingSet::NotifiableWaitingSet(std::vector<Location> slots_) : slots(std::move(slots_))
{
    occupants.reserve(slots.size());
}

bool NotifiableWaitingSet::IsCompleted(const GenericAgent& agent)
{
    if(state == WaitingSetState::Active) {
        return false;
    }
    const auto find_iter = std::find(std::begin(occupants), std::end(occupants), agent.id);
    if(find_iter != std::end(occupants)) {
        return true;
    }
    return distance_to(agent, slots[0]) <= 1;
}

Point NotifiableWaitingSet::Target(const GenericAgent& agent)
{
    if(state == WaitingSetState::Inactive) {
        return slots[0].xy();
    }

    const auto next_slot_index = std::min(occupants.size(), slots.size() - 1);

    for(size_t index = 0; index < next_slot_index; ++index) {
        if(agent.id == occupants[index]) {
            return slots[index].xy();
        }
    }

    return slots[next_slot_index].xy();
}

void NotifiableWaitingSet::State(WaitingSetState s)
{
    if(state == s) {
        return;
    }
    if(s == WaitingSetState::Active) {
        occupants.clear();
    }
    state = s;
}

WaitingSetState NotifiableWaitingSet::State() const
{
    return state;
}

StageProxy NotifiableWaitingSet::Proxy(Simulation* simulation)
{
    return NotifiableWaitingSetProxy(simulation, this);
}

const std::vector<GenericAgent::ID>& NotifiableWaitingSet::Occupants() const
{
    return occupants;
}

void NotifiableWaitingSet::Update(const EnvironmentQuery& envQuery)
{
    if(state == WaitingSetState::Inactive) {
        return;
    }
    const auto count_occupants = occupants.size();
    if(count_occupants == slots.size()) {
        return;
    }

    for(size_t index = count_occupants; index < slots.size(); ++index) {
        const auto& slot = slots[index];
        auto candidates = envQuery.AgentsInRange(slot.xy(), 2, [&](const GenericAgent& candidate) {
            return candidate.location && envQuery.NoGeometryBetween(slot, *candidate.location);
        });

        GenericAgent::ID occupant = GenericAgent::ID::Invalid;
        double min_distance = std::numeric_limits<double>::max();
        for(const auto& agent : candidates) {
            if(agent.stageId == id) {
                if(std::find(std::begin(occupants), std::end(occupants), agent.id) ==
                   std::end(occupants)) {
                    const auto distance = (agent.Position() - slot.xy()).Norm();
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

////////////////////////////////////////////////////////////////////////////////
/// NotifiablQueue
////////////////////////////////////////////////////////////////////////////////
NotifiableQueue::NotifiableQueue(std::vector<Location> slots_) : slots(std::move(slots_))
{
}

bool NotifiableQueue::IsCompleted(const GenericAgent& agent)
{
    const bool completed = exitingThisUpdate.contains(agent.id);
    if(completed) {
        exitingThisUpdate.erase(agent.id);
    }
    return completed;
}

Point NotifiableQueue::Target(const GenericAgent& agent)
{

    if(const auto index_opt = IndexInContainer(occupants, agent.id); index_opt) {
        return slots[*index_opt].xy();
    }

    const auto next_target_index = std::min(occupants.size(), slots.size() - 1);
    return slots[next_target_index].xy();
}

void NotifiableQueue::Pop(size_t count)
{
    for(size_t counter = 0; counter < count; ++counter) {
        if(occupants.empty()) {
            return;
        }
        exitingThisUpdate.insert(occupants.front());
        occupants.erase(std::begin(occupants));
    }
}

StageProxy NotifiableQueue::Proxy(Simulation* simulation)
{
    return NotifiableQueueProxy(simulation, this);
}

const std::vector<GenericAgent::ID>& NotifiableQueue::Occupants() const
{
    return occupants;
}

void NotifiableQueue::Update(const EnvironmentQuery& envQuery)
{
    const auto count_occupants = occupants.size();
    if(count_occupants == slots.size()) {
        return;
    }

    for(size_t index = count_occupants; index < slots.size(); ++index) {
        const auto& slot = slots[index];
        auto candidates = envQuery.AgentsInRange(slot.xy(), 2, [&](const GenericAgent& candidate) {
            return candidate.location && envQuery.NoGeometryBetween(slot, *candidate.location);
        });

        GenericAgent::ID occupant = GenericAgent::ID::Invalid;
        double min_distance = std::numeric_limits<double>::max();
        for(const auto& agent : candidates) {
            if(agent.stageId != id || Contains(occupants, agent.id) ||
               exitingThisUpdate.contains(agent.id)) {
                continue;
            }
            const auto distance = (agent.Position() - slot.xy()).Norm();
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
