#pragma once

#include "geometry/Building.h"

#include <Logger.h>
#include <algorithm>
#include <iostream>
#include <string>

// Describes the event
enum class EventAction {
    DOOR_OPEN,
    DOOR_CLOSE,
    DOOR_TEMP_CLOSE,
    DOOR_RESET_USAGE,
    TRAIN_ARRIVAL,
    TRAIN_DEPARTURE
};

class Event
{
protected:
    /**
     * Creates an Event.
     * @param time when the event shall take place
     * @param action type of the event
     */
    Event(double time, EventAction action);

    /**
     * Time the event is triggered
     */
    double _time;

    /**
     * Type of event
     */
    EventAction _action;

public:
    virtual ~Event() = default;

    /**
     * Processes the given \a _action at \a time on \a _building.
     */
    virtual void Process() = 0;

    /**
      * @return the time at which the event was recorded
      */
    [[nodiscard]] double GetTime() const { return _time; };

    /**
      * @return human readable string representation
      */
    [[nodiscard]] virtual std::string ToString() const = 0;
};

std::ostream & operator<<(std::ostream & out, const Event & event);