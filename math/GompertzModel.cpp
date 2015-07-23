/**
 * \file        GompertzModel.cpp
 * \date        Apr 15, 2014
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
 * Implementation of classes for force-based models.
 * Actually we've got two different models:
 * 2. Gompertz Model
 *
 *
 **/


#include "../pedestrian/Pedestrian.h"
#include "../routing/DirectionStrategy.h"
#include "../mpi/LCGrid.h"
#include "../geometry/Wall.h"
#include "../geometry/SubRoom.h"

#include "GompertzModel.h"

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

using std::vector;
using std::string;

GompertzModel::GompertzModel(DirectionStrategy* dir, double nuped, double aped, double bped, double cped,
                             double nuwall, double awall, double bwall, double cwall)
{
     _direction = dir;
     // Force_rep_PED Parameter
     _nuPed = nuped;
     _aPed = aped;
     _bPed = bped;
     _cPed = cped;
     // Force_rep_WALL Parameter
     _nuWall = nuwall;
     _aWall = awall;
     _bWall = bwall;
     _cWall = cwall;
}


GompertzModel::~GompertzModel()
{

}

bool GompertzModel::Init (Building* building) const
{
    const vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

    for(unsigned int p=0;p<allPeds.size();p++)
    {
         Pedestrian* ped = allPeds[p];
         double cosPhi, sinPhi;
         //a destination could not be found for that pedestrian
         if (ped->FindRoute() == -1) {
              Log->Write(
                   "ERROR:\tGompertzModel::Init() cannot initialise route. ped %d is deleted.\n",ped->GetID());
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

void GompertzModel::ComputeNextTimeStep(double current, double deltaT, Building* building) const
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
                     fprintf(stderr, "GompertzModel::calculateForce_LC() WARNING: actual velocity (%f) of iped %d "
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
                          repPed = repPed + ForceRepPed(ped, ped1);
                     } else {
                          // or in neighbour subrooms
                          SubRoom* sb2=building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID());
                          if(subroom->IsDirectlyConnectedWith(sb2)) {
                               repPed = repPed + ForceRepPed(ped, ped1);
                          }
                     }
                }
                //repulsive forces to walls and closed transitions that are not my target
                Point repWall = ForceRepRoom(allPeds[p], subroom);
                Point fd = ForceDriv(ped, room);

                Point acc = (fd + repPed + repWall) / ped->GetMass();
                if(ped->GetID()==-242)
                {
                     printf("t=%f, Pos1 =[%f, %f]\n", current,ped->GetPos().GetX(), ped->GetPos().GetY());
                     printf("acc= %f %f, fd= %f, %f,  repPed = %f %f, repWall= %f, %f\n", acc.GetX(), acc.GetY(), fd.GetX(), fd.GetY(), repPed.GetX(), repPed.GetY(), repWall.GetX(), repWall.GetY());
                     // if(current >16) getc(stdin);
                }

                result_acc.push_back(acc);
           }

           //#pragma omp barrier
           // update
           for (int p = start; p <= end; ++p) {
                Pedestrian* ped = allPeds[p];

                Point v_neu = ped->GetV() + result_acc[p - start] * deltaT;
                Point pos_neu = ped->GetPos() + v_neu * deltaT;
               
                if( (v_neu.Norm() > 1.2*ped->GetV0Norm() )) { // Stop pedestrians if the velocity is too high
                      //                     Log->Write("WARNING: \tped %d is stopped because v=%f (v0=%f)\n", ped->GetID(), v_neu.Norm(), ped->GetV0Norm());
                     v_neu = v_neu*0.01;
                     pos_neu = ped->GetPos();
                }

               //Jam is based on the current velocity
                if ( v_neu.Norm() >= ped->GetV0Norm()*0.5) {
                     ped->ResetTimeInJam();
                } else {
                     ped->UpdateTimeInJam();
                }
                
                //only update the position if the velocity is above a threshold
                if (1 || v_neu.Norm() >= J_EPS_V*0.7)
                {
                     ped->SetPos(pos_neu);
                     ped->SetPhiPed();
                }

                ped->SetV(v_neu);

           }
      }//end parallel
}

