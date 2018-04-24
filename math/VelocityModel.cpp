/**
 * \file        VelocityModel.cpp
 * \date        Aug. 07, 2015
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
 * Implementation of first-order model
 * 3. Velocity Model: Tordeux2015
 *
 *
 **/

# define NOMINMAX
#include "../pedestrian/Pedestrian.h"
//#include "../routing/DirectionStrategy.h"
#include "../mpi/LCGrid.h"
#include "../geometry/Wall.h"
#include "../geometry/SubRoom.h"


#include "VelocityModel.h"
#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

double xRight = 26.0;
double xLeft = 0.0;
double cutoff = 2.0;

using std::vector;
using std::string;

VelocityModel::VelocityModel(std::shared_ptr<DirectionStrategy> dir, double aped, double Dped,
                             double awall, double Dwall)
{
     _direction = dir;
     // Force_rep_PED Parameter
     _aPed = aped;
     _DPed = Dped;
     // Force_rep_WALL Parameter
     _aWall = awall;
     _DWall = Dwall;
}


VelocityModel::~VelocityModel()
{

}

bool VelocityModel::Init (Building* building)
{

    if(auto dirff = dynamic_cast<DirectionFloorfield*>(_direction.get())){
        Log->Write("INFO:\t Init DirectionFloorfield starting ...");
        double _deltaH = building->GetConfig()->get_deltaH();
        double _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
        bool _useWallAvoidance = building->GetConfig()->get_use_wall_avoidance();
        dirff->Init(building, _deltaH, _wallAvoidDistance, _useWallAvoidance);
        Log->Write("INFO:\t Init DirectionFloorfield done");
    }

     if(auto dirlocff = dynamic_cast<DirectionLocalFloorfield*>(_direction.get())){
          Log->Write("INFO:\t Init DirectionLOCALFloorfield starting ...");
          double _deltaH = building->GetConfig()->get_deltaH();
          double _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
          bool _useWallAvoidance = building->GetConfig()->get_use_wall_avoidance();
          dirlocff->Init(building, _deltaH, _wallAvoidDistance, _useWallAvoidance);
          Log->Write("INFO:\t Init DirectionLOCALFloorfield done");
     }

     if(auto dirsublocff = dynamic_cast<DirectionSubLocalFloorfield*>(_direction.get())){
          Log->Write("INFO:\t Init DirectionSubLOCALFloorfield starting ...");
          double _deltaH = building->GetConfig()->get_deltaH();
          double _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
          bool _useWallAvoidance = building->GetConfig()->get_use_wall_avoidance();
          dirsublocff->Init(building, _deltaH, _wallAvoidDistance, _useWallAvoidance);
          Log->Write("INFO:\t Init DirectionSubLOCALFloorfield done");
     }

    const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     size_t peds_size = allPeds.size();
    for(unsigned int p=0;p < peds_size;p++)
    {
         Pedestrian* ped = allPeds[p];
         double cosPhi, sinPhi;
         //a destination could not be found for that pedestrian
         if (ped->FindRoute() == -1) {
              Log->Write(
                   "ERROR:\tVelocityModel::Init() cannot initialise route. ped %d is deleted in Room %d %d.\n",ped->GetID(), ped->GetRoomID(), ped->GetSubRoomID());
              building->DeletePedestrian(ped);
              p--;
              peds_size--;
              continue;
         }




         Point target = ped->GetExitLine()->ShortestPoint(ped->GetPos());
         Point d = target - ped->GetPos();
         double dist = d.Norm();
         if (dist != 0.0) {
              cosPhi = d._x / dist;
              sinPhi = d._y / dist;
         } else {
              Log->Write(
                   "ERROR: \tallPeds::Init() cannot initialise phi! "
                   "dist to target is 0\n");
              return false;
         }

         ped->InitV0(target);

         JEllipse E = ped->GetEllipse();
         E.SetCosPhi(cosPhi);
         E.SetSinPhi(sinPhi);
         ped->SetEllipse(E);
    }
    return true;
}

