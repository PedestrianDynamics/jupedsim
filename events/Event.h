/*
 * Event.h
 *
 *  Created on: Jul 21, 2015
 *      Author: piccolo
 */

#pragma once

#include <string>

// Describes the event
enum class EventAction { OPEN, CLOSE, TEMP_CLOSE, RESET_USAGE, NOTHING};

class Event
{
public:

     /**
      * Constructor
      * @param id
      * @param time
      * @param type
      * @param state
      */
     Event(int id, double time, const std::string& type,
               const std::string& state);

     /**
      * Destructor
      */
     virtual ~Event();

     /**
      * @return the id of the event
      */
     int GetId() const;

     /**
      * @return the type of the event
      */
     const EventAction& GetAction() const;

     /**
      * @return the time at which the event was recorded
      */
     double GetTime() const;

     /**
      * @return the type of the event (room, door)
      */
     const std::string& GetType() const;

     /**
      * return a description of the event
      */
     const std::string GetDescription() const;

private:
     double _time;
     int _id;
     std::string _type;
     EventAction _action;

     EventAction StringToEventAction(const std::string&);
};
