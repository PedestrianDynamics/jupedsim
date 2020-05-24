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
 * The EventManager is responsible for handling events in the simulations.
 **/
#pragma once

#include "DoorEvent.h"

#include <string>
#include <vector>

class Building;

class EventManager
{
private:
    /**
     * List of all events processed by the EventManager.
     */
    std::vector<std::unique_ptr<Event>> _events;

    /**
     * Geometry the events will be processed on.
     */
    Building * _building;

    /**
     * Indicates if Events need to be sorted when calling ProcessEvent() 
     */
    bool _needs_sorting{false};

public:
    /**
      * Constructor.
      */
    EventManager() = default;

    /**
      * Default deconstructor.
      */
    ~EventManager() = default;

    // NOTE: disable copy constructor and assignment operator
    EventManager(const EventManager &) = delete;
    EventManager & operator=(const EventManager &) & = delete;

    /**
     * Adds \p event to the EventManager. This should be done before the simulation
     * starts.
     * @param event Event which should be handled by the event manager
     */
    void AddEvent(std::unique_ptr<Event> event);

    /**
      * Print the events handled by EventManager.
      */
    void ListEvents();

    /**
     * Process all events until the now timestamp.
     * Processing is done for all events where time <=now.
     * Processed events will be discared.
     * @param now timestamp until which all events will be processed.
     * @return Any event was processed.
     */
    bool ProcessEvents(double now);
};
