/**
 * \file        AgentsQueue.cpp
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
#include "AgentsQueue.h"

#include "Pedestrian.h"

std::vector<Pedestrian*> AgentsQueueIn::_agentsQueue;
std::vector<Pedestrian*> AgentsQueueOut::_agentsQueue;

std::mutex AgentsQueueIn::_queueMutex;
std::mutex AgentsQueueOut::_queueMutex;


void AgentsQueueIn::Add(std::vector<Pedestrian*>& peds)
{
     _queueMutex.lock();
     _agentsQueue.insert(_agentsQueue.end(),peds.begin(),peds.end());
     _queueMutex.unlock();
}

void AgentsQueueIn::GetandClear(std::vector<Pedestrian*>& peds)
{
     _queueMutex.lock();

     if(_agentsQueue.size()!=0)
     {
          peds.insert(peds.end(),_agentsQueue.begin(), _agentsQueue.end());
          //_agentsQueue.pop_back();
          _agentsQueue.clear();
     }
     _queueMutex.unlock();
}

bool AgentsQueueIn::IsEmpty()
{
     return (_agentsQueue.size()==0);
}

int AgentsQueueIn::Size()
{
     return _agentsQueue.size();
}

/////////////////////////////////////////////////////
////////////////////////////////////////////////////

void AgentsQueueOut::Add(std::vector<Pedestrian*>& peds)
{
     _queueMutex.lock();
     _agentsQueue.insert(_agentsQueue.end(),peds.begin(),peds.end());

     //todo: Can save time using a map
     std::sort( _agentsQueue.begin(), _agentsQueue.end() );
     _agentsQueue.erase( std::unique( _agentsQueue.begin(), _agentsQueue.end() ), _agentsQueue.end() );

     _queueMutex.unlock();
}

void AgentsQueueOut::Add(Pedestrian* ped)
{
     _queueMutex.lock();
     _agentsQueue.push_back(ped);
     _queueMutex.unlock();
}

void AgentsQueueOut::GetandClear(std::vector<Pedestrian*>& peds)
{
     _queueMutex.lock();

     if(_agentsQueue.size()!=0)
     {
          peds.insert(peds.end(),_agentsQueue.begin(), _agentsQueue.end());
          //_agentsQueue.pop_back();
          _agentsQueue.clear();
     }
     _queueMutex.unlock();
}

bool AgentsQueueOut::IsEmpty()
{
     return (_agentsQueue.size()==0);
}

int AgentsQueueOut::Size()
{
     return _agentsQueue.size();
}