void VelocityModel::ComputeNextTimeStep(double current, double deltaT, Building* building, int periodic)
{
      // collect all pedestrians in the simulation.
      const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
      vector<Pedestrian*> pedsToRemove;
      pedsToRemove.reserve(500);
      unsigned long nSize;
      nSize = allPeds.size();

      int nThreads = omp_get_max_threads();

      //nThreads = 1; //debug only
      int partSize;
      partSize = ((int)nSize > nThreads)? (int) (nSize / nThreads):(int)nSize;
      if(partSize == (int)nSize)
            nThreads = 1; // not worthy to parallelize
      #pragma omp parallel default(shared) num_threads(nThreads)
      {
           vector< Point > result_acc = vector<Point > ();
           result_acc.reserve(nSize);
           vector< my_pair > spacings = vector<my_pair > ();
           spacings.reserve(nSize); // larger than needed
           spacings.push_back(my_pair(100, 1)); // in case there are no neighbors
           const int threadID = omp_get_thread_num();

           int start = threadID*partSize;
           int end;
           end = (threadID < nThreads - 1) ? (threadID + 1) * partSize - 1: (int) (nSize - 1);
           for (int p = start; p <= end; ++p) {
                 // printf("\n------------------\nid=%d\t p=%d\n", threadID, p);
                Pedestrian* ped = allPeds[p];
                Room* room = building->GetRoom(ped->GetRoomID());
                SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
                Point repPed = Point(0,0);
                vector<Pedestrian*> neighbours;
                building->GetGrid()->GetNeighbourhood(ped,neighbours);

                int size = (int) neighbours.size();
                for (int i = 0; i < size; i++) {
                     Pedestrian* ped1 = neighbours[i];
                     //if they are in the same subroom
                     Point p1 = ped->GetPos();
                     Point p2 = ped1->GetPos();
                     //subrooms to consider when looking for neighbour for the 3d visibility
                     vector<SubRoom*> emptyVector;
                     emptyVector.push_back(subroom);
                     emptyVector.push_back(building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID()));
                     bool isVisible = building->IsVisible(p1, p2, emptyVector, false);
                     if (!isVisible)
                          continue;
                     if (ped->GetUniqueRoomID() == ped1->GetUniqueRoomID()) {
                           repPed += ForceRepPed(ped, ped1, periodic);
                     } else {
                          // or in neighbour subrooms
                          SubRoom* sb2=building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID());
                          if(subroom->IsDirectlyConnectedWith(sb2)) {
                                repPed += ForceRepPed(ped, ped1, periodic);
                          }
                     }
                } // for i
                //repulsive forces to walls and closed transitions that are not my target
                Point repWall = ForceRepRoom(allPeds[p], subroom);

                // calculate new direction ei according to (6)
                Point direction = e0(ped, room) + repPed + repWall;
                for (int i = 0; i < size; i++) {
                      Pedestrian* ped1 = neighbours[i];
                     // calculate spacing
                     // my_pair spacing_winkel = GetSpacing(ped, ped1);
                      if (ped->GetUniqueRoomID() == ped1->GetUniqueRoomID()) {
                            spacings.push_back(GetSpacing(ped, ped1, direction, periodic));
                      } else {
                            // or in neighbour subrooms
                            SubRoom* sb2=building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID());
                            if(subroom->IsDirectlyConnectedWith(sb2)) {
                                  spacings.push_back(GetSpacing(ped, ped1, direction, periodic));
                            }
                      }
                }
                // @todo: get spacing to walls
                // @todo: update direction every DT?

                // if(ped->GetID()==-10)
                //       std::cout << "time: " << ped->GetGlobalTime() << "  |  updateRate  " <<ped->GetUpdateRate() << "   modulo " <<fmod(ped->GetGlobalTime(), ped->GetUpdateRate())<<std::endl;

                // calculate min spacing
                std::sort(spacings.begin(), spacings.end(), sort_pred());
                double spacing = spacings[0].first;
                //============================================================
                // TODO: Hack for Head on situations: ped1 x ------> | <------- x ped2
                if(0 && direction.NormSquare() < 0.5)
                {
                      double pi_half = 1.57079663;
                      double alpha =  pi_half*exp(-spacing);
                      direction = e0(ped, room).Rotate(cos(alpha), sin(alpha));
                      printf("\nRotate %f, %f, norm = %f alpha = %f, spacing = %f\n", direction._x, direction._y, direction.NormSquare(), alpha, spacing);
                      getc(stdin);
                }
                //============================================================
                //double winkel = spacings[0].second;
                //Point tmp;
                Point speed = direction.Normalized() *OptimalSpeed(ped, spacing);
                result_acc.push_back(speed);

                spacings.clear(); //clear for ped p

                // stuck peds get removed. Warning is thrown. low speed due to jam is omitted.
                if(ped->GetGlobalTime() > 30 + ped->GetPremovementTime() &&
                          std::max(ped->GetMeanVelOverRecTime(), ped->GetV().Norm()) < 0.01 &&
                          size == 0 ) // size length of peds neighbour vector
                {
                      Log->Write("WARNING:\tped %d with vmean  %f has been deleted in room [%i]/[%i] after time %f s (current=%f\n", ped->GetID(), ped->GetMeanVelOverRecTime(), ped->GetRoomID(), ped->GetSubRoomID(), ped->GetGlobalTime(), current);
                      #pragma omp critical(VelocityModel_ComputeNextTimeStep_pedsToRemove)
                      pedsToRemove.push_back(ped);
                }

           } // for p

           #pragma omp barrier
           // update
           for (int p = start; p <= end; ++p) {
                Pedestrian* ped = allPeds[p];

                Point v_neu = result_acc[p - start];
                Point pos_neu = ped->GetPos() + v_neu * deltaT;

               //Jam is based on the current velocity
                if ( v_neu.Norm() >= ped->GetV0Norm()*0.5) {
                     ped->ResetTimeInJam();
                } else {
                     ped->UpdateTimeInJam();
                }
                //only update the position if the velocity is above a threshold
                if (v_neu.Norm() >= J_EPS_V)
                {
                     ped->SetPhiPed();
                }
                ped->SetPos(pos_neu);
                if(periodic){
                      if(ped->GetPos()._x >= xRight){
                            ped->SetPos(Point(ped->GetPos()._x - (xRight - xLeft), ped->GetPos()._y));
                            //ped->SetID( ped->GetID() + 1);
                      }
                }
                ped->SetV(v_neu);
           }
      }//end parallel

      // remove the pedestrians that have left the building
      for (unsigned int p = 0; p<pedsToRemove.size(); p++) {
            building->DeletePedestrian(pedsToRemove[p]);
      }
      pedsToRemove.clear();
}

