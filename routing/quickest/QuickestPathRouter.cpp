/**
 * \file        QuickestPathRouter.cpp
 * \date        Apr 20, 2011
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
 *
 **/
#include "QuickestPathRouter.h"

#include "geometry/SubRoom.h"
#include "geometry/Wall.h"
#include "mpi/LCGrid.h"

#include <tinyxml.h>

QuickestPathRouter::QuickestPathRouter( ):GlobalRouter() { }

QuickestPathRouter::~QuickestPathRouter() { }

bool QuickestPathRouter::Init(Building* building)
{
     Log->Write("INFO:\tInit Quickest Path Router Engine");

     // prefer path through corridors to path through rooms
     SetEdgeCost(2.0);
     if (!GlobalRouter::Init(building))
          return false;

     if (!ParseAdditionalParameters())
          return false;

     // activate the spotlight for tracking some pedestrians
     //Pedestrian::SetColorMode(AgentColorMode::BY_SPOTLIGHT);

     //vector<string> rooms;
     //rooms.push_back("150");
     //rooms.push_back("outside");
     //WriteGraphGV("routing_graph.gv",FINAL_DEST_ROOM_040,rooms);
     //WriteGraphGV("routing_graph.gv",FINAL_DEST_OUT,rooms);
     //DumpAccessPoints(1185);
     //exit(0);
     Log->Write("INFO:\tDone with Quickest Path Router Engine!");
     return true;
}

int QuickestPathRouter::FindExit(Pedestrian* ped)
{
     //ped->ClearMentalMap();
     int next=FindNextExit(ped);

     // that ped will be deleted
     if(next==-1) return next;

     if(ped->IsFeelingLikeInJam())
     {
          //ped->SetSpotlight(true);
          if(isCongested(ped))
          {
               Redirect(ped);
               ped->ResetTimeInJam();
               //ped->SetSpotlight(true);
          }
          //ped->RerouteIn(2.50); // seconds
     }
     else if(ped->IsReadyForRerouting())
     {
          Redirect(ped);
          ped->ResetRerouting();

     }
     else
     {
          ped->UpdateReroutingTime();
     }
     return next;
}

int QuickestPathRouter::FindNextExit(Pedestrian* ped)
{
     int nextDestination = ped->GetNextDestination();
     //ped->Dump(1);

     if (nextDestination == -1)
     {
          return GetBestDefaultRandomExit(ped);

     } else {

          SubRoom* sub = _building->GetRoom(ped->GetRoomID())->GetSubRoom(
                    ped->GetSubRoomID());

          for(const auto& apID: sub->GetAllGoalIDs())
          {
               AccessPoint* ap = _accessPoints[apID];

               const Point& pt3 = ped->GetPos();
               double distToExit = ap->GetNavLine()->DistTo(pt3);

               if (distToExit > J_EPS_DIST)       //if anyone understands this, please write comment
                    continue;

               nextDestination = GetQuickestRoute(ped);
               //uncomment these lines to return to the gsp
               //nextDestination = ap->GetNearestTransitAPTO(ped->GetFinalDestination());

               if (nextDestination == -1)
               { // we are almost at the exit
                    return ped->GetNextDestination();
               }
               else
               {
                    //check that the next destination is in the actual room of the pedestrian
                    if (!_accessPoints[nextDestination]->isInRange(
                              sub->GetUID()))
                    {
                         //return the last destination if defined
                         int previousDestination = ped->GetNextDestination();

                         //we are still somewhere in the initialization phase
                         if (previousDestination == -1)
                         {
                              ped->SetExitIndex(apID);
                              ped->SetExitLine(_accessPoints[apID]->GetNavLine());
                              return apID;
                         }
                         else
                         { // we are still having a valid destination, don't change
                              return previousDestination;
                         }
                    }
                    else
                    { // we have reached the new room
                         ped->SetExitIndex(nextDestination);
                         ped->SetExitLine(
                                   _accessPoints[nextDestination]->GetNavLine());
                         return nextDestination;
                    }
               }
          }

          // still have a valid destination, so return it
          return nextDestination;
     }
}