Point GompertzModel::ForceDriv(Pedestrian* ped, Room* room) const
{
#define DEBUG 0

# if DEBUG
     printf("=====\n Enter GompertzModel::ForceDriv\n");
# endif
     const Point& target = _direction->GetTarget(room, ped);
     Point F_driv;
     Point e0;
     const Point& pos = ped->GetPos();
     double dist = ped->GetExitLine()->DistTo(pos);
    
     // check if the molified version works
     if (dist > J_EPS_GOAL) {
          e0 = ped->GetV0(target);
          if(ped->GetID()==-4)
          printf("1 e0 %f %f, target %f %f\n", e0.GetX(), e0.GetY(), target.GetX(), target.GetY());
     } else {
          ped->SetSmoothTurning();
          e0 = ped->GetV0();
                    if(ped->GetID()==-4)
            printf("2 e0 %f %f\n", e0.GetX(), e0.GetY());
     }
      F_driv = ((e0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
  
      //double v =  sqrt(ped->GetV().GetX()*ped->GetV().GetX() +ped->GetV().GetY()*ped->GetV().GetY());

// #if DEBUG
      if (ped->GetID()==-4){
           double e0norm = sqrt(e0.GetX()*e0.GetX() +e0.GetY()*e0.GetY());
           printf( "pos %f %f target %f %f\n", pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
           printf("mass=%f, v0norm=%f, e0Norm=%f, tau=%f\n", ped->GetMass(), ped->GetV0Norm(), e0norm, ped->GetTau());
           printf("Fdriv=  [%f, %f]\n", F_driv.GetX(), F_driv.GetY());
           fprintf(stdout, "%d   %f    %f    %f    %f    %f    %f\n", ped->GetID(), ped->GetPos().GetX(), ped->GetPos().GetY(), ped->GetV().GetX(), ped->GetV().GetY(), target.GetX(), target.GetY());

      }
// #endif

     return F_driv;
}

Point GompertzModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const
{
     Point F_rep(0.0, 0.0);
     // x- and y-coordinate of the distance between p1 and p2
     Point distp12 = ped2->GetPos() - ped1->GetPos();
     //const Point& vp1 = ped1->GetV(); // v Ped1
     //const Point& vp2 = ped2->GetV(); // v Ped2
     Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
     //double K_ij;
     double B_ij, f;
     const JEllipse& E1 = ped1->GetEllipse();
     const JEllipse& E2 = ped2->GetEllipse();
     Point AP1inE1 = E1.GetCenter();
     Point AP2inE2 = E2.GetCenter();
     // ActionPoint von E1 in Koordinaten von E2 (transformieren)
     Point AP1inE2 = AP1inE1.TransformToEllipseCoordinates(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
     // ActionPoint von E2 in Koordinaten von E1 (transformieren)
     Point AP2inE1 = AP2inE2.TransformToEllipseCoordinates(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
     double r1 = (AP1inE1 - E1.PointOnEllipse(AP2inE1)).Norm();
     double r2 = (AP2inE2 - E2.PointOnEllipse(AP1inE2)).Norm();
     //fprintf(stderr, "%f %f %f %f\n",  E1.GetEA(), E1.GetEB(), E2.GetEA(), E2.GetEB());
     //fprintf(stderr, "%f %f\n",  r1, r2);
     const double EPS = 0.001;
     double Distance = distp12.Norm() + EPS; // Molified See Koester2013

     // if(ped1->GetID() ==logped)
     // {
     //     printf("ped1=%d ped2=%d  Distance=%f\n",ped1->GetID(), ped2->GetID(), Distance);
     // }
     if (Distance >= J_EPS) {
          ep12 = distp12.Normalized();
     } else {
          //printf("ERROR: \tin GompertzModel::forcePedPed() ep12 can not be calculated!!!\n");
          Log->Write("WARNING: \tin GompertzModel::forcePedPed() ep12 can not be calculated!!!\n");
          Log->Write("\t\t Pedestrians are too near to each other.");
          Log->Write("\t\t Get your model right. Going to exit.");
          return F_rep; // should never happen
          exit(EXIT_FAILURE);
     }
//------------------------- check if others are behind using v0 instead of v
     double tmpv = ped1->GetV().ScalarProduct(ep12); // < v^0_i , e_ij >
     double ped2IsBehindv =  (tmpv<=0)?0:1; //exp(-exp(-5*tmpv)); //step function: continuous version
     if (ped2IsBehindv < J_EPS) {
          return F_rep; // ignore ped2
     }
//------------------------------------------------------------------------------

     // calculate B_ij
     B_ij = 1.0 - Distance/(r1+r2); //TODO: Simplification to avoid accelerating predecessors
     //Gompertz-function parameter.
     //TODO: Check later if other values are more appropriate
     double b = _bPed, c = _cPed; //repped
     B_ij = exp(-b*exp(-c*B_ij));
     //TODO: check if we need K_ij in the  f
     //f = -ped1->GetMass() * _nuPed * ped1->GetV0Norm() * K_ij * B_ij;
    
     f = -ped1->GetMass() * _nuPed * ped1->GetV0Norm() * B_ij;

     F_rep = ep12 * f;
     if(ped1->GetID() ==-4) {
          printf("\nNAN return ----> p1=%d p2=%d pos1=%f %f, pos2=%f %f\n", ped1->GetID(),
                 ped2->GetID(), ped1->GetPos().GetX(), ped1->GetPos().GetY(),  ped2->GetPos().GetX(), ped2->GetPos().GetY());
                 
     
          printf("F=[%f, %f] v0=%f, nu=%f, B_ij=%f D=%f, r1=%f, r2=%f\n", F_rep.GetX(), F_rep.GetY(), ped1->GetV0Norm(), _nuPed, B_ij, Distance, r1, r2);
     }
//check isNan
     if (F_rep.GetX() != F_rep.GetX() || F_rep.GetY() != F_rep.GetY()) {
          char tmp[CLENGTH];
          sprintf(tmp, "\nNAN return ----> p1=%d p2=%d Frepx=%f, Frepy=%f\n", ped1->GetID(),
                  ped2->GetID(), F_rep.GetX(), F_rep.GetY());
          Log->Write(tmp);
          Log->Write("ERROR:\t fix this as soon as possible");
          return Point(0,0); // FIXME: should never happen
          exit(EXIT_FAILURE);
     }
     return F_rep;
}//END Gompertz:ForceRepPed()

Point GompertzModel::ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const
{
     Point f(0., 0.);
     Point centroid = subroom->GetCentroid();
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
     for(auto & goal: subroom->GetAllTransitions())
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

Point GompertzModel::ForceRepWall(Pedestrian* ped, const Line& w, const Point& centroid, bool inside) const
{
#define DEBUG 0
     Point F_wrep = Point(0.0, 0.0);
#if DEBUG
     if(ped->GetID()==-4)
     printf("=========\n\tEnter GompertzWall with PED=%d, wall=[%.2f, %.2f]--[%.2f, %.2f]\n", ped->GetID(), w.GetPoint1().GetX(),  w.GetPoint1().GetY(), w.GetPoint2().GetX(),  w.GetPoint2().GetY());
#endif
     // getc(stdin);
     // if direction of pedestrians does not intersect walls --> ignore

     Point pt = w.ShortestPoint(ped->GetPos());
//     double wlen = w.LengthSquare();
     // if (wlen <= 0.03) { // ignore walls smaller than 0.15m  (15cm)
     //      return F_wrep;
     // }
     Point dist = pt - ped->GetPos(); // x- and y-coordinate of the distance between ped and p
     const double EPS = 0.001; // molified see Koester2013
     double Distance = dist.Norm() + EPS; // distance between the centre of ped and point p
     //double vn = w.NormalComp(ped->GetV()); //normal component of the velocity on the wall
     Point e_iw; // x- and y-coordinate of the normalized vector between ped and pt
     //double K_iw;
     double Radius, B_iw;
     double f;
     Point r;
     Point pinE; // vorher x1, y1
     const JEllipse& E = ped->GetEllipse();
     const Point& v = ped->GetV();
     double min_distance_to_wall = 0.1; // 10 cm
     if (Distance > min_distance_to_wall) {
           e_iw = dist / Distance;
     }
     else {
          // Log->Write("WARNING:\t Gompertz: forceRepWall() ped %d is too near to the wall",ped->GetID());
          Point new_dist = centroid - ped->GetPos();
          new_dist = new_dist/new_dist.Norm();
          
          e_iw = (inside ? new_dist:new_dist*-1);
          // Distance = EPS;
          // Log->Write("INFO:\t\t --- dist = %f, e= %f %f inside=%d",ped->GetID(), Distance, e_iw.GetX(), e_iw.GetY(), inside);
     }

//------------------------- check if others are behind using v0 instead of v
     // tmp = ped->GetV0().ScalarProduct(e_iw); // < v^0_i , e_iw >
     double tmpv = v.ScalarProduct(e_iw);
     //double wallIsBehind = exp(-exp(-5*tmp)); //step function: continuous version
     // double wallIsBehindv = exp(-exp(-5*tmpv)); //step function: continuous version
     double wallIsBehindv = (tmpv<=0)?0:1;
#if DEBUG
     if(ped->GetID()==-4){
     printf("Distance = %f tmpv=%f\n",Distance, tmpv);
     printf("v = %f, %f \n", v.GetX(), v.GetY());
     printf("pos = %f, %f \n", ped->GetPos().GetX(), ped->GetPos().GetY());
     printf("pt = %f, %f \n", pt.GetX(), pt.GetY());
     printf("e_iw = %f, %f\n",e_iw.GetX(), e_iw.GetY());
     printf("WallIsBehind = %f (%f)\n",wallIsBehindv,J_EPS);}
#endif

     if (wallIsBehindv < J_EPS && Distance > min_distance_to_wall) { // Wall is behind the direction of motion
          return F_wrep;
     }
//------------------------------------------------------------------------
     // pt in coordinate system of Ellipse
    pinE = pt.TransformToEllipseCoordinates(E.GetCenter(), E.GetCosPhi(), E.GetSinPhi());
     // Punkt auf der Ellipse
     r = E.PointOnEllipse(pinE);
     //double radiuss  = (r - E.GetCenter()).Norm();
     Radius = E.GetBmax();
     //-------------------------

     const Point& pos = ped->GetPos();
     double distGoal = ped->GetExitLine()->DistToSquare(pos);
     if(distGoal < J_EPS_GOAL*J_EPS_GOAL)
          return F_wrep;

     // Line  direction = Line(ped->GetPos(), ped->GetPos() + v*100);
     // if(Distance>Radius && !direction.IntersectionWith(w)) {
     //      return F_wrep;
     // }

//-------------------------

     double b = _bWall, c = _cWall;
     B_iw = 1.0 - Distance/(Radius);
     B_iw = exp(-b*exp(-c*B_iw));
     //f = -ped->GetMass() * _nuWall * ped->GetV0Norm() * K_iw * B_iw;
     f = -ped->GetMass() * _nuWall * B_iw  * ped->GetV0Norm();

     F_wrep = e_iw * f;

#if DEBUG
     printf("b=%f, c=%f, a=%f, m=%f\n",b,c,_nuWall, ped->GetMass());
     printf("Distance=%f, Radius=%f, B_iw=%f, G(B_iw)=%f\n",Distance, Radius, 1.0 - Distance/(Radius), B_iw);
     printf("\t\tf= %f, e_iw= %f, %f\n",f, e_iw.GetX(), e_iw.GetY() );
     printf("F_Rep = [%f, %f]\n---------------\n", F_wrep.GetX(), F_wrep.GetY());
     if (0 && std::fabs(f)>0.01)
           getc(stdin);
#endif
     return F_wrep;
}

string GompertzModel::GetDescription() const
{
     string rueck;
     char tmp[CLENGTH];
     sprintf(tmp, "\t\tNu: \t\tPed: %f \tWall: %f\n", _nuPed, _nuWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\ta: \t\tPed: %f \tWall: %f\n", _aPed, _aWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\tb: \t\tPed: %f \tWall: %f\n", _bPed, _bWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\tc: \t\tPed: %f \tWall: %f\n", _cPed, _cWall);
     rueck.append(tmp);
     return rueck;
}

DirectionStrategy* GompertzModel::GetDirection() const
{
     return _direction;
}

double GompertzModel::GetNuPed() const
{
     return _nuPed;
}

double GompertzModel::GetaPed() const
{
     return _aPed;
}

double GompertzModel::GetbPed() const
{
     return _bPed;
}

double GompertzModel::GetcPed() const
{
     return _cPed;
}

double GompertzModel::GetNuWall() const
{
     return _nuWall;
}


double GompertzModel::GetaWall() const
{
     return _aWall;
}

double GompertzModel::GetbWall() const
{
     return _bWall;
}

double GompertzModel::GetcWall() const
{
     return _cWall;
}