Point VelocityModel::e0(Pedestrian* ped, Room* room) const
{
      const Point target = _direction->GetTarget(room, ped); // target is where the ped wants to be after the next timestep
      Point desired_direction;
      const Point pos = ped->GetPos();
      double dist = ped->GetExitLine()->DistTo(pos);
      // check if the molified version works
      Point lastE0 = ped->GetLastE0();
      ped->SetLastE0(target-pos);

      if ( (dynamic_cast<DirectionFloorfield*>(_direction.get())) ||
           (dynamic_cast<DirectionLocalFloorfield*>(_direction.get())) ||
           (dynamic_cast<DirectionSubLocalFloorfield*>(_direction.get()))  ) {
          if (dist > 50*J_EPS_GOAL) {
               desired_direction = target - pos; //ped->GetV0(target);
          } else {
               desired_direction = lastE0;
               ped->SetLastE0(lastE0); //keep old vector (revert set operation done 9 lines above)
          }
      }
      else if (dist > J_EPS_GOAL) {
            desired_direction = ped->GetV0(target);
      } else {
          ped->SetSmoothTurning();
          desired_direction = ped->GetV0();
     }
     return desired_direction;
}


double VelocityModel::OptimalSpeed(Pedestrian* ped, double spacing) const
{
      double v0 = ped->GetV0Norm();
      double T = ped->GetT();
      double l = 2*ped->GetEllipse().GetBmax(); //assume peds are circles with const radius
      double speed = (spacing-l)/T;
      speed = (speed>0)?speed:0;
      speed = (speed<v0)?speed:v0;
//      (1-winkel)*speed;
     //todo use winkel
      return speed;
}

