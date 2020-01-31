/*
 * Event.h
 *
 *  Created on: Jul 21, 2015
 *      Author: piccolo
 */

#pragma once

#include <string>

// Describes the event
enum class EventAction { OPEN, CLOSE, TEMP_CLOSE, RESET_USAGE, NOTHING };

class Event
{
public:
    /**
      * Constructor
      * @param id
      * @param time
      * @param state
      */
    Event(int id, double time, const std::string & state);

    /**
      * Destructor
      */
    ~Event() = default;

    /**
      * @return the id of the event
      */
    [[nodiscard]] int GetId() const;

    /**
      * @return the type of the event
      */
    [[nodiscard]] const EventAction & GetAction() const;

    /**
      * @return the time at which the event was recorded
      */
    [[nodiscard]] double GetTime() const;

    /**
      * @return a description of the event
      */
    [[nodiscard]] std::string GetDescription() const;

private:
    /**
     * Time when the event should be executed.
     */
    double _time;

    /**
     * ID of transition which is effected by event.
     */
    int _id;

    /**
     * Action which should be performed.
     */
    EventAction _action;

    /**
     * Helper function to get the Action from the input.
     * @param input Input string
     * @return Action which is identified by \p input
     */
    static EventAction StringToEventAction(const std::string & input);
};
