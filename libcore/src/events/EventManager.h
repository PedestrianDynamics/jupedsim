/**
 * \file        EventManager.h
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
 * The EventManager is responsible for handling events related to the door in the simulations.
 **/
#pragma once

#include "DoorEvent.h"

#include <string>
#include <vector>

class Building;

class EventManager
{
public:
    /**
      * Constructor.
      */
    explicit EventManager(Building * _b);

    /**
      * Default deconstructor.
      */
    ~EventManager() = default;

    // NOTE: disable copy constructor and assignment operator
    EventManager(const EventManager &) = delete;
    EventManager & operator=(const EventManager &) & = delete;

    /**
     * Adds the events from \p events to the EventManager. This should be done before the simulation
     * starts.
     * @param events List of events, which should be used in the simulation
     */
    void AddEvent(std::unique_ptr<Event> event);

    /**
      * Print the parsed events.
      */
    void ListEvents();

    /**
     * Process the event using the current time stamp from the pedestrian class.
     *
     * @return Any event was processed.
     */
    bool ProcessEvent();

private:
    /**
     * List of all events processed by the EventManager.
     */
    std::vector<std::unique_ptr<Event>> _events;

    /**
     * Geometry the events will be processed on.
     */
    Building * _building;
};
