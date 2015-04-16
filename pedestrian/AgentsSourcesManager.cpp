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
#include "../voronoi/VoronoiDiagramGenerator.h"
#include "../geometry/Building.h"

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
     long updateFrequency=5;// 1 second
     do
     {

          int current_time = Pedestrian::GetGlobalTime();

          if ( (current_time != _lastUpdateTime) &&
                    ((current_time % updateFrequency) == 0))
          {

               finished=true;
               for (const auto& src: _sources)
               {
                    if(src->GetPoolSize())
                    {
                         vector<Pedestrian*> peds;
                         src->GenerateByFrequency(peds);
                         AgentsQueue::Add(peds);

                         ComputeBestPosition(src.get());

                         // compute the optimal position for insertion
                         for (auto&& ped: peds)
                         {
                              ped->SetPos(Point(15,15),true);
                         }
                         finished=false;
                         //cout<<"Agents generated: "<<peds.size()<<endl;
                    }
                    //src->Dump();
               }
               _lastUpdateTime = current_time;
          }
          //wait some time
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
     }while (!finished);
}

void AgentsSourcesManager::ComputeBestPosition(AgentsSource* src)
{
     int roomID=0;
     int subroomID=0;

     //Get all pedestrians in that location
     vector<Pedestrian*> peds;
     _building->GetPedestrians(roomID,subroomID,peds);

     // compute the cells and cut with the bounds
     const int count=peds.size();
     float xValues[count];
     float yValues[count];

     for(int i=0;i<count;i++)
     {
          xValues[i]=peds[i]->GetPos()._x;
          yValues[i]=peds[i]->GetPos()._y;
     }

     VoronoiDiagramGenerator vdg;
     vdg.generateVoronoi(xValues,yValues,count, -100,100,-100,100,3);
     vdg.resetIterator();
     vdg.resetVerticesIterator();

     float x1,y1;

     printf("\n------vertices---------\n");
     while(vdg.getNextVertex(x1,y1))
     {
          printf("GOT Point (%f,%f)\n",x1,y1);
     }


     //exit(0);
     // float x1,y1,x2,y2;
     //while(vdg.getNext(x1,y1,x2,y2))
     //{
     //     printf("GOT Line (%f,%f)->(%f,%f)\n",x1,y1,x2, y2);
     //
     //}
     //compute the best position
     exit(0);
}

void AgentsSourcesManager::AddSource(std::shared_ptr<AgentsSource> src)
{
     _sources.push_back(src);
}

const std::vector<std::shared_ptr<AgentsSource> >& AgentsSourcesManager::GetSources() const
{
     return _sources;
}

void AgentsSourcesManager::SetBuilding(Building* building)
{
   _building=building;
}
