#pragma once
#include "Event.h"

#include <string>
#include <vector>


class TrainEvent : public Event
{
private:
    int _trainID;
    int _platformID;
    TrainType _train;
    int _roomID;
    int _subroomID;
    Point _trackStart;
    Point _trackEnd;
    Point _trainStart;
    Point _trainEnd;

public:
    TrainEvent(
        double time,
        EventAction action,
        int trainID,
        int platformID,
        TrainType train,
        int roomID,
        int subroomID,
        Point trackStart,
        Point trackEnd,
        Point trainStart,
        Point trainEnd) :
        _trainID(trainID),
        _platformID(platformID),
        _train(train),
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
    void TrainArrival();

    void TrainDeparture();
};
