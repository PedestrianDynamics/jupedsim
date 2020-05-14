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
     * ID of the track the train is acting on
     */
    int _trackID;

    /**
     * Type of train
     */
    TrainType _trainType;

    double _trainStartOffset;

    bool _fromEnd;

public:
    /**
     * Constructs a TrainEvent
     * @param time time at which the event is triggered
     * @param action action of event
     * @param trainID ID of train
     * @param trackID ID of track, where the train is located
     * @param trainType Type of train
     * @param trackStart starting point of track
     * @param trackEnd end point of track
     * @param trainStart starting point of train
     * @param trainEnd end point of train
     */
    TrainEvent(
        double time,
        EventAction action,
        int trainID,
        int trackID,
        TrainType trainType,
        double trainStartOffset,
        bool fromEnd) :
        _trainID(trainID),
        _trackID(trackID),
        _trainType(trainType),
        _trainStartOffset(trainStartOffset),
        _fromEnd(fromEnd)
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
