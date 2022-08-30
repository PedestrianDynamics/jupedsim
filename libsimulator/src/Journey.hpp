#pragma once

#include "Clonable.hpp"
#include "Point.hpp"

#include <UniqueID.hpp>

#include <tuple>
#include <vector>

class Journey : public Clonable<Journey>
{
public:
    using ID = jps::UniqueID<Journey>;
    virtual ~Journey() = default;

private:
    ID id;

public:
    ID Id() const { return id; }

private:
};

class SimpleJourney final : public Journey
{
    using Waypoint = std::tuple<Point, double>;

    std::vector<Waypoint> waypoints{};

public:
    ~SimpleJourney() override = default;
    void AddWaypoint(Point p, double distance);

    const Waypoint& operator[](size_t index) const { return waypoints[index]; }

    size_t size() const { return waypoints.size(); }

    std::unique_ptr<Journey> Clone() const override;
};

class Behaviour
{
public:
    virtual ~Behaviour() = default;

    virtual Point DesiredDestination(Point currentPosition) = 0;
};

class FollowWaypointsBehaviour : public Behaviour
{
    const SimpleJourney* journey;
    size_t currentWaypointIndex{0};

public:
    FollowWaypointsBehaviour(const SimpleJourney* journey) : journey(journey) {}
    ~FollowWaypointsBehaviour() override = default;

    Point DesiredDestination(Point currentPosition) override
    {
        for(auto index = currentWaypointIndex; index < journey->size(); ++index) {
            const auto [currentWaypoint, maximumDistance] = (*journey)[index];
            const auto distance = (currentPosition - currentWaypoint).Norm();
            if(distance > maximumDistance) {
                currentWaypointIndex = index;
                return currentWaypoint;
            }
        }
        return currentPosition;
    }
};