double QuickestPathRouter::CBA (double ref_g1, double comp_g2)
{
     return (comp_g2-ref_g1)/(ref_g1+comp_g2);
}


int QuickestPathRouter::GetQuickestRoute(Pedestrian* ped)
{

     //int preferredExit=nearestAP->GetNearestTransitAPTO(ped->GetFinalDestination());
     int preferredExit=GetBestDefaultRandomExit(ped);
     double preferredExitTime=FLT_MAX;
     int quickest=-1;
     double minTime=FLT_MAX;

     // get all AP connected to the nearest
     //const vector<AccessPoint*>& aps = nearestAP->GetConnectingAPs();

     //const vector<AccessPoint*>& aps = nearestAP->GetTransitAPsTo(ped->GetFinalDestination());

     std::vector <AccessPoint*> aps;

     GetRelevantRoutesTofinalDestination(ped,aps);


     //special case where there is only one alternative
     if(aps.size()==1) return preferredExit;


     //in some cases the nearestAP might be included. Remove it
     //     aps.erase(
     //         std::remove_if(aps.begin(), aps.end(),
     //             [&nearestAP](AccessPoint * o) { return o->GetID()==nearestAP->GetID();}),
     //             aps.end());

     //select the optimal time
     for(unsigned int ap=0; ap<aps.size(); ap++)
     {
          int exitid=aps[ap]->GetID();
          //          if(exitid==nearestAP->GetID()){
          //               printf("\n Ignoring: %d out of %ld\n",exitid,aps.size());
          //               continue;
          //          }
          double time= GetEstimatedTravelTimeVia(ped,exitid);

          if(time<minTime)
          {
               minTime=time;
               quickest=exitid;
          }

          //also save the time for the default destinations for later comparison
          if (exitid==preferredExit)
          {
               preferredExitTime=time;
          }

     }
     //compare it with my preferred (shortest nearest)
     if(quickest==preferredExit) return quickest;

     double cba = CBA(gain(preferredExitTime),gain(minTime));

     //cout<<"cba:" <<cba<<endl;
     if (cba<_cbaThreshold) return preferredExit;

     return quickest;
}


double QuickestPathRouter::gain(double time)
{
     return 1.0/time;
}


bool QuickestPathRouter::SelectReferencePedestrian(Pedestrian* myself, Pedestrian** myref, double jamThreshold, int exitID, int* flag)
{
     *flag=FREE_EXIT; // assume free exit

     Hline* crossing=_building->GetTransOrCrossByUID(exitID);
     if(!crossing) return false;

     double radius=3.0;//start radius for looking at the reference in metres
     bool done=false;

     do {
          std::vector<Pedestrian*> queue;
          queue.reserve(250);
          GetQueueAtExit(crossing,jamThreshold,radius,queue,myself->GetSubRoomID());

          if(queue.size()==0)
          {
               //check if I can see/reach the exit without much effort
               if(IsDirectVisibilityBetween(myself,crossing))
               {
                    *myref=nullptr;
                    *flag=FREE_EXIT;
               }
               else
               {
                    *myref=nullptr;
                    *flag=UNREACHEABLE_EXIT;
               }
               // we should return here as there is no queue
               done=true;
          }
          else
          {
               double closestDistance=FLT_MAX;
               //select a reference pedestrian I can see
               for(unsigned int p=0; p<queue.size(); p++)
               {
                    Pedestrian* ped = queue[p]; //ped->SetSpotlight(true);
                    if(IsDirectVisibilityBetween(myself,ped)==false) continue;
                    double dist= (ped->GetPos()-myself->GetPos()).NormSquare();
                    //cout<<"suspect found 1 @ "<< dist<< " { "<< closestDistance<<" }"<<endl;
                    if(dist<closestDistance)
                    {
                         closestDistance=dist;
                         *myref=ped;
                         *flag=REF_PED_FOUND; // at least one reference was found
                         done=true;
                    }
               }
          }

          // we could not find any visible reference so far, we widen our sight range
          // and to the same again
          radius=radius*2;
          if(radius>100)
          {
               if(queue.size()>0)
               { // there were some ref pedes only not visible
                    *myref=nullptr;
                    *flag=UNREACHEABLE_EXIT;
                    done=true;

               }
               else
               {
                    *myref=nullptr;
                    *flag=UNREACHEABLE_EXIT;
                    //done=true; //this line has no effect, cause of return statement below
                    Log->Write("ERROR: reference ped cannot be found for ped %d within [%f] "
                              "m  around the exit [%d]\n",myself->GetID(),radius,crossing->GetID());
                    return false;
               }
          }

     } while (!done);


     //debug area
//     if(*myref) {
//           if(myself->GetID()==-488){
//            myself->SetSpotlight(true);
//               (*myref)->SetSpotlight(true);
//               (*myref)->Dump((*myref)->GetID());
//           }
//
//     } else {
//          cout<<"no ref ped found: " <<endl;
//          getc(stdin);
//     }
     return true;
}

