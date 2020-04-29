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

#include "DoorEvent.h"
#include "general/Logger.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

#include <cmath>

EventManager::EventManager(Building * _b)
{
    _building = _b;
}

void EventManager::AddEvent(std::unique_ptr<Event> event)
{
    event->SetBuilding(_building);
    _events.emplace_back(std::move(event));
    std::sort(std::begin(_events), std::end(_events), [](auto & event1, auto & event2) {
        return event1->GetTime() < event2->GetTime();
    });
}

void EventManager::ListEvents()
{
    for(const auto & event : _events) {
        LOG_INFO("{}", event->GetDescription());
    }
}

bool EventManager::ProcessEvent()
{
    double timeMin = Pedestrian::GetGlobalTime() - J_EPS_EVENT;
    std::unique_ptr<Event> eventMin =
        std::make_unique<DoorEvent>(-1, timeMin, EventAction::DOOR_OPEN);

    auto firstEventAtTime = std::upper_bound(
        std::begin(_events), std::end(_events), eventMin, [](auto & val, auto & event) {
            return val->GetTime() < event->GetTime();
        });

    double timeMax = Pedestrian::GetGlobalTime() + J_EPS_EVENT;
    std::unique_ptr<Event> eventMax =
        std::make_unique<DoorEvent>(-1, timeMax, EventAction::DOOR_OPEN);
    auto lastEventAtTime = std::lower_bound(
        std::begin(_events), std::end(_events), eventMax, [](auto & event, auto & val) {
            return event->GetTime() <= val->GetTime();
        });

    std::for_each(firstEventAtTime, lastEventAtTime, [](auto & event) { event->Process(); });

    return std::distance(firstEventAtTime, lastEventAtTime) != 0;
}
