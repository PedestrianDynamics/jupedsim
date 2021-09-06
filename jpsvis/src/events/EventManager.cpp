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

#include <Logger.h>

void EventManager::AddEvent(std::unique_ptr<Event> event)
{
    _events.emplace_back(std::move(event));
    _needs_sorting = true;
}

void EventManager::ListEvents()
{
    for(const auto & event : _events) {
        LOG_INFO("{}", *event);
    }
}

bool EventManager::ProcessEvents(double now)
{
    if(_needs_sorting) {
        std::sort(std::begin(_events), std::end(_events), [](auto & event1, auto & event2) {
            return event1->GetTime() > event2->GetTime();
        });
        _needs_sorting = false;
    }

    bool has_processed = false;
    while(!_events.empty() && _events.back()->GetTime() <= now) {
        _events.back()->Process();
        _events.pop_back();
        has_processed = true;
    }

    return has_processed;
}