void QuickestPathRouter::GetQueueAtExit(Hline* hline, double minVel,
          double radius, std::vector<Pedestrian*>& queue,int subroomToConsider)
{

     SubRoom* sbr1 = hline->GetSubRoom1();

     //tentative upgrade to Crossing for getting the second subroom
     SubRoom* sbr2 = nullptr;
     if(Crossing* cros=dynamic_cast<Crossing*>(hline))
     {
          sbr2=cros->GetSubRoom2();
     }
     //int exitID=crossing->GetID();
     int exitID=hline->GetUniqueID();
     double radius2=radius*radius;
     double minVel2=minVel*minVel;

     //if this is a hline
     if(sbr1==sbr2)
     {
          sbr2=nullptr;
     }

     if (sbr1 && (sbr1->GetSubRoomID()==subroomToConsider))
     {
          //double closestDistance=FLT_MAX;
          std::vector<Pedestrian*> peds;
          _building->GetPedestrians(sbr1->GetRoomID(),sbr1->GetSubRoomID(),peds);

          for(const auto& ped:peds)
          {
               if(ped->GetExitIndex()==exitID)
               {
                    if(ped->GetV().NormSquare()<minVel2)
                    {
                         double dist= (ped->GetPos()-hline->GetCentre()).NormSquare();
                         //cout<<"suspect found 1 @ "<< dist<< " { "<< closestDistance<<" }"<<endl;
                         if(dist<radius2)
                         {
                              queue.push_back(ped);
                         }
                    }
               }
          }
     }

     if (sbr2 && (sbr2->GetSubRoomID()==subroomToConsider))
     {
          //double closestDistance=FLT_MAX;
          std::vector<Pedestrian*> peds;
          _building->GetPedestrians(sbr2->GetRoomID(),sbr2->GetSubRoomID(),peds);

          for(const auto& ped:peds)
          {
               if(ped->GetExitIndex()==exitID)
               {
                    if(ped->GetV().NormSquare()<minVel2)
                    {
                         double dist= (ped->GetPos()-hline->GetCentre()).NormSquare();
                         //cout<<"distance: radius"<<dist<<":"<<radius<<endl;
                         //cout<<"suspect found 1 @ "<< dist<< " { "<< closestDistance<<" }"<<endl;
                         if(dist<radius2)
                         {
                              queue.push_back(ped);
                         }
                    }
               }
          }
     }

     //cout<<"queue size:"<<queue.size()<<endl;
     //cout<<"mean val:"<<minVel2<<endl;
}

bool QuickestPathRouter::IsDirectVisibilityBetween(Pedestrian* ped, Pedestrian* ref)
{
     int ignore_ped1 = ped->GetID();
     int ignore_ped2 = ref->GetID();
     Hline* ignore_hline = _building->GetTransOrCrossByUID(ref->GetExitIndex());

     //can happen, if the pedestrian is suddenly in a different room ( e.g went through a wall)
     if(!ignore_hline) return false;

     unsigned int obstacles = GetObstaclesCountBetween(ped->GetPos(), ref->GetPos(),
               ignore_hline, ignore_ped1, ignore_ped2);

     return _visibilityObstruction<obstacles ? false : true;
}

