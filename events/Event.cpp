/*
 * Event.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: piccolo
 */

#include "Event.h"

#include <algorithm>

Event::Event(int id, double time, const std::string& type,
          const std::string& state)
{
     _id=id;
     _time=time;
     _type=type;
     _action= StringToEventAction(state);
}

Event::~Event()
{
}

int Event::GetId() const
{
     return _id;
}

const EventAction& Event::GetAction() const
{
     return _action;
}

double Event::GetTime() const
{
     return _time;
}

const std::string& Event::GetType() const
{
     return _type;
}

const std::string Event::GetDescription() const
{
     char tmp[1024];
     std::string state;
     switch (_action){
     case EventAction::OPEN:
          state = "open";
          break;
     case EventAction::CLOSE:
          state = "close";
          break;
     case EventAction::TEMP_CLOSE:
          state = "temp_close";
          break;
     case EventAction::RESET_USAGE:
          state = "reset_usage";
          break;
     case EventAction::NOTHING:
          state = "nothing";
          break;
     }
     sprintf(tmp,"After %.2f sec, %s door %d", _time, state.c_str(), _id);
     return std::string(tmp);
}

EventAction Event::StringToEventAction(const std::string& in)
{
     std::string name(in);
     std::transform(name.begin(), name.end(), name.begin(), ::tolower);

     if (name == "open"){
          return EventAction::OPEN;
     }

     if (name == "temp_close"){
          return EventAction::TEMP_CLOSE;
     }

     if (name == "close"){
          return EventAction::CLOSE;
     }

     if (name == "close"){
          return EventAction::CLOSE;
     }

     if (name == "reset"){
          return EventAction::RESET_USAGE;
     }

     return EventAction::NOTHING;
}
