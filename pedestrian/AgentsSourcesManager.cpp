/**
 * \file        AgentsSourcesManager.cpp
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
 * This class is responsible for materialising agent in a given location at a given frequency up to a maximum number.
 * The optimal position where to put the agents is given by various algorithms, for instance
 * the Voronoi algorithm or the Mitchell Best candidate algorithm.
 *
 **/
#include "AgentsSourcesManager.h"

#include "AgentsQueue.h"
#include "Pedestrian.h"

#include "mpi/LCGrid.h"
#include "voronoi-boost/VoronoiPositionGenerator.h"

#include <thread>

bool AgentsSourcesManager::_isCompleted=true;

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
      SetRunning(true);
     Log->Write("INFO:\tStarting agent manager thread");
     std::cout<< KRED << "\n Starting agent manager thread\n" << ">> time: " << Pedestrian::GetGlobalTime() << RESET << "\n";
     //Generate all agents required for the complete simulation
     //It might be more efficient to generate at each frequency step
     //TODO  this loop is exactly GenerateAgents( --> REFACTOR)
     for (const auto& src : _sources)
     {
          std::cout << "Generate src: " << src->GetId() << "\n" ;
          src->GenerateAgentsAndAddToPool(src->GetMaxAgents(), _building);
     }

     //first call ignoring the return value
     ProcessAllSources();

     //the loop is updated each x second.
     //it might be better to use a timer
     _isCompleted = false;
     bool finished = false;
     SetBuildingUpdated(false);
     long updateFrequency = 1; // @todo parse this from inifile
     /* std::cout << KMAG << "RUN Starting thread manager with _lastUpdateTime " << _lastUpdateTime<< std::endl; */
     do
     {
          int current_time = (int)Pedestrian::GetGlobalTime();
          // std::cout << KBLU << ">> RUN: current_time " << current_time << " last update  " << _lastUpdateTime << "\n" << RESET;

          if ((current_time != _lastUpdateTime)
              && ((current_time % updateFrequency) == 0))
          {
                /* std::cout << "   ---  Enter IF  --- \n" << KRED << "QUEUE isempty: " << AgentsQueueIn::IsEmpty() << "\n" << RESET; */
                if(AgentsQueueIn::IsEmpty())
                //if queue is empty. Otherwise, wait for main thread to empty it and update _building
                {
                      /* std::cout << "   ---  Enter QUEUE EMPTY --- \n"; */
                      finished=ProcessAllSources();
                      _lastUpdateTime = current_time;
                      //SetBuildingUpdated(false);
                }
          }
          // wait for main thread to update building
          if(current_time >= GetMaxSimTime())
                break; // break if max simulation time is reached.

     } while (!finished);
     Log->Write("INFO:\tTerminating agent manager thread");
     _isCompleted = true;
}

