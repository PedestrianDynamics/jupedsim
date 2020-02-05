/**
 * \file        EventManager.cpp
 * \date        Jul 4, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/
#include "EventManager.h"

#include "Event.h"
#include "general/Logger.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <cmath>

EventManager::EventManager(Building * _b)
{
    _building = _b;
}

void EventManager::AddEvents(std::vector<Event> events)
{
    _events.insert(_events.end(), events.begin(), events.end());
}

void EventManager::ListEvents()
{
    for(const auto & event : _events) {
        LOG_INFO("{}", event.GetDescription());
    }
}

bool EventManager::ProcessEvent()
{
    bool eventProcessed = false;

    for(const auto & event : _events) {
        if(fabs(event.GetTime() - Pedestrian::GetGlobalTime()) < J_EPS_EVENT) {
            //Event with current time stamp detected
            LOG_INFO("Event: after {:.2f} sec", Pedestrian::GetGlobalTime());
            switch(event.GetAction()) {
                case EventAction::OPEN:
                    OpenDoor(event.GetId());
                    break;
                case EventAction::CLOSE:
                    CloseDoor(event.GetId());
                    break;
                case EventAction::TEMP_CLOSE:
                    TempCloseDoor(event.GetId());
                    break;
                case EventAction::RESET_USAGE:
                    ResetDoor(event.GetId());
                    break;
                case EventAction::NOTHING:
                    LOG_WARNING("Unknown event action in events. open, close, reset or "
                                "temp_close. Default: do nothing");
                    break;
            }
            eventProcessed = true;
        }
    }
    return eventProcessed;
}

void EventManager::CloseDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->Close(true);
    LOG_INFO("Closing door {}", id);
}

void EventManager::TempCloseDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->TempClose(true);
    LOG_INFO("Closing door {}", id);
}

void EventManager::OpenDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->Open(true);
    LOG_INFO("Opening door {}", id);
}

void EventManager::ResetDoor(int id)
{
    Transition * t = _building->GetTransition(id);
    t->ResetDoorUsage();
    LOG_INFO("Resetting door usage {}", id);
}