// return spacing and id of the nearest pedestrian
my_pair VelocityModel::GetSpacing(Pedestrian* ped1, Pedestrian* ped2, Point ei, int periodic) const
{
      Point distp12 = ped2->GetPos() - ped1->GetPos(); // inversed sign
      if(periodic){
            double x = ped1->GetPos()._x;
            double x_j = ped2->GetPos()._x;

            if((xRight-x) + (x_j-xLeft) <= cutoff){
                 distp12._x = distp12._x + xRight - xLeft;
            }
      }
      double Distance = distp12.Norm();
      double l = 2*ped1->GetEllipse().GetBmax();
      Point ep12;
      if (Distance >= J_EPS) {
            ep12 = distp12.Normalized();
      } else {
            //printf("ERROR: \tin VelocityModel::forcePedPed() ep12 can not be calculated!!!\n");
            Log->Write("WARNING: \tin VelocityModel::GetSPacing() ep12 can not be calculated!!!\n");
            Log->Write("\t\t Pedestrians are too near to each other (%f).", Distance);
            my_pair(FLT_MAX, ped2->GetID());
            //exit(EXIT_FAILURE); TODO
     }

      double condition1 = ei.ScalarProduct(ep12); // < e_i , e_ij > should be positive
      double condition2 = ei.Rotate(0, 1).ScalarProduct(ep12); // theta = pi/2. condition2 should <= than l/Distance
      condition2 = (condition2>0)?condition2:-condition2; // abs

      if((condition1 >=0 ) && (condition2 <= l/Distance))
            // return a pair <dist, condition1>. Then take the smallest dist. In case of equality the biggest condition1
            return  my_pair(distp12.Norm(), ped2->GetID());
      else
            return  my_pair(FLT_MAX, ped2->GetID());
}
Point VelocityModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2, int periodic) const
{
     Point F_rep(0.0, 0.0);
     // x- and y-coordinate of the distance between p1 and p2
     Point distp12 = ped2->GetPos() - ped1->GetPos();

     if(periodic){
            double x = ped1->GetPos()._x;
            double x_j = ped2->GetPos()._x;
            if((xRight-x) + (x_j-xLeft) <= cutoff){
                 distp12._x = distp12._x + xRight - xLeft;
            }
      }

     double Distance = distp12.Norm();
     Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
     double R_ij;
     double l = 2*ped1->GetEllipse().GetBmax();

     if (Distance >= J_EPS) {
          ep12 = distp12.Normalized();
     } else {
          //printf("ERROR: \tin VelocityModel::forcePedPed() ep12 can not be calculated!!!\n");
          Log->Write(KRED "\nWARNING: \tin VelocityModel::forcePedPed() ep12 can not be calculated!!!" RESET);
          Log->Write("\t\t Pedestrians are too near to each other (dist=%f).", Distance);
          Log->Write("\t\t Maybe the value of <a> in force_ped should be increased. Going to exit.\n");
          printf("ped1 %d  ped2 %d\n", ped1->GetID(), ped2->GetID());
          printf("ped1 at (%f, %f), ped2 at (%f, %f)\n", ped1->GetPos()._x, ped1->GetPos()._y, ped2->GetPos()._x, ped2->GetPos()._y);
          // exit(EXIT_FAILURE); //TODO: quick and dirty fix for issue #158
          // (sometimes sources create peds on the same location)
          return F_rep; //no forces, in the hope these two get away soon
     }
      Point ei = ped1->GetV().Normalized();
      if(ped1->GetV().NormSquare()<0.01){
            ei = ped1->GetV0().Normalized();
      }
      double condition1 = ei.ScalarProduct(ep12); // < e_i , e_ij > should be positive
      condition1 = (condition1>0)?condition1:0; // abs

      R_ij = - _aPed * exp((l-Distance)/_DPed);
      F_rep = ep12 * R_ij;

     return F_rep;
}//END Velocity:ForceRepPed()