bool QuickestPathRouter::IsDirectVisibilityBetween(Pedestrian* myself, Hline* hline)
{
     int ignore_ped1 = myself->GetID();
     int ignore_ped2 = -1;     //there is no second ped to ignore
     unsigned int obstacles = GetObstaclesCountBetween(myself->GetPos(),
               hline->GetCentre(), hline, ignore_ped1, ignore_ped2);

     return obstacles > _visibilityObstruction ? false : true;
}

unsigned int QuickestPathRouter::GetObstaclesCountBetween(const Point& p1, const Point& p2, Hline* hline,
        int ignore_ped1, int ignore_ped2)
{

     SubRoom* sbr1 = hline->GetSubRoom1();

     //tentative upgrade to Crossing for getting the second subroom
     SubRoom* sbr2 = nullptr;
     if(Crossing* cros=dynamic_cast<Crossing*>(hline))
     {
          sbr2=cros->GetSubRoom2();
     }

     Line visibilityLine = Line(p1,p2);

     int exitID=hline->GetID();
     unsigned int obstacles=0;

     //if this is a hline
     if(sbr1==sbr2)
     {
          sbr2=nullptr;
     }

     if (sbr1)
     {
          std::vector<Pedestrian*> peds;
          _building->GetPedestrians(sbr1->GetRoomID(),sbr1->GetSubRoomID(),peds);

          for(const auto& ped:peds)
          {
               //avoiding myself
               if(ped->GetID()==ignore_ped1) continue;
               if(ped->GetID()==ignore_ped2) continue;
               // pedestrian going in that direction are not obstacles to me
               if(ped->GetExitIndex()==exitID) continue;

               if(visibilityLine.IntersectionWithCircle(ped->GetPos())) {
                    obstacles++;
                    if(obstacles>_visibilityObstruction) return obstacles;
               }

          }
     }

     if (sbr2) {
          std::vector<Pedestrian*> peds;
          _building->GetPedestrians(sbr2->GetRoomID(),sbr2->GetSubRoomID(),peds);

          for(const auto& ped:peds)
          {
               //avoiding myself
               if(ped->GetID()==ignore_ped1) continue;
               if(ped->GetID()==ignore_ped2) continue;
               // pedestrian going in that direction are not obstacles to me
               if(ped->GetExitIndex()==exitID) continue;

               if(visibilityLine.IntersectionWithCircle(ped->GetPos()))
               {
                    obstacles++;
                    if(obstacles>_visibilityObstruction) return obstacles;
               }
          }
     }

     return obstacles;
}

int QuickestPathRouter::isCongested(Pedestrian* ped)
{
     //define as the ratio of people in front of me and behind me
     std::vector<Pedestrian*> allPeds;
     _building->GetPedestrians(ped->GetRoomID(),ped->GetSubRoomID(),allPeds);

     //in the case there are only few people in the room
     //revise this condition
     if(allPeds.size()<=_visibilityObstruction) return false;

     double myDist=ped->GetDistanceToNextTarget();
     double inFrontofMe=0;
     double behindMe=0;

     for(const auto& ped2:allPeds)
     {
          //only consider  pedestrians that are going in my direction
          // caution this will not work with hlines
          if(ped2->GetExitIndex()!=ped->GetExitIndex()) continue;
          // skip myself
          if(ped2->GetID()==ped->GetID()) continue;

          if(myDist>ped2->GetDistanceToNextTarget())
          {
               inFrontofMe++;
          }
          else
          {
               behindMe++;
          }
     }

     double ratio = inFrontofMe / (inFrontofMe + behindMe);

     // the threshold is defined by the middle of the queue
     return ratio > 0.5 ? true : false;

}

