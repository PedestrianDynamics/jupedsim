/*
 * AgentsSourcesManager.cpp
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#include "AgentsSourcesManager.h"
#include "Pedestrian.h"
#include "AgentsQueue.h"
#include "StartDistribution.h"
#include "PedDistributor.h"
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

void AgentsSourcesManager::operator()()
{
     Log->Write("INFO:\tStarting agent manager thread");
     //the loop is updated each second.
     //it might be better to use a timer
     _isCompleted = false;
     bool finished = false;
     long updateFrequency = 5;     // 1 second
     do
     {
          int current_time = Pedestrian::GetGlobalTime();

          if ((current_time != _lastUpdateTime)
                    && ((current_time % updateFrequency) == 0))
          {
               //cout<<"TIME:"<<current_time<<endl;
               finished=ProcessAllSources();
               _lastUpdateTime = current_time;
               cout << "source size: " << _sources.size() << endl;
          }
          //wait some time
          //cout<<"sleepinp..."<<endl;
          //std::this_thread::sleep_for(std::chrono::milliseconds(1));
     } while (!finished);
     Log->Write("INFO:\tTerminating agent manager thread");
     _isCompleted = true;
}

bool AgentsSourcesManager::ProcessAllSources()
{
     bool empty=true;
     for (const auto& src : _sources)
     {
          if (src->GetPoolSize())
          {
               vector<Pedestrian*> peds;
               src->GenerateByFrequency(peds);
               ComputeBestPositionRandom(src.get(), peds);
               // compute the optimal position for insertion
               for (auto&& ped : peds)
               {
                    //ComputeBestPositionVoronoi(src.get(), ped);
                    //ped->SetPos(Point(15,15),true);
                    //ped->Dump(ped->GetID());
               }
               AgentsQueue::Add(peds);
               empty = false;
               cout << "Agents generated: " << peds.size() << endl;
          }
          //src->Dump();//exit(0);
     }
     return empty;
}

void AgentsSourcesManager::ComputeBestPositionVoronoi(AgentsSource* src,
          Pedestrian* agent)
{
     auto dist = src->GetStartDistribution();
     double bounds[4];
     dist->Getbounds(bounds);
     int roomID = dist->GetRoomId();
     int subroomID = dist->GetSubroomID();

     //Get all pedestrians in that location
     vector<Pedestrian*> peds;
     _building->GetPedestrians(roomID, subroomID, peds);

     //filter the points that are not in the boundaries
     for (auto&& iter = peds.begin(); iter != peds.end();)
     {
          const Point& pos = (*iter)->GetPos();
          if ((bounds[0] <= pos._x && pos._x <= bounds[1])
                    && (bounds[1] <= pos._y && pos._y <= bounds[2]))
          {
               iter = peds.erase(iter);
               cout << "removing..." << endl;
               exit(0);
          } else
          {
               ++iter;
          }
     }

     //special case with 1, 2 or only three pedestrians in the area
     if (peds.size() < 3)
     {
          //TODO/random position in the area
          return;

     }
     // compute the cells and cut with the bounds
     const int count = peds.size();
     float* xValues = new float[count];
     float* yValues = new float[count];
     //float xValues[count];
     //float yValues[count];

     for (int i = 0; i < count; i++)
     {
          xValues[i] = peds[i]->GetPos()._x;
          yValues[i] = peds[i]->GetPos()._y;
     }

     VoronoiDiagramGenerator vdg;
     vdg.generateVoronoi(xValues, yValues, count, bounds[0], bounds[1],
               bounds[2], bounds[3], 3);
     vdg.resetIterator();
     vdg.resetVerticesIterator();

     printf("\n------vertices---------\n");
     //collect the positions
     vector<Point> positions;
     float x1, y1;
     while (vdg.getNextVertex(x1, y1))
     {
          printf("GOT Point (%f,%f)\n", x1, y1);
          positions.push_back(Point(x1, y1));
     }

     //look for the biggest spot
     map<double, Point> map_dist_to_position;

     for (auto&& pos : positions)
     {
          double min_dist = FLT_MAX;

          for (auto&& ped : peds)
          {
               double dist = (pos - ped->GetPos()).NormSquare();
               if (dist < min_dist)
               {
                    min_dist = dist;
               }
          }
          map_dist_to_position[min_dist] = pos;
     }

     //list the result
     for (auto&& mp : map_dist_to_position)
     {
          cout << "dist: " << mp.first << " pos: " << mp.second.toString()
                    << endl;
          //agent->SetPos(mp.second, true);
     }

     //the elements are ordered.
     // so the last one has the largest distance
     if (!map_dist_to_position.empty())
     {
          agent->SetPos(map_dist_to_position.rbegin()->second, true);
          cout << "position:" << agent->GetPos().toString() << endl;
          //exit(0);

     } else
     {
          cout << "position not set:" << endl;
          cout << "size: " << map_dist_to_position.size() << endl;
          cout << " for " << peds.size() << " pedestrians" << endl;
          exit(0);
     }
     //exit(0);
     // float x1,y1,x2,y2;
     //while(vdg.getNext(x1,y1,x2,y2))
     //{
     //     printf("GOT Line (%f,%f)->(%f,%f)\n",x1,y1,x2, y2);
     //
     //}
     //compute the best position
     //exit(0);
}

void AgentsSourcesManager::ComputeBestPositionRandom(AgentsSource* src,
          std::vector<Pedestrian*>& peds)
{

     //generate the agents with default positions
     auto dist = src->GetStartDistribution();
     auto subroom = _building->GetRoom(dist->GetRoomId())->GetSubRoom(
               dist->GetSubroomID());
     vector<Point> positions = PedDistributor::PossiblePositions(*subroom);
     double bounds[4] = { 0, 0, 0, 0 };
     dist->Getbounds(bounds);
     //int roomID = dist->GetRoomId();
     //int subroomID = dist->GetSubroomID();
     // first default Position

     for (const auto& ped : peds)
     {
          //ped->Dump(ped->GetID()); continue;
          int index = -1;

          //in the case a range was specified
          for (unsigned int a = 0; a < positions.size(); a++)
          {
               Point pos = positions[a];
               if ((bounds[0] <= pos._x) && (pos._x <= bounds[1])
                         && (bounds[2] <= pos._y) && (pos._y < bounds[3]))
               {
                    index = a;
                    break;
               }
          }
          if (index == -1)
          {
               if (positions.size())
               {
                    Log->Write(
                              "ERROR:\t Cannot distribute pedestrians in the mentioned area [%0.2f,%0.2f,%0.2f,%0.2f]",
                              bounds[0], bounds[1], bounds[2], bounds[3]);
                    Log->Write("ERROR:\t Specifying a subroom_id might help");
               }
          } else
          {
               const Point& pos = positions[index];
               ped->SetPos(pos, true); //true for the initial position
               positions.erase(positions.begin() + index);

               //               const Point& start_pos =  Point(_startX, _startY);
               //               if ((std::isnan(start_pos._x) == 0) && (std::isnan(start_pos._y) == 0))
               //               {
               //                    if (_building->GetRoom(ped->GetRoomID())->GetSubRoom(ped->GetSubRoomID())->IsInSubRoom(
               //                              start_pos) == false)
               //                    {
               //                         Log->Write(
               //                                   "ERROR: \t cannot distribute pedestrian %d in Room %d at fixed position %s",
               //                                   *pid, GetRoomId(), start_pos.toString().c_str());
               //                         Log->Write(
               //                                   "ERROR: \t Make sure that the position is inside the geometry and belongs to the specified room / subroom");
               //                         exit(EXIT_FAILURE);
               //                    }
               //
               //                    ped->SetPos(start_pos, true); //true for the initial position
               //                    Log->Write("INFO: \t fixed position for ped %d in Room %d %s", *pid, GetRoomId(),
               //                              start_pos.toString().c_str());
               //               }
          }
     }

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
     _building = building;
}

bool AgentsSourcesManager::IsCompleted() const
{
     return _isCompleted;
}
