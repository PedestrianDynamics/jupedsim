/*
 * Event.cpp
 *
 *  Created on: Jul 21, 2015
 *      Author: piccolo
 */

#include "Event.h"

Event::Event(int id, double time, const std::string& type,
          const std::string& state)
{
     _id=id;
     _time=time;
     _type=type;
     _state=state;
}

Event::~Event()
{
}

int Event::GetId() const
{
     return _id;
}

const std::string& Event::GetState() const
{
     return _state;
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
     sprintf(tmp,"After %.2f sec, %s door %d", _time,_state.c_str(), _id);
     return std::string(tmp);
}
