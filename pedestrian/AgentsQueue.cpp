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
mutex AgentsQueue::_queueMutex;

void AgentsQueue::Add(vector<Pedestrian*>& peds)
{
     _queueMutex.lock();
     _agentsQueue.insert(_agentsQueue.end(),peds.begin(),peds.end());
     _queueMutex.unlock();

     //     while (true)
     //     {
     //          // try to lock mutex to modify 'job_shared'
     //          if (_queueMutex.try_lock())
     //          {
     //               _agentsQueue.insert(_agentsQueue.end(),peds.begin(),peds.end());
     //               _queueMutex.unlock();
     //               return;
     //          }
     //          else
     //          {
     //               //maybe the mutex is beeing used
     //               std::cout << "unable to lock the mutex for adding 1" <<endl;
     //               //std::this_thread::sleep_for(100);
     //          }
     //     }
}

void AgentsQueue::GetandClear(std::vector<Pedestrian*>& peds)
{
     //while (true)
     //{
     // try to lock mutex to modify _agentsQueue
     _queueMutex.lock();

     if(_agentsQueue.size()!=0)
     {
          peds.insert(peds.end(),_agentsQueue.begin(), _agentsQueue.end());
          //_agentsQueue.pop_back();
          _agentsQueue.clear();
     }
     _queueMutex.unlock();
     //          }
     //          else
     //          {
     //               //maybe the mutex is beeing used
     //               std::cout << "unable to lock the mutex for removing 2" <<endl;
     //               //std::this_thread::sleep_for(100);
     //          }
     //}
}

bool AgentsQueue::IsEmpty()
{
     return (_agentsQueue.size()==0);
}

