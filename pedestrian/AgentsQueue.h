/**
 * \file        AgentsQueue.h
 * \date        Apr 14, 2015
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
 **/
#pragma once

#include <mutex>
#include <vector>

//forward declaration
class Pedestrian;

/**
 * Queue for incoming agents
 */
class AgentsQueueIn
{
public:
     static void Add(std::vector<Pedestrian*>& ped);
     static void GetandClear(std::vector<Pedestrian*>& peds);
     static bool IsEmpty();
     static int Size();
     //static std::vector<Pedestrian*> GetandClear();

private:
     AgentsQueueIn(){};
     virtual ~AgentsQueueIn(){};
     static std::vector<Pedestrian*> _agentsQueue;
     static std::mutex _queueMutex;
};

/**
 * Queue for outgoing agents
 */
class AgentsQueueOut
{
public:
     static void Add(std::vector<Pedestrian*>& ped);
     static void Add(Pedestrian* ped);
     static void GetandClear(std::vector<Pedestrian*>& peds);
     static bool IsEmpty();
     static int Size();

private:
     AgentsQueueOut(){};
     virtual ~AgentsQueueOut(){};
     static std::vector<Pedestrian*> _agentsQueue;
     static std::mutex _queueMutex;
};
