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
 *
 *
 **/
#pragma once

#include "general/Configuration.h"
#include "general/Macros.h"
#include "general/Filesystem.h"
#include "Event.h"

#include <vector>
#include <string>
#include <random>

class Building;
class Pedestrian;
class Router;
class RoutingEngine;
class OutputHandler;

extern OutputHandler* Log;

class EventManager
{
public:
     /**
      * Constructor
      */
     EventManager(Configuration* config, Building *_b, unsigned int seed);

     /**
      * destructor
      */
     ~EventManager();

     /**
      * Read and parse the events
      * @return false if an error occured
      */
     bool ReadEventsXml();

     /**
      * Print the parsed events
      */
     void ListEvents();

     /**
      * Read and parse events from a text file
      * @param time
      */
     void ReadEventsTxt(double time);

     /**
      * Reads the schedule file
      */
     bool ReadSchedule();

     //process the event using the current time stamp
     //from the pedestrian class
     void ProcessEvent();
     //Eventhandling
     void CloseDoor(int id);
     void TempCloseDoor(int id);
     void OpenDoor(int id);
     void ResetDoor(int id);
     //void ChangeRouting(int id, const std::string& state);
     void GetEvent(char* c);


private:
     /**
      * collect the close doors and generate a new graph
      * @param _building
      */
     bool CreateRoutingEngine(Building* _b, int first_engine=false);

     /**
      * Create a router corresponding to the given strategy
      * @param strategy
      * @return a router/NULL for invalid strategies
      */
     Router * CreateRouter(const RoutingStrategy& strategy);

     /**
      * Update the knowledge about closed doors.
      * Each pedestrian who is xx metres from a closed door,
      * will save that information
      * @param _b, the building object
      */
     bool DisseminateKnowledge(Building* _b);

     /**
      * Gather knowledge about the state of the doors.
      * Which is going to be disseminated afterwards.
      * @param _b
      * @return
      */
     bool CollectNewKnowledge(Building* _b);

     /**
      * Synchronize the knowledge of the two pedestrians.
      * The information with the newest timestamp
      * is always accepted with a probability of one.
      * @param p1, first pedestrian
      * @param p2, second pedestrian
      * @return true if the information could be synchronized
      */
     bool SynchronizeKnowledge(Pedestrian* p1, Pedestrian* p2);

     /**
      * Merge the knowledge of the two pedestrians. Ped1 is informing ped2 who depending
      * on his risk awareness probability could accept of refuse the new information.
      * @param p1, the informant with the new information
      * @param p2, the pedestrian receiving the information
      * @return true in the case p2 accepted the new information
      */
     bool MergeKnowledge(Pedestrian* p1, Pedestrian* p2);

     bool MergeKnowledgeUsingProbability(Pedestrian* p1, Pedestrian* p2);


     /**
      * Update the pedestrian route based on the new information
      * @param p1
      * @return
      */
     bool UpdateRoute(Pedestrian* p1);

     void CreateSomeEngines();

private:
     Configuration* _config;
     std::vector<Event> _events;
     Building *_building;
     FILE *_file;
     bool _dynamic;
     int _eventCounter;
     long int _lastUpdateTime;
     //information propagation time in seconds
     int _updateFrequency;
     //information propagation range in meters
     int _updateRadius;
     //save the router corresponding to the actual state of the building
     std::map<std::string, RoutingEngine*> _eventEngineStorage;
     //save the available routers defined in the simulation
     std::vector<RoutingStrategy> _availableRouters;

     // random number generator
     std::mt19937 _rdGenerator;
     std::uniform_real_distribution<double> _rdDistribution;
     std::map<int, std::vector<int>> groupDoor;
};
