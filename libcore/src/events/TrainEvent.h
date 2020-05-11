#pragma once
#include "Event.h"

#include <string>
#include <vector>


class TrainEvent : public Event
{
private:
    /**
     * ID of the train handled by this event
     */
    int _trainID;

    /**
     * ID of platform the train is acting on
     */
    int _platformID;

    /**
     * Type of train
     */
    TrainType _trainType;

    /**
     * ID of room where the train is located
     */
    int _roomID;

    /**
     * ID of the subroom where the train is located
     */
    int _subroomID;

    /**
     * Starting point of the track
     */
    Point _trackStart;

    /**
     * End point of the track
     */
    Point _trackEnd;

    /**
     * Starting point of the train
     */
    Point _trainStart;

    /**
     * End point of the train
     */
    Point _trainEnd;

public:
    /**
     * Constructs a TrainEvent
     * @param time time at which the event is triggered
     * @param action action of event
     * @param trainID ID of train
     * @param platformID ID of platform, where the train is located
     * @param trainType Type of train
     * @param roomID ID of room, where the train is located
     * @param subroom ID of subroom, where the train is located
     * @param trackStart starting point of track
     * @param trackEnd end point of track
     * @param trainStart starting point of train
     * @param trainEnd end point of train
     */
    TrainEvent(
        double time,
        EventAction action,
        int trainID,
        int platformID,
        TrainType trainType,
        int roomID,
        int subroomID,
        Point trackStart,
        Point trackEnd,
        Point trainStart,
        Point trainEnd) :
        _trainID(trainID),
        _platformID(platformID),
        _trainType(trainType),
        _roomID(roomID),
        _subroomID(subroomID),
        _trackStart(trackStart),
        _trackEnd(trackEnd),
        _trainStart(trainStart),
        _trainEnd(trainEnd)
    {
        _time   = time;
        _action = action;
    };

    virtual ~TrainEvent() = default;

    virtual void Process() override;

    [[nodiscard]] std::string GetDescription() const override;

private:
    /**
     * Let a train arrive at the given platform (\a _platformID) in \a _roomID, \a _subroomID.
     * The following happens:
     * - find projected points of train doors on walls
     * - split the walls at the projected point
     * - add new doors, new walls to subroom
     * - add new doors, new walls, old walls to building fields (for restoring)
     * - update subroom
     */
    void TrainArrival();

    /**
     * Let a arrived train depart from the given platform (\a _platformID) in \a _roomID, \a _subroomID.
     * The following happens:
     * - remove walls added by arriving train
     * - remove doors added by arriving train
     * - restore old walls
     * - update subroom
     */
    void TrainDeparture();
};
