// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include "GenericAgent.hpp"
#include "Journey.hpp"
#include "Point.hpp"
#include "Simulation.hpp"
#include "UniqueID.hpp"
#include "Util.hpp"

#include <cstddef>
#include <list>
#include <stdexcept>
#include <vector>

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
Waypoint::Waypoint(Point position_, double distance_) : position(position_), distance(distance_)
{
}

bool Waypoint::IsCompleted(const GenericAgent& agent)
{
    const auto actual_distance = (agent.pos - position).Norm();
    return actual_distance <= distance;
}

Point Waypoint::Target(const GenericAgent&)
{
    return position;
}

StageProxy Waypoint::Proxy(Simulation* simulation)
{
    return WaypointProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////////////////////////
Exit::Exit(Polygon area_, std::vector<GenericAgent::ID>& toRemove_)
    : area(std::move(area_)), toRemove(toRemove_)
{
    if(!area.IsConvex()) {
        throw SimulationError("Exit areas need to be bounded by convex polygons.");
    }
}

bool Exit::IsCompleted(const GenericAgent& agent)
{
    const bool hasReachedExit = area.IsInside(agent.pos);
    if(hasReachedExit) {
        toRemove.push_back(agent.id);
    }
    return hasReachedExit;
}

Point Exit::Target(const GenericAgent&)
{
    return area.Centroid();
}

StageProxy Exit::Proxy(Simulation* simulation)
{
    return ExitProxy(simulation, this);
}

////////////////////////////////////////////////////////////////////////////////
/// NotifiableWaitingSet
////////////////////////////////////////////////////////////////////////////////
NotifiableWaitingSet::NotifiableWaitingSet(std::vector<Point> slots_) : slots(std::move(slots_))
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
    const auto distance = (agent.pos - slots[0]).Norm();
    return distance <= 1;
}

Point NotifiableWaitingSet::Target(const GenericAgent& agent)
{
    if(state == WaitingSetState::Inactive) {
        return slots[0];
    }

    const auto next_slot_index = occupants.size() == 0 ? 0 : occupants.size() - 1;

    for(size_t index = 0; index < next_slot_index; ++index) {
        if(agent.id == occupants[index]) {
            return slots[index];
        }
    }

    return slots[next_slot_index];
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

////////////////////////////////////////////////////////////////////////////////
/// NotifiablQueue
////////////////////////////////////////////////////////////////////////////////
NotifiableQueue::NotifiableQueue(std::vector<Point> slots_) : slots(std::move(slots_))
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
        return slots[*index_opt];
    }

    const auto next_target_index = std::min(occupants.size(), slots.size() - 1);
    return slots[next_target_index];
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
