#pragma once
#include "Event.h"
#include "geometry/Building.h"
#include "geometry/Point.h"

#include <string>

struct TrainEventInfo {
    /**
     * Building to operate on
     */
    Building * building;

    /**
     * ID of the train handled by this event
     */
    int trainID;

    /**
     * ID of platform the train is acting on
     */
    int platformID;

    /**
     * Type of train
     */
    TrainType trainType;

    /**
     * ID of room where the train is located
     */
    int roomID;

    /**
     * ID of the subroom where the train is located
     */
    int subroomID;

    /**
     * Starting point of the track
     */
    Point trackStart;

    /**
     * End point of the track
     */
    Point trackEnd;

    /**
     * Starting point of the train
     */
    Point trainStart;

    /**
     * End point of the train
     */
    Point trainEnd;
};

/**
 * Let a train arrive at the given platform (\a _platformID) in \a _roomID, \a _subroomID.
 * The following happens:
 * - find projected points of train doors on walls
 * - split the walls at the projected point
 * - add new doors, new walls to subroom
 * - add new doors, new walls, old walls to building fields (for restoring)
 * - update subroom
 */
class TrainArrivalEvent : public Event
{
private:
    TrainEventInfo _info;

public:
    TrainArrivalEvent(double time, TrainEventInfo info);
    void Process() override;
    [[nodiscard]] std::string ToString() const override;
};

/**
 * Let a arrived train depart from the given platform (\a _platformID) in \a _roomID, \a _subroomID.
 * The following happens:
 * - remove walls added by arriving train
 * - remove doors added by arriving train
 * - restore old walls
 * - update subroom
 */
class TrainDepartureEvent : public Event
{
private:
    TrainEventInfo _info;

public:
    TrainDepartureEvent(double time, TrainEventInfo info);
    void Process() override;
    [[nodiscard]] std::string ToString() const override;
};
