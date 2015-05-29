/*
 * AgentsQueue.cpp
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#include "AgentsQueue.h"

#include "Pedestrian.h"

using namespace std;

vector<Pedestrian*> AgentsQueueIn::_agentsQueue;
vector<Pedestrian*> AgentsQueueOut::_agentsQueue;

mutex AgentsQueueIn::_queueMutex;
mutex AgentsQueueOut::_queueMutex;


void AgentsQueueIn::Add(vector<Pedestrian*>& peds)
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

void AgentsQueueOut::Add(vector<Pedestrian*>& peds)
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
