#pragma once

#include "general/Logger.h"
#include "geometry/Building.h"

#include <algorithm>
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
     * Time the event is triggered
     */
    double _time;

    /**
     * Type of event
     */
    EventAction _action;

    /**
     * Geometry where the event should be applied to
     */
    Building * _building;

public:
    virtual ~Event() = default;

    /**
     * Processes the given \a _action at \a time on \a _building.
     */
    virtual void Process() = 0;

    /**
     * Sets the geometry for the event
     * @param building Geometry, on which the event should be applied
     */
    void SetBuilding(Building * building) { _building = building; }

    /**
      * @return the type of the event
      */
    [[nodiscard]] const EventAction & GetAction() const { return _action; };

    /**
      * @return the time at which the event was recorded
      */
    [[nodiscard]] double GetTime() const { return _time; };

    /**
      * @return a description of the event
      */
    [[nodiscard]] virtual std::string GetDescription() const = 0;
};
