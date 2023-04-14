/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Stage.hpp"

#include "GenericAgent.hpp"
#include "Point.hpp"
#include "Simulation.hpp"
#include "UniqueID.hpp"
#include <list>
#include <stdexcept>
#include <vector>

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

////////////////////////////////////////////////////////////////////////////////
/// Exit
////////////////////////////////////////////////////////////////////////////////
Exit::Exit(Polygon area_, std::vector<GenericAgent::ID>& toRemove_)
    : area(std::move(area_)), toRemove(toRemove_)
{
    if(!area.IsConvex()) {
        throw std::runtime_error("Exit areas need to be bounded by convex polygons.");
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

////////////////////////////////////////////////////////////////////////////////
/// NotifiableWaitingSet
////////////////////////////////////////////////////////////////////////////////
NotifiableWaitingSet::NotifiableWaitingSet(
    std::vector<Point> slots_,
    jps::UniqueID<Journey> journeyId_)
    : slots(std::move(slots_)), journeyId(journeyId_)
{
    occupants.reserve(slots.size());
}

bool NotifiableWaitingSet::IsCompleted(const GenericAgent& agent)
{
    if(state == WaitingState::Active) {
        return false;
    }
    const auto find_iter = std::find(std::begin(occupants), std::end(occupants), agent.id);
    if(find_iter != std::end(occupants)) {
        occupants.erase(find_iter);
        return true;
    }
    const auto distance = (agent.pos - slots[0]).Norm();
    return distance <= 1;
}

Point NotifiableWaitingSet::Target(const GenericAgent& agent)
{
    if(state == WaitingState::Inactive) {
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

void NotifiableWaitingSet::State(WaitingState s)
{
    state = s;
}

NotifiableWaitingSet::WaitingState NotifiableWaitingSet::State() const
{
    return state;
}
