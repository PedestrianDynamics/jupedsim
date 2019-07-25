/*
 * Event.h
 *
 *  Created on: Jul 21, 2015
 *      Author: piccolo
 */

#ifndef EVENTS_EVENT_H_
#define EVENTS_EVENT_H_

#include <string>
#include "../general/Macros.h"

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
      * @return the state (open, close) of the event
      */
     const DoorState& GetState() const;

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
     DoorState _state;
};

#endif /* EVENTS_EVENT_H_ */