bool AgentsSourcesManager::ProcessAllSources() const
{
     // std::cout << "\nSTART   AgentsSourcesManager::ProcessAllSources()\n";

     bool empty=true;
     double current_time = (int)Pedestrian::GetGlobalTime();
     std::vector<Pedestrian*> source_peds; // we have to collect peds from all sources, so that we can consider them  while computing new positions
     for (const auto& src : _sources)
     {
          // std::cout << KRED << "\nprocessing src: " <<  src->GetId() << " -- current time: " << current_time << " schedule time: " << src->GetPlanTime() <<". number of peds in building " << _building->GetAllPedestrians().size() << "\n" << RESET;

          auto srcLifeSpan = src-> GetLifeSpan();
          bool inTime = (current_time >= srcLifeSpan[0]) && (current_time <= srcLifeSpan[1]);
          // inTime is always true if src got some PlanTime (default values
          // if src has no PlanTime, then this is set to 0. In this case inTime
          // is important in the following condition
          bool newCycle =  std::fmod(current_time, src->GetFrequency()) == 0;
          bool subCycle;
          subCycle = (current_time > src->GetFrequency())?std::fmod((current_time-src->GetFrequency()), src->GetRate()) == 0:false;

          if(newCycle)
               src->ResetRemainingAgents();

          bool timeToCreate = newCycle || subCycle;
          // if(subCycle)
          //      std::cout << KGRN << " freq: " << src->GetFrequency() << ", rate: " << src->GetRate() << ", " << ": remaining: " << src->GetRemainingAgents() <<"\n" << RESET;                                                                                                                                                             std::cout << " <<<<  time to create " <<  timeToCreate  << "  newCycle: " << newCycle << ", subcycle: " << subCycle << ", inTime: " << inTime<< "\n";


          if (timeToCreate && src->GetPoolSize() && (src->GetPlanTime() <= current_time) && inTime && src->GetRemainingAgents())// maybe diff<eps
          {
               std::vector<Pedestrian*> peds;

               src->RemoveAgentsFromPool(peds, src->GetChunkAgents() * src->GetPercent());
               src->UpdateRemainingAgents(src->GetChunkAgents() * src->GetPercent());

               source_peds.reserve(source_peds.size() + peds.size());

               Log->Write("\nINFO:\tSource %d generating %d agents at %3.3f s, %d (%d remaining in pool)\n",src->GetId(),peds.size(), current_time,src->GetRemainingAgents(),src->GetPoolSize());
               printf("\nINFO:\tSource %d generating %lu agents (%d remaining)\n",src->GetId(), peds.size(),src->GetPoolSize());

               //ComputeBestPositionRandom(src.get(), peds);
               //todo: here every pedestrian needs an exitline
                if(!std::isnan(src->GetStartX()) && !std::isnan(src->GetStartY()))
                {
                      printf("INFO:\tSet source agent on fixed position (%.2f, %.2f)", src->GetStartX(), src->GetStartY());
                      InitFixedPosition(src.get(), peds);
                }
                else
                     if( !ComputeBestPositionVoronoiBoost(src.get(), peds, _building, source_peds) )
                          Log->Write("WARNING:\tThere was no place for some pedestrians");

               source_peds.insert(source_peds.end(), peds.begin(), peds.end());
               /* std::cout << KRED << ">>  Add to queue " << peds.size() << "\n" << RESET; */
                /* for( auto pp: peds) */
                /*       std::cout << "id: "<< pp->GetID() << "  pos " << pp->GetPos()._x << ", " << pp->GetPos()._y << "\n"; */
                /* std::cout << "------\n"; */
                /* for( auto pp: source_peds) */
                /*       std::cout  <<  "id: "<< pp->GetID() << "  POS " << pp->GetPos()._x << ", " << pp->GetPos()._y << "\n"; */

               AgentsQueueIn::Add(peds);
               empty = false;
               //src->Dump();
          }
          bool timeConstraint = (src->GetPlanTime() > current_time) || (current_time < srcLifeSpan[1]);
          if (timeConstraint) // for the case we still expect
               // agents coming
               empty = false;

     }
     // std::cout << "LEAVE   AgentsSourcesManager::ProcessAllSources()\n";
     // std::cout << current_time << "\n";

     // std::cout << " Source building: "<<  _building << " size "  << _building->GetAllPedestrians().size()<< " empty = " << empty << std::endl;

     // for(auto pp: _building->GetAllPedestrians())
     //      std::cout<< KBLU << "BUL: agentssourcesManager: " << pp->GetPos()._x << ", " << pp->GetPos()._y << RESET << std::endl;
     //
// std::cout << "========================\n";
          return empty;
}


void AgentsSourcesManager::InitFixedPosition(AgentsSource* src,
                                                    std::vector<Pedestrian*>& peds)const
{
     for(auto&& ped : peds)
     {
          Point v;
          if (ped->GetExitLine()) {
               v = (ped->GetExitLine()->ShortestPoint(ped->GetPos())- ped->GetPos()).Normalized();
          } else {
               auto transitions = ped->GetBuilding()->GetAllTransitions();
               auto transition = transitions[0];
               int trans_ID = transition->GetID();
               ped->SetExitLine(transition); // set dummy line
               ped->SetExitIndex(trans_ID);
               //ped->SetFinalDestination(trans_ID);
               v = Point(0., 0.);
          }
          double speed=ped->GetEllipse().GetV0();
          v=v*speed;
          ped->SetV(v);
          ped->SetPos( Point(src->GetStartX(), src->GetStartY()) );
     }
}

void AgentsSourcesManager::ComputeBestPositionCompleteRandom(AgentsSource* src,
                                                             std::vector<Pedestrian*>& peds)const
{
     auto dist = src->GetStartDistribution();
     auto subroom = _building->GetRoom(dist->GetRoomId())->GetSubRoom(dist->GetSubroomID());
     auto config=GetBuilding()->GetConfig();
     auto distributor = std::unique_ptr<PedDistributor>(new PedDistributor(config));
     std::vector<Point> positions = distributor->PossiblePositions(*subroom);
     double seed = time(0);
     //TODO: get the seed from the simulation
     std:: cout << "seed: "<< seed << std::endl;

//     srand (seed);

     for (auto& ped : peds)
     {
          if( positions.size() )
          {
               int index = rand()%positions.size();
               Point new_pos = positions[index];
               positions.erase(positions.begin() + index);
               ped->SetPos(new_pos, true);
               std:: cout << "pos: " << new_pos._x << new_pos._y << std::endl;

               AdjustVelocityByNeighbour(ped);
          }
          else
          {
               Log->Write("\t No place for a pedestrian");
               break;
          }
     }

}

/*
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
  cout << "removing (testing only)..." << endl;
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
*/