double QuickestPathRouter::GetEstimatedTravelTimeVia(Pedestrian* ped, int exitid)
{
     //select a reference pedestrian
     Pedestrian* myref=nullptr;
     int flag=FREE_EXIT; //assume free exit
     SelectReferencePedestrian(ped,&myref,_queueVelocityFromJam,exitid,&flag);

     AccessPoint* ap=_accessPoints[exitid];

     // compute the time
     double time=FLT_MAX;

     // case of free exit
     if((myref==nullptr)&& (flag==FREE_EXIT))
     {
          double t1 = (ped->GetPos()- ap->GetCentre()).Norm()/ped->GetV0Norm();
          // time to reach the AP
          //double t1 = (ped->GetPos()- ap->GetCentre()).Norm()/ped->GetV().Norm();

          //guess time from the Ap to the outside
          double t2 = (ap->GetDistanceTo(ped->GetFinalDestination()))/ped->GetV0Norm();

          time=t1+t2;
     }

     // case of unreachable exit
     if((myref==nullptr)&& (flag==UNREACHEABLE_EXIT))
     {
          time= FLT_MAX;
     }

     // case of ref ped
     if((myref!=nullptr) && (flag==REF_PED_FOUND))
     {

          //time to reach the reference
          double t1= (ped->GetPos()- myref->GetPos()).Norm()/ped->GetMeanVelOverRecTime();
          //double t1= (ped->GetPos()- myref->GetPos()).Norm()/ped->GetV().Norm();
          //double t1= (ped->GetPos()- myref->GetPos()).Norm()/ped->GetV0Norm();

          if(myref->GetV().Norm()==0.0)
          {
               //Log->Write("WARNING:\t the reference pedestrian velocity is zero !");
          }
          //time for the reference to get out
          //double t2=(myref->GetPos() -  ap->GetCentre()).Norm()/myref->GetV().Norm();
          double t2=(myref->GetPos() - ap->GetCentre()).Norm()/myref->GetMeanVelOverRecTime();

          //guess time from the Ap to the outside
          //double t3 = (ap->GetDistanceTo(ped->GetFinalDestination()))/ped->GetV().Norm();
          // we assume the desired velocity, because we cannot see anything
          double t3 = (ap->GetDistanceTo(ped->GetFinalDestination()))/ped->GetV0Norm();

          time=t1+t2+t3;
     }

     if((myref==nullptr) && (flag==REF_PED_FOUND))
     {
          Log->Write("ERROR:\t Fatal Error in Quickest Path Router");
          Log->Write("      \t reference pedestrians is nullptr");
          time= FLT_MAX;
     }

     return time;
}

void QuickestPathRouter::Redirect(Pedestrian* ped)
{
     int preferredExit=ped->GetExitIndex();
     double preferredExitTime=FLT_MAX;
     int quickest=-1;
     double minTime=FLT_MAX;
     //ped->SetSpotlight(true);

     //only redirect to other final exits in the actual room.
     // if there is no final exit in the sight range,
     // then no redirection is possible

     // collect the possible alternatives
     Room* room=_building->GetRoom(ped->GetRoomID());
     SubRoom* sub=room->GetSubRoom(ped->GetSubRoomID());

     //const vector<int>& goals=room->GetAllTransitionsIDs();
     //filter to keep only the emergencies exits.

     std::vector <AccessPoint*> relevantAPs;
     GetRelevantRoutesTofinalDestination(ped,relevantAPs);

     if(relevantAPs.size()==0)
     {
          //Log->Write("WARNING:\t Cannot redirect the pedestrian [%d]", ped->GetID());
          return;
     }

     for(const auto& ap:relevantAPs)
     {
          int exitid=ap->GetID();
          //              }
          //
          //              for(unsigned int g=0;g<goals.size();g++){
          //                      int exitid=goals[g];
          //                      AccessPoint* ap=_accessPoints[exitid];

          //only final are accounted
          //if(ap->GetFinalExitToOutside()==false) continue;

          //check if I can reach that exit, there should exits a direct line
          // segment connecting the two APs/goals
          const Point& p1 = ap->GetCentre();
          const Point& p2 = ped->GetPos();
          Line segment = Line(p1,p2);

          bool isVisible=true;

          for(const auto& wall: sub->GetAllWalls())
          {
               if(segment.IntersectionWith(wall))
               {
                    isVisible=false;
                    break;
               }
          }
          if(!isVisible) continue;

          double time=GetEstimatedTravelTimeVia(ped, exitid);

          if(time<minTime)
          {
               minTime=time;
               quickest=exitid;
          }

          //printf(" ped [%d] checking [%d] ---> [%f]\n",ped->GetPedIndex(),exitid,time);
          //also save the time for the default destinations for later comparison
          if (exitid==preferredExit) {
               preferredExitTime=time;
          }
     }

     if(quickest==-1)
     {
          //Log->Write("WARNING:\t Cannot redirect the pedestrian [%d]", ped->GetID());
          return;
     }

     //compare it with my preferred/current (shortest nearest)
     if(quickest!=preferredExit)
     {
          double cba = CBA(gain(preferredExitTime),gain(minTime));
          if (cba>_cbaThreshold)
          {
               ped->SetExitIndex(quickest);
               ped->SetExitLine(_accessPoints[quickest]->GetNavLine());
               //ped->SetSpotlight(false);
          }
     }
     //exit(0);
}

