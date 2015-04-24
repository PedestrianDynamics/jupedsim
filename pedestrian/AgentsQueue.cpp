/*
 * AgentsQueue.cpp
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#include "AgentsQueue.h"
#include "Pedestrian.h"

using namespace std;

vector<Pedestrian*> AgentsQueue::_agentsQueue;
vector<Pedestrian*> AgentsQueueOut::_agentsQueue;

mutex AgentsQueue::_queueMutex;
mutex AgentsQueueOut::_queueMutex;


void AgentsQueue::Add(vector<Pedestrian*>& peds)
{
     _queueMutex.lock();
     _agentsQueue.insert(_agentsQueue.end(),peds.begin(),peds.end());
     _queueMutex.unlock();
}

void AgentsQueue::GetandClear(std::vector<Pedestrian*>& peds)
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

bool AgentsQueue::IsEmpty()
{
     return (_agentsQueue.size()==0);
}


/////////////////////////////////////////////////////
////////////////////////////////////////////////////

void AgentsQueueOut::Add(vector<Pedestrian*>& peds)
{
     _queueMutex.lock();
     _agentsQueue.insert(_agentsQueue.end(),peds.begin(),peds.end());

     //todo: avoid this by using a map
     std::sort( _agentsQueue.begin(), _agentsQueue.end() );
     _agentsQueue.erase( unique( _agentsQueue.begin(), _agentsQueue.end() ), _agentsQueue.end() );
     //std::cout<<"queue size:"<<_agentsQueue.size()<<endl;
     ///
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
