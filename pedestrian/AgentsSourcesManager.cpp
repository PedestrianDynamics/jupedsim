/*
 * AgentsSourcesManager.cpp
 *
 *  Created on: 14.04.2015
 *      Author: piccolo
 */

#include "AgentsSourcesManager.h"
#include "Pedestrian.h"
#include "StartDistribution.h"
#include "PedDistributor.h"
#include "AgentsSource.h"
#include "../voronoi/VoronoiDiagramGenerator.h"
#include "../geometry/Building.h"
#include "../geometry/Point.h"


#include "../mpi/LCGrid.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "AgentsQueue.h"

using namespace std;

bool AgentsSourcesManager::_isCompleted=false;

AgentsSourcesManager::AgentsSourcesManager()
{
}

AgentsSourcesManager::~AgentsSourcesManager()
{
}

void AgentsSourcesManager::operator()()
{
     Run();
}

void AgentsSourcesManager::Run()
{
     Log->Write("INFO:\tStarting agent manager thread");


     //Generate all agents required for the complete simulation
     //It might be more efficient to generate at each frequency step
     for (const auto& src : _sources)
     {
          src->GenerateAgentsAndAddToPool(src->GetMaxAgents(), _building);
          cout<<"generation: "<<src->GetPoolSize()<<endl;
     }

     //first call ignoring the return value
     ProcessAllSources();

     //the loop is updated each x second.
     //it might be better to use a timer
     _isCompleted = false;
     bool finished = false;
     long updateFrequency = 5;     // 1 second
     do
     {
          int current_time = Pedestrian::GetGlobalTime();

          //first step
          //if(current_time==0){
          //finished=ProcessAllSources();
          //     ProcessAllSources();
          //     //cout<<"here:"<<endl; exit(0);
          //}
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
     _isCompleted = true;//exit(0);
}

bool AgentsSourcesManager::ProcessAllSources() const
{
     bool empty=true;
     //cout<<"src size: "<<_sources.size()<<endl;
     for (const auto& src : _sources)
     {
          //cout<<"size: "<<src->GetPoolSize()<<endl;//exit(0);
          if (src->GetPoolSize())
          {
               vector<Pedestrian*> peds;
               src->RemoveAgentsFromPool(peds,src->GetFrequency());

               ComputeBestPositionRandom(src.get(), peds);
               //todo: compute the optimal position for insertion using voronoi
               //for (auto&& ped : peds)
               //{
               //ComputeBestPositionVoronoi(src.get(), ped);
               //ped->Dump(ped->GetID());
               //}
               AgentsQueueIn::Add(peds);
               empty = false;
          }
          //src->Dump();//exit(0);
     }
     return empty;
}

void AgentsSourcesManager::ComputeBestPositionVoronoi(AgentsSource* src,
          Pedestrian* agent) const
{
     auto dist = src->GetStartDistribution();
     double bounds[4];
     dist->Getbounds(bounds);
     int roomID = dist->GetRoomId();
     int subroomID = dist->GetSubroomID();

     //Get all pedestrians in that location
     vector<Pedestrian*> peds;
     _building->GetPedestrians(roomID, subroomID, peds);

     //filter the points that are not within the boundaries
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
               double distance = (pos - ped->GetPos()).NormSquare();
               if (distance < min_dist)
               {
                    min_dist = distance;
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
          std::vector<Pedestrian*>& peds) const
{

     //generate the agents with default positions
     auto dist = src->GetStartDistribution();
     auto subroom = _building->GetRoom(dist->GetRoomId())->GetSubRoom(
               dist->GetSubroomID());
     vector<Point> positions = PedDistributor::PossiblePositions(*subroom);
     double bounds[4] = { 0, 0, 0, 0 };
     dist->Getbounds(bounds);

     vector<Point> extra_positions;

     for (auto& ped : peds)
     {
          //need to be called at each iteration
          SortPositionByDensity(positions, extra_positions);

          int index = -1;

          //in the case a range was specified
          //just take the first element
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
                              "ERROR:\t AgentSourceManager Cannot distribute pedestrians in the mentioned area [%0.2f,%0.2f,%0.2f,%0.2f]",
                              bounds[0], bounds[1], bounds[2], bounds[3]);
                    Log->Write("ERROR:\t Specifying a subroom_id might help");
               }
          }
          else
          {
               const Point& pos = positions[index];
               extra_positions.push_back(pos);
               ped->SetPos(pos, true); //true for the initial position
               positions.erase(positions.begin() + index);

               //at this point we have a position
               //so we can adjust the velocity
               //AdjustVelocityUsingWeidmann(ped);
               AdjustVelocityByNeighbour(ped);
          }
     }
}

