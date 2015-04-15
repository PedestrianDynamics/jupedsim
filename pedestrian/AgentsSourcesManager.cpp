/*
 * AgentsSourcesManager.cpp
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#include "AgentsSourcesManager.h"
#include "Pedestrian.h"
#include "AgentsQueue.h"
#include "AgentsSource.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

AgentsSourcesManager::AgentsSourcesManager()
{

}

AgentsSourcesManager::~AgentsSourcesManager()
{
}

void AgentsSourcesManager::operator()(int value)
{
     //the loop is updated each second.
     //it might be better to use a timer
     bool finished=false;

     do
     {
          finished=true;
          for (const auto& src: _sources)
          {
               if(src->GetPoolSize())
               {
                    vector<Pedestrian*> peds;
                    src->GenerateByFrequency(peds);
                    AgentsQueue::Add(peds);

                    // compute the optimal position for insertion
                    for (auto&& ped: peds)
                    {
                         ped->SetPos(Point(15,15),true);
                    }
                    finished=false;
                    cout<<"Agents generated: "<<peds.size()<<endl;
               }
               //src->Dump();
          }
          //cout<<"Number of sources: "<<_sources.size()<<endl;

          //wait one second
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
     }while (!finished);
}

void AgentsSourcesManager::AddSource(std::shared_ptr<AgentsSource> src)
{
     _sources.push_back(src);
}

const std::vector<std::shared_ptr<AgentsSource> >& AgentsSourcesManager::GetSources() const
{
     return _sources;
}
