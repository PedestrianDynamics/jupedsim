#pragma once
#include "Event.h"

#include <string>
#include <vector>


class TrainEvent : public Event
{
private:
    int _trackID;
    TrainType _train;

public:
    TrainEvent(int trackID, double time, EventAction action, TrainType train);

    virtual ~TrainEvent() = default;

    virtual void Process() override {};

    [[nodiscard]] std::string GetDescription() const override;

private:
    void TrainArrival();

    void TrainDeparture();
};