Point VelocityModel::ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const
{
     Point f(0., 0.);
     const Point& centroid = subroom->GetCentroid();
     bool inside = subroom->IsInSubRoom(centroid);
     //first the walls
     for(const auto & wall: subroom->GetAllWalls())
     {
           f += ForceRepWall(ped, wall, centroid, inside);
     }

     //then the obstacles
     for(const auto & obst: subroom->GetAllObstacles())
     {
          if(obst->Contains(ped->GetPos()))
          {
               Log->Write("ERROR:\t Agent [%d] is trapped in obstacle in room/subroom [%d/%d]",ped->GetID(),subroom->GetRoomID(), subroom->GetSubRoomID());
               exit(EXIT_FAILURE);
          }
          else
          for(const auto & wall: obst->GetAllWalls())
          {
                f += ForceRepWall(ped, wall, centroid, inside);
          }
     }

     // and finally the closed doors
     for(const auto & goal: subroom->GetAllTransitions())
     {
          if(! goal->IsOpen())
          {
                f +=  ForceRepWall(ped,*(static_cast<Line*>(goal)), centroid, inside);
          }
          // int uid1= goal->GetUniqueID();
          // int uid2=ped->GetExitIndex();
          // // ignore my transition consider closed doors
          // //closed doors are considered as wall

          // if((uid1 != uid2) || (goal->IsOpen()==false ))
          // {
          //      f +=  ForceRepWall(ped,*(static_cast<Line*>(goal)), centroid, inside);
          // }
     }

     return f;
}

Point VelocityModel::ForceRepWall(Pedestrian* ped, const Line& w, const Point& centroid, bool inside) const
{
     Point F_wrep = Point(0.0, 0.0);
     Point pt = w.ShortestPoint(ped->GetPos());

     Point dist = pt - ped->GetPos(); // x- and y-coordinate of the distance between ped and p
     const double EPS = 0.000; // molified see Koester2013
     double Distance = dist.Norm() + EPS; // distance between the centre of ped and point p
     //double vn = w.NormalComp(ped->GetV()); //normal component of the velocity on the wall
     Point e_iw; // x- and y-coordinate of the normalized vector between ped and pt
     //double K_iw;
     double l = ped->GetEllipse().GetBmax();
     double R_iw;
     double min_distance_to_wall = 0.001; // 10 cm

     if (Distance > min_distance_to_wall) {
           e_iw = dist / Distance;
     }
     else {
          Log->Write("WARNING:\t Velocity: forceRepWall() ped %d [%f, %f] is too near to the wall [%f, %f]-[%f, %f] (dist=%f)", ped->GetID(), ped->GetPos()._y, ped->GetPos()._y, w.GetPoint1()._x, w.GetPoint1()._y,  w.GetPoint2()._x, w.GetPoint2()._y,Distance);
          Point new_dist = centroid - ped->GetPos();
          new_dist = new_dist/new_dist.Norm();
          //printf("new distance = (%f, %f) inside=%d\n", new_dist._x, new_dist._y, inside);
          e_iw = (inside ? new_dist:new_dist*-1);
     }
     //-------------------------

     const Point& pos = ped->GetPos();
     double distGoal = ped->GetExitLine()->DistToSquare(pos);
     if(distGoal < J_EPS_GOAL*J_EPS_GOAL)
          return F_wrep;
//-------------------------
     R_iw = - _aWall * exp((l-Distance)/_DWall);
     F_wrep = e_iw * R_iw;

     return F_wrep;
}

string VelocityModel::GetDescription()
{
     string rueck;
     char tmp[CLENGTH];

     sprintf(tmp, "\t\ta: \t\tPed: %f \tWall: %f\n", _aPed, _aWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\tD: \t\tPed: %f \tWall: %f\n", _DPed, _DWall);
     rueck.append(tmp);
     return rueck;
}

std::shared_ptr<DirectionStrategy> VelocityModel::GetDirection() const
{
     return _direction;
}


double VelocityModel::GetaPed() const
{
     return _aPed;
}

double VelocityModel::GetDPed() const
{
     return _DPed;
}


double VelocityModel::GetaWall() const
{
     return _aWall;
}

double VelocityModel::GetDWall() const
{
     return _DWall;
}