int QuickestPathRouter::GetBestDefaultRandomExit(Pedestrian* ped)
{
     // prob parameters
     //double alpha=0.2000005;
     //double normFactor=0.0;
     //map <int, double> doorProb;


     // get the relevant opened exits
     std::vector <AccessPoint*> relevantAPs;
     GetRelevantRoutesTofinalDestination(ped,relevantAPs);

     if(relevantAPs.size()==1)
     {
          auto&& ap=relevantAPs[0];
          ped->SetExitIndex(ap->GetID());
          ped->SetExitLine(ap->GetNavLine());
          return ap->GetID();
     }

     int bestAPsID = -1;
     double minDistGlobal = FLT_MAX;
     double minDistLocal = FLT_MAX;

     // get the opened exits
     SubRoom* sub = _building->GetRoom(ped->GetRoomID())->GetSubRoom(
               ped->GetSubRoomID());


     for(unsigned int g=0; g<relevantAPs.size(); g++)
     {
          AccessPoint* ap=relevantAPs[g];


          if (!ap->isInRange(sub->GetUID()))
               continue;
          //check if that exit is open.
          if (ap->IsClosed())
               continue;

          //the line from the current position to the centre of the nav line.
          // at least the line in that direction minus EPS
          const Point& posA = ped->GetPos();
          const Point& posB = ap->GetNavLine()->GetCentre();
          const Point& posC = (posB - posA).Normalized()
                                        * ((posA - posB).Norm() - J_EPS) + posA;


          //check if visible
          if(_building->IsVisible(posA, posC, _subroomsAtElevation[sub->GetElevation(sub->GetCentroid())],true)==false)
          {
               ped->RerouteIn(10);
               continue;
          }

          double dist1 = ap->GetDistanceTo(ped->GetFinalDestination());
          double dist2 = ap->DistanceTo(posA._x, posA._y);
          double dist=dist1+dist2;

          //        doorProb[ap->GetID()]= exp(-alpha*dist);
          //        normFactor += doorProb[ap->GetID()];


          //          if (dist < minDistGlobal) {
          //               bestAPsID = ap->GetID();
          //               minDistGlobal = dist;
          //          }

          // normalize the probs
          //    double randomVar = _rdDistribution(_rdGenerator);
          //
          //    for (auto it = doorProb.begin(); it!=doorProb.end(); ++it){
          //        it->second =  it->second / normFactor;
          //    }
          //
          //    double cumProb= doorProb.begin()->second;
          //    auto it = doorProb.begin();
          //    while(cumProb<randomVar) {
          //        it++;
          //        cumProb+=it->second;
          //    }
          //    bestAPsID=it->first;

          //very useful for short term decisions
          // if two doors are feasible to the final destination without much differences
          // in the distances, then the nearest is preferred.
          //cout<<"CBA (---): "<<  (dist-minDistGlobal) / (dist+minDistGlobal)<<endl;

          if(_defaultStrategy==GLOBAL_SHORTEST)
          {
               if (dist < minDistGlobal)
               {
                    bestAPsID = ap->GetID();
                    minDistGlobal = dist;
                    minDistLocal=dist2;
               }

          }
          else if(_defaultStrategy==LOCAL_SHORTEST)
          {
               if (     (dist > minDistGlobal)                   //is longer but ...
                     && (( (dist-minDistGlobal) / (dist+minDistGlobal) ) < _cbaThreshold)) //not more than 2 * _cbaThreshold
               {
                    if (dist2 < minDistLocal)
                    {
                         bestAPsID = ap->GetID();
                         minDistGlobal = dist;
                         minDistLocal= dist2;
                    }

               }
               else
               {
                    if (dist < minDistGlobal)
                    {
                         bestAPsID = ap->GetID();
                         minDistGlobal = dist;
                         minDistLocal=dist2;
                    }
               }
          }
          else
          {
               std::cout<<"Unknown Strategy: "<<_defaultStrategy<<std::endl;
               exit(0);
          }

     }

     if (bestAPsID != -1)
     {
          ped->SetExitIndex(bestAPsID);
          ped->SetExitLine(_accessPoints[bestAPsID]->GetNavLine());
          return bestAPsID;
     }
     else
     {
          if (_building->GetRoom(ped->GetRoomID())->GetCaption() != "outside")
               Log->Write(
                         "ERROR:\t Cannot find valid destination for ped [%d] located in room [%d] subroom [%d] going to destination [%d]",
                         ped->GetID(), ped->GetRoomID(), ped->GetSubRoomID(),
                         ped->GetFinalDestination());

          return -1;
     }
}

