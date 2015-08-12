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


#include "../pedestrian/Pedestrian.h"
#include "../routing/DirectionStrategy.h"
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

using std::vector;
using std::string;

VelocityModel::VelocityModel(DirectionStrategy* dir, double aped, double Dped,
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

bool VelocityModel::Init (Building* building) const
{
    const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

    for(unsigned int p=0;p<allPeds.size();p++)
    {
         Pedestrian* ped = allPeds[p];
         double cosPhi, sinPhi;
         //a destination could not be found for that pedestrian
         if (ped->FindRoute() == -1) {
              Log->Write(
                   "ERROR:\tVelocityModel::Init() cannot initialise route. ped %d is deleted.\n",ped->GetID());
             building->DeletePedestrian(ped);
              continue;
         }

         Line* e = ped->GetExitLine();
         const Point& e1 = e->GetPoint1();
         const Point& e2 = e->GetPoint2();
         Point target = (e1 + e2) * 0.5;
         Point d = target - ped->GetPos();
         double dist = d.Norm();
         if (dist != 0.0) {
              cosPhi = d.GetX() / dist;
              sinPhi = d.GetY() / dist;
         } else {
              Log->Write(
                   "ERROR: \allPeds::Init() cannot initialise phi! "
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

void VelocityModel::ComputeNextTimeStep(double current, double deltaT, Building* building) const
{
     double delta = 0.5;
      // collect all pedestrians in the simulation.
      const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

     unsigned long nSize;
     nSize = allPeds.size();

      int nThreads = omp_get_max_threads();

     int partSize;
     partSize = (int) (nSize / nThreads);

      #pragma omp parallel  default(shared) num_threads(nThreads)
      {
           vector< Point > result_acc = vector<Point > ();
           result_acc.reserve(nSize);
           vector< double > spacings = vector<double > ();
           spacings.reserve(nSize); // bigger than needed
           spacings.push_back(100); //in case there are no neighbors
           const int threadID = omp_get_thread_num();

           int start = threadID*partSize;
           int end;
           end = (threadID + 1) * partSize - 1;
           if ((threadID == nThreads - 1)) end = (int) (nSize - 1);

           for (int p = start; p <= end; ++p) {

                Pedestrian* ped = allPeds[p];
                Room* room = building->GetRoom(ped->GetRoomID());
                SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());

                double normVi = ped->GetV().ScalarProduct(ped->GetV()); //squared
                double HighVel = (ped->GetV0Norm() + delta) * (ped->GetV0Norm() + delta); //(v0+delta)^2
                if (normVi > HighVel && ped->GetV0Norm() > 0) {
                     fprintf(stderr, "VelocityModel::calculateForce_LC() WARNING: actual velocity (%f) of iped %d "
                             "is bigger than desired velocity (%f) at time: %fs\n",
                             sqrt(normVi), ped->GetID(), ped->GetV0Norm(), current);

                     // remove the pedestrian and abort
                     Log->Write("\tERROR: ped [%d] was removed due to high velocity",ped->GetID());
                     building->DeletePedestrian(ped);
                     //continue;  //FIXME tolerate first
                     exit(EXIT_FAILURE);
                }

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
                          repPed += ForceRepPed(ped, ped1);
                     } else {
                          // or in neighbour subrooms
                          SubRoom* sb2=building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID());
                          if(subroom->IsDirectlyConnectedWith(sb2)) {
                               repPed += ForceRepPed(ped, ped1);
                          }
                     }
                     // calculate spacing
                     double spacing = GetSpacing(ped, ped1);
                     spacings.push_back(spacing);
                }
                //repulsive forces to walls and closed transitions that are not my target
                Point repWall = ForceRepRoom(allPeds[p], subroom);
                
                Point direction = (e0(ped, room) + repPed + repWall);
                // calculate min spacing
                double spacing;
                spacing = *std::min_element(std::begin(spacings), std::end(spacings));

                // for (auto s: spacings)
                // {
                //       std::cout << "s: " << s << std::endl;
                // }
                
                Point acc = direction.Normalized() * OptimalSpeed(ped, spacing);
                // std::cout << direction.Norm() << std::endl;
                // std::cout << direction.GetX() << " | " << direction.GetY() << std::endl;
                if(direction.Norm()< 0.3 && ped->GetGlobalTime() >5)
                      acc = Point(0,0);
                result_acc.push_back(acc);
                // spacings.resize(0);
                // spacings.shrink_to_fit();
                spacings.clear();
           } // for p

           //#pragma omp barrier
           // update
           for (int p = start; p <= end; ++p) {
                Pedestrian* ped = allPeds[p];

                Point v_neu = result_acc[p - start];
                Point pos_neu = ped->GetPos() + v_neu * deltaT;
               
                // if( (v_neu.Norm() > 1.2*ped->GetV0Norm() )) { // Stop pedestrians if the velocity is too high
                //       //Log->Write("WARNING: \tped %d is stopped because v=%f (v0=%f)\n", ped->GetID(), v_neu.Norm(), ped->GetV0Norm());
                //      v_neu = v_neu*0.01;
                //      pos_neu = ped->GetPos();
                // }

               //Jam is based on the current velocity
                if ( v_neu.Norm() >= ped->GetV0Norm()*0.5) {
                     ped->ResetTimeInJam();
                } else {
                     ped->UpdateTimeInJam();
                }
                
                //only update the position if the velocity is above a threshold
                //if (v_neu.Norm() >= J_EPS_V*0.7)
                {
                     ped->SetPos(pos_neu);
                     ped->SetPhiPed();
                }

                ped->SetV(v_neu);

           }
      }//end parallel
}

Point VelocityModel::e0(Pedestrian* ped, Room* room) const
{
      const Point& target = _direction->GetTarget(room, ped);
      Point e0;
      const Point& pos = ped->GetPos();
      double dist = ped->GetExitLine()->DistTo(pos);
    
      // check if the molified version works
      if (dist > J_EPS_GOAL) {
            e0 = ped->GetV0(target);

      } else {
          ped->SetSmoothTurning();
          e0 = ped->GetV0();
     }


      // if (ped->GetID()==-4){
      //      double e0norm = sqrt(e0.GetX()*e0.GetX() +e0.GetY()*e0.GetY());
      //      printf( "pos %f %f target %f %f\n", pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
      //      printf("mass=%f, v0norm=%f, e0Norm=%f, tau=%f\n", ped->GetMass(), ped->GetV0Norm(), e0norm, ped->GetTau());
      //      printf("Fdriv=  [%f, %f]\n", F_driv.GetX(), F_driv.GetY());
      //      fprintf(stdout, "%d   %f    %f    %f    %f    %f    %f\n", ped->GetID(), ped->GetPos().GetX(), ped->GetPos().GetY(), ped->GetV().GetX(), ped->GetV().GetY(), target.GetX(), target.GetY());

      // }

     return e0;
}


double VelocityModel::OptimalSpeed(Pedestrian* ped, double spacing) const
{
      double v0 = ped->GetV0Norm();
      double T = ped->GetT();
      double l = ped->GetEllipse().GetBmax(); //assume peds are circles with const radius
      double speed = (spacing-l)/T;
      speed = (speed>0)?speed:0;
      speed = (speed<v0)?speed:v0;

      return speed;
}

double VelocityModel::GetSpacing(Pedestrian* ped1, Pedestrian* ped2) const
{
      // printf("GetSpacing with %d and %d\n", ped1->GetID(), ped2->GetID());
      Point distp12 = ped2->GetPos() - ped1->GetPos(); // inversed sign 
      double Distance = distp12.Norm();
      double l = ped1->GetEllipse().GetBmax();
      Point ep12;
      if (Distance >= J_EPS) {
            ep12 = distp12.Normalized();
      } else {
            //printf("ERROR: \tin VelocityModel::forcePedPed() ep12 can not be calculated!!!\n");
            Log->Write("WARNING: \tin VelocityModel::forcePedPed() ep12 can not be calculated!!!\n");
            Log->Write("\t\t Pedestrians are too near to each other.");
            Log->Write("\t\t Get your model right. Going to exit.");
            exit(EXIT_FAILURE);
     }
      Point ei = ped1->GetV().Normalized();
      double condition1 = ei.ScalarProduct(ep12); // < v_i , e_ij > should be positive
      double condition2 = ei.Rotate(0, 1).ScalarProduct(ep12); // theta = pi/2. condition2 should <= than l/Distance
      condition2 = (condition2>0)?condition2:-condition2; // abs
      // printf("condition1= %f, condition2=%f l/D=%f, norm=%f\n", condition1, condition2, l/Distance, distp12.Norm());
      
      if((condition1 >=0 ) && (condition2 <= l/Distance))
            return distp12.Norm();
      else
            return std::numeric_limits<double>::max();
}      
Point VelocityModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const
{
     Point F_rep(0.0, 0.0);
     // x- and y-coordinate of the distance between p1 and p2
     Point distp12 = ped2->GetPos() - ped1->GetPos();
     double Distance = distp12.Norm();
     Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
     double R_ij;
     const double EPS = 0.001;
     double l = ped1->GetEllipse().GetBmax();

     if (Distance >= J_EPS) {
          ep12 = distp12.Normalized();
     } else {
          //printf("ERROR: \tin VelocityModel::forcePedPed() ep12 can not be calculated!!!\n");
          Log->Write("WARNING: \tin VelocityModel::forcePedPed() ep12 can not be calculated!!!\n");
          Log->Write("\t\t Pedestrians are too near to each other.");
          Log->Write("\t\t Get your model right. Going to exit.");
          exit(EXIT_FAILURE);
     }

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
          //  int uid1= goal->GetUniqueID();
          //  int uid2=ped->GetExitIndex();
          //  // ignore my transition consider closed doors
          //  //closed doors are considered as wall
          //
          //  if((uid1 != uid2) || (goal->IsOpen()==false ))
          //  {
          //    f +=  ForceRepWall(ped,*(static_cast<Line*>(goal)));
          //  }
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
     double min_distance_to_wall = 0.1; // 10 cm
     
     if (Distance > min_distance_to_wall) {
           e_iw = dist / Distance;
     }
     else {
          // Log->Write("WARNING:\t Velocity: forceRepWall() ped %d is too near to the wall",ped->GetID());
          Point new_dist = centroid - ped->GetPos();
          new_dist = new_dist/new_dist.Norm();
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

string VelocityModel::GetDescription() const
{
     string rueck;
     char tmp[CLENGTH];
     
     sprintf(tmp, "\t\ta: \t\tPed: %f \tWall: %f\n", _aPed, _aWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\tD: \t\tPed: %f \tWall: %f\n", _DPed, _DWall);
     rueck.append(tmp);
     return rueck;
}

DirectionStrategy* VelocityModel::GetDirection() const
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
