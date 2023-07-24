/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Logger.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "UniqueID.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <queue>
#include <vector>

class Simulation;

class Stage
{
public:
    virtual ~Stage() = default;
    virtual bool IsCompleted(const GenericAgent& agent) = 0;
    virtual Point Target(const GenericAgent& agent) = 0;
};

class Waypoint : public Stage
{
    Point position;
    double distance;

public:
    Waypoint(Point position_, double distance_);
    ~Waypoint() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
};

/// Notifies simulation of all agents that need to be removed at the beginning of the next iteration
class Exit : public Stage
{
    Polygon area;
    std::vector<GenericAgent::ID>& toRemove;

public:
    Exit(Polygon area, std::vector<GenericAgent::ID>& toRemove_);
    ~Exit() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
};

class NotifiableWaitingSet : public Stage
{
public:
    enum class WaitingState {
        Active,
        Inactive,
    };

private:
    std::vector<Point> slots;
    jps::UniqueID<Journey> journeyId;
    std::vector<GenericAgent::ID> occupants{};
    WaitingState state{WaitingState::Active};

public:
    NotifiableWaitingSet(std::vector<Point> slots_, jps::UniqueID<Journey> journeyId);
    ~NotifiableWaitingSet() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    void State(WaitingState s);
    WaitingState State() const;
    template <typename T>
    void Update(const NeighborhoodSearch<T>& neighborhoodSearch);
};

template <typename T>
void NotifiableWaitingSet::Update(const NeighborhoodSearch<T>& neighborhoodSearch)
{
    if(state == WaitingState::Inactive) {
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
            if(agent.journeyId == journeyId) {
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

class NotifiableQueue : public Stage
{

private:
    std::vector<Point> slots;
    jps::UniqueID<Journey> journeyId;
    std::vector<GenericAgent::ID> occupants{};
    std::vector<GenericAgent::ID> exitingThisUpdate{};
    size_t popCountOnNextUpdate{};

public:
    NotifiableQueue(std::vector<Point> slots_, jps::UniqueID<Journey> journeyId_);
    ~NotifiableQueue() override = default;
    bool IsCompleted(const GenericAgent& agent) override;
    Point Target(const GenericAgent& agent) override;
    template <typename T>
    void Update(const NeighborhoodSearch<T>& neighborhoodSearch);
    void Pop(size_t count);
};

template <typename T>
void NotifiableQueue::Update(const NeighborhoodSearch<T>& neighborhoodSearch)
{
    exitingThisUpdate.clear();
    if(popCountOnNextUpdate > 0) {
        const auto beginCopy = std::begin(occupants);
        const auto endCopy = beginCopy + popCountOnNextUpdate;
        std::copy(beginCopy, endCopy, std::back_inserter(exitingThisUpdate));
        occupants.erase(beginCopy, endCopy);
        popCountOnNextUpdate = 0;
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
            if(agent.journeyId == journeyId) {
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
            LOG_DEBUG("occupant {} enqueued @ {}", occupant, occupants.size());
        } else {
            return;
        }
    }
}