bool QuickestPathRouter::ParseAdditionalParameters()
{
     TiXmlDocument doc(_building->GetProjectFilename().string());
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \t GlobalRouter: could not parse the project file");
          return false;
     }

     // everything is fine. proceed with parsing
     TiXmlElement* xMainNode = doc.RootElement();
     TiXmlNode* xRouters=xMainNode->FirstChild("route_choice_models");

     for(TiXmlElement* e = xRouters->FirstChildElement("router"); e;
               e = e->NextSiblingElement("router"))
     {

          std::string strategy=e->Attribute("description");

          if( ( strategy=="quickest") && e->FirstChild("parameters"))
          {

               TiXmlElement* para =e->FirstChildElement("parameters");

               if (para)
               {
                    _cbaThreshold=xmltof(para->Attribute("cba_gain"), _cbaThreshold);
                    _congestionRation=xmltof(para->Attribute("congestion_ratio"), _congestionRation);
                    _queueVelocityFromJam=xmltof(para->Attribute("queue_vel_escaping_jam"), _queueVelocityFromJam);
                    _queueVelocityNewRoom=xmltof(para->Attribute("queue_vel_new_room"), _queueVelocityNewRoom);
                    _visibilityObstruction=(unsigned int)xmltoi(para->Attribute("visibility_obstruction"), _visibilityObstruction);

                    std::string selection_mode=xmltoa(para->Attribute("reference_peds_selection"), "single");
                    if(selection_mode=="single") _refPedSelectionMode=RefSelectionMode::SINGLE;
                    if(selection_mode=="all") _refPedSelectionMode=RefSelectionMode::ALL;

                    std::string default_strategy=xmltoa(para->Attribute("default_strategy"), "local_shortest");
                    if(default_strategy=="local_shortest") _defaultStrategy=DefaultStrategy::LOCAL_SHORTEST;
                    if(default_strategy=="global_shortest") _defaultStrategy=DefaultStrategy::GLOBAL_SHORTEST;

               }
          }
     }
     return true;
}