void AgentsSourcesManager::ComputeBestPositionRandom(AgentsSource* src,
                                                     std::vector<Pedestrian*>& peds) const
{

     //generate the agents with default positions
     auto dist = src->GetStartDistribution();
     auto subroom = _building->GetRoom(dist->GetRoomId())->GetSubRoom(
          dist->GetSubroomID());
     auto config=GetBuilding()->GetConfig();
     auto distributor = std::unique_ptr<PedDistributor>(new PedDistributor(config));
     std::vector<Point> positions = distributor->PossiblePositions(*subroom);
     double bounds[4] = { 0, 0, 0, 0 };
     dist->Getbounds(bounds);

     std::vector<Pedestrian*> peds_without_place;

     std::vector<Point> extra_positions;

     std::vector<Pedestrian*>::iterator iter_ped;
     for (iter_ped = peds.begin(); iter_ped != peds.end(); )
     {
          //need to be called at each iteration
          SortPositionByDensity(positions, extra_positions);

          int index = -1;
          double radius = ( (*iter_ped)->GetEllipse() ).GetBmax()   ;

          //in the case a range was specified
          //just take the first element
          for (unsigned int a = 0; a < positions.size(); a++)
          {
               Point pos = positions[a];
               //cout<<"checking: "<<pos.toString()<<endl;
               // for positions inside bounds, check it there is enough space
               if ((bounds[0] <= pos._x) && (pos._x <= bounds[1])
                   && (bounds[2] <= pos._y) && (pos._y < bounds[3]))
               {

                    bool enough_space = true;

                    //checking enough space!!
                    std::vector<Pedestrian*> neighbours;
                    _building->GetGrid()->GetNeighbourhood(pos,neighbours);

                    for (const auto& ngh: neighbours)
                         if(  (ngh->GetPos() - pos).NormSquare() < 4*radius*radius )
                         {
                              enough_space = false;
                              break;
                         }


                    if( enough_space )
                    {
                         index = a;
                         break;
                    }

               }
          }
          if (index == -1)
          {
               if (positions.size())
               {
                    Log->Write(
                         "ERROR:\t AgentSourceManager Cannot distribute pedestrians in the mentioned area [%0.2f,%0.2f,%0.2f,%0.2f]",
                         bounds[0], bounds[1], bounds[2], bounds[3]);
                    Log->Write("     \t Specifying a subroom_id might help");
                    Log->Write("     \t %d positions were available",positions.size());
                    //exit(EXIT_FAILURE);
               }
               //dump the pedestrian, move iterator
               peds_without_place.push_back(*iter_ped);
               iter_ped=peds.erase(iter_ped);
          }
          else //we found a position with enough space
          {
               const Point& pos = positions[index];

               extra_positions.push_back(pos);
               (*iter_ped)->SetPos(pos, true); //true for the initial position
               positions.erase(positions.begin() + index);

               //at this point we have a position
               //so we can adjust the velocity
               //AdjustVelocityUsingWeidmann(ped);
               AdjustVelocityByNeighbour( (*iter_ped) );
               //move iterator
               iter_ped++;

          }

          //return the pedestrians without place
     }
     if(peds_without_place.size()>0)
          src->AddAgentsToPool(peds_without_place);
}

void AgentsSourcesManager::AdjustVelocityByNeighbour(Pedestrian* ped) const
{
     //get the density
     std::vector<Pedestrian*> neighbours;
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
          speed=ped->GetEllipse().GetV0(); // FIXME:  bad fix for: peds without navline (ar.graf)
          //speed=ped->GetV0Norm();
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
          Log->Write(">> ERROR:\t no route could be found for agent [%d] going to [%d]",ped->GetID(),ped->GetFinalDestination());
          //that will be most probably be fixed in the next computation step.
          // so do not abort
     }

}

void AgentsSourcesManager::AdjustVelocityUsingWeidmann(Pedestrian* ped) const
{
     //get the density
     std::vector<Pedestrian*> neighbours;
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
          Log->Write(">>> SOURCE ERROR:\t no route could be found for agent [%d] going to [%d]",ped->GetID(),ped->GetFinalDestination());
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
          std::vector<Pedestrian*> neighbours;
          _building->GetGrid()->GetNeighbourhood(pt,neighbours);
          //density in pers per m2
          double density = 0.0;
          double radius_square=0.40*0.40;

          for(const auto& p: neighbours)
          {
               //FIXME: p  can be null, if deleted in the main simulation thread.
               if( p && (pt-p->GetPos()).NormSquare()<=radius_square)
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
     _isCompleted=false;//at least one source was provided
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

bool AgentsSourcesManager::IsRunning() const
{
     return _isRunning;
}


bool AgentsSourcesManager::IsBuildingUpdated() const
{
     return _buildingUpdated;
}

void AgentsSourcesManager::SetBuildingUpdated(bool update)
{
     _buildingUpdated = update;
}


void AgentsSourcesManager::SetRunning(bool running)
{
     _isRunning = running;
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


 int AgentsSourcesManager::GetMaxSimTime() const{
      return maxSimTime;
}
void AgentsSourcesManager::SetMaxSimTime(int t){
      maxSimTime = t;
}