void AgentsSourcesManager::AdjustVelocityByNeighbour(Pedestrian* ped) const
{
     //get the density
     vector<Pedestrian*> neighbours;
     _building->GetGrid()->GetNeighbourhood(ped,neighbours);

     double speed=0.0;
     double radius_square=0.56*0.56;//corresponding to an area of 1m3
     int count=0;

     for(const auto& p: neighbours)
     {
          //only pedes in a sepcific rance
          if( (ped->GetPos()-p->GetPos()).NormSquare()<=radius_square)
          {
               //only peds with the same destination
               if(ped->GetExitIndex()==p->GetExitIndex())
               {
                    double dist1=ped->GetDistanceToNextTarget();
                    double dist2=p->GetDistanceToNextTarget();
                    //only peds in front of me
                    if(dist2<dist1)
                    {
                         speed+=p->GetV().Norm();
                         count++;
                    }
               }
          }

     }
     //mean speed
     if(count==0)
     {
          //speed=ped->GetV0Norm();
          speed=ped->GetEllipse().GetV0(); //bad fix for: peds without navline (ar.graf)
     }
     else
     {
          speed=speed/count;
     }

     if(ped->FindRoute()!=-1)
     {
          //get the next destination point
          Point v =(ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
          v=v*speed;
          ped->SetV(v);
     }
     else
     {
          Log->Write("ERROR:\t no route could be found for agent [%d] going to [%d]",ped->GetID(),ped->GetFinalDestination());
          //that will be most probably be fixed in the next computation step.
          // so do not abort
     }

}

void AgentsSourcesManager::AdjustVelocityUsingWeidmann(Pedestrian* ped) const
{
     //get the density
     vector<Pedestrian*> neighbours;
     _building->GetGrid()->GetNeighbourhood(ped,neighbours);

     //density in pers per m2
     double density = 1.0;
     //radius corresponding to a surface of 1m2
     //double radius_square=0.564*0.564;
     double radius_square=1.0;

     for(const auto& p: neighbours)
     {
          if( (ped->GetPos()-p->GetPos()).NormSquare()<=radius_square)
               density+=1.0;
     }
     density=density/(radius_square*M_PI);

     //get the velocity
     double density_max=5.4;

     //speed from taken from weidmann FD
     double speed=1.34*(1-exp(-1.913*(1.0/density-1.0/density_max)));
     if(speed>=ped->GetV0Norm())
     {
          speed=ped->GetV0Norm();
     }

     //set the velocity vector
     if(ped->FindRoute()!=-1)
     {
          //get the next destination point
          Point v =(ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
          v=v*speed;
          ped->SetV(v);
          //cout<<"density: "<<density<<endl;
     }
     else
     {
          Log->Write("ERROR:\t no route could be found for agent [%d] going to [%d]",ped->GetID(),ped->GetFinalDestination());
          //that will be most probably be fixed in the next computation step.
          // so do not abort
     }

}

void AgentsSourcesManager::SortPositionByDensity(std::vector<Point>& positions, std::vector<Point>& extra_positions) const
{
     std::multimap<double,Point> density2pt;
     //std::map<double,Point> density2pt;

     for(auto&& pt:positions)
     {
          vector<Pedestrian*> neighbours;
          _building->GetGrid()->GetNeighbourhood(pt,neighbours);
          //density in pers per m2
          double density = 0.0;
          //double radius_square=0.56*0.56;
          double radius_square=0.40*0.40;

          for(const auto& p: neighbours)
          {
               if( (pt-p->GetPos()).NormSquare()<=radius_square)
                    density+=1.0;
          }

          //consider the extra positions
          for(const auto& ptx: extra_positions)
          {
               if( (ptx-pt).NormSquare()<=radius_square)
                    density+=1.0;
          }
          density=density/(radius_square*M_PI);

          density2pt.insert(std::pair<double,Point>(density,pt));

     }

     //cout<<"------------------"<<positions.size()<<"-------"<<endl;
     positions.clear();
     for(auto&& d: density2pt)
     {
          positions.push_back(d.second);
          //     printf("density [%lf, %s]\n",d.first, d.second.toString().c_str());
     }

}


void AgentsSourcesManager::GenerateAgents()
{

     for (const auto& src : _sources)
     {
          src->GenerateAgentsAndAddToPool(src->GetMaxAgents(), _building);
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


Building* AgentsSourcesManager::GetBuilding() const
{
     return _building;
}

long AgentsSourcesManager::GetMaxAgentNumber() const
{
     long pop=0;
     for (const auto& src : _sources)
     {
          pop+=src->GetMaxAgents();
     }
     return pop;
}
