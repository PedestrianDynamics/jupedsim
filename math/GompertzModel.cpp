/**
 * \file        GompertzModel.cpp
 * \date        Apr 15, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
                   "WARNING: \allPeds::Init() cannot initialise route. ped %d is deleted.\n",ped->GetID());
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

      //int debugPed = -69;//10;
      //building->GetGrid()->HighlightNeighborhood(-9, building);


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


                // if(debugPed != ped->GetID())
                // {
                //      Point p1 = ped->GetPos();
                //      Point p2 = ped->GetPos();
                //      fprintf(stderr, "%f     %f    %f    %f     %f   %d  %d  %d\n", time,  p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY(), -1, ped->GetID(), ped->GetID());
                // }


                double normVi = ped->GetV().ScalarP(ped->GetV()); //squared
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
                //if(ped->GetID()==61) building->GetGrid()->HighlightNeighborhood(ped,building);

                int size = (int) neighbours.size();
                // double B_ij=0;
                // int count_Bij=0;

                // if(debugPed == ped->GetID())
                // {
                //      printf("\n\n nsiZe=%d\n",size);
                // }

                for (int i = 0; i < size; i++) {
                     Pedestrian* ped1 = neighbours[i];
                     //-------------- TESTING ---------
                     // Point distp12 = ped1->GetPos() - ped->GetPos();
                     // double Distance = distp12.Norm();
                     // double tmp;
                     // tmp = 1.0 - Distance/(0.25 + 0.25);
                     // B_ij += exp(-_bPed*exp(-_cPed*tmp));
                     // if (B_ij > J_EPS)
                     //     count_Bij += 1;
                     //--------------------------------
                     //if they are in the same subroom
                     Point p1 = ped->GetPos();
                     Point p2 = ped1->GetPos();
                     bool isVisible = building->IsVisible(p1, p2, false);
                     if (!isVisible)
                          continue;
                     // if(debugPed == ped->GetID())
                     // {
                     //      fprintf(stderr, "%f     %f    %f    %f     %f   %d  %d  %d\n", current,  p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY(), isVisible, ped->GetID(), ped1->GetID());
                     //      printf("t=%.2f, ped:%d    ped1:%d   p1(%.2f, %.2f), p2(%.2f, %.2f) isVisibile = %d\n", current, ped->GetID(), ped1->GetID(), p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY(), isVisible);

                     // }
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

                // if(count_Bij)
                //     B_ij /=count_Bij;
                // else
                //     B_ij = 0;
                // double correction = -B_ij/ped->GetTau();

                // make pedestrians want to walk slower in jam
                // = fd ; //+ ped->GetV0()*correction;

                Point acc = (fd + repPed + repWall) / ped->GetMass();
                // if(1 || ped->GetID() == 976 ) {
                //      printf("Pos1 =[%f, %f]\n", ped->GetPos().GetX(), ped->GetPos().GetY());
                //      printf("acc= %f %f, fd= %f, %f,  repPed = %f %f, repWall= %f, %f\n", acc.GetX(), acc.GetY(), fd.GetX(), fd.GetY(), repPed.GetX(), repPed.GetY(), repWall.GetX(), repWall.GetY());
                //      getc(stdin);
                // }
                result_acc.push_back(acc);
           }

           //#pragma omp barrier
           // update
           for (int p = start; p <= end; ++p) {
                Pedestrian* ped = allPeds[p];

                Point vToAdd = result_acc[p - start] * deltaT;
                //----------------- update new pos and new vel -----------------
                
                Point v_neu = ped->GetV() + vToAdd;
                // printf("v_neu=[%f, %f], v=[%f, %f], toAdd=[%f, %f]\n", v_neu.GetX(), v_neu.GetY(), ped->GetV().GetX(), ped->GetV().GetY(), vToAdd.GetX(), vToAdd.GetY());
                // if(ped->GetID() == 2 )
                //      v_neu = Point(0,0);
                Point pos_neu = ped->GetPos() + v_neu * deltaT;
                //---------------------------------------------------------------

               
                if(v_neu.Norm() > ped->GetV0Norm()+0.2 ) { // Stop pedestrians

                     Log->Write("WARNING: \tped %d is stopped because v=%f (v0=%f)", ped->GetID(), v_neu.Norm(), ped->GetV0Norm());
                     v_neu = v_neu*0.01;
                     pos_neu = ped->GetPos();
                }
 // //--------------------------------------------------------------------------------------------------
 //                //Jam is based on the current velocity
                if ( v_neu.Norm() >= ped->GetV0Norm()*0.5) {
                     ped->ResetTimeInJam();
                } else {
                     ped->UpdateTimeInJam();
                }
                
 //--------------------------------------------------------------------------------------------------
                     //fprintf(stderr, "\n----\n%f %f %f %f %f %f\n----\n",ped->GetV().GetX(), ped->GetV().GetY(), ped->GetV0().GetX(),ped->GetV0().GetY(), ped->GetPos().GetX(), ped->GetPos().GetY());
                ped->SetPos(pos_neu);
                ped->SetV(v_neu);
                ped->SetPhiPed();
                // if(ped->GetID() == 976 ) {
                //      printf("toadd [%f, %f] m=%f\n", vToAdd.GetX(), vToAdd.GetY(), ped->GetMass());
                //      printf("pos_neu= %f %f  v_neu %f %f\n", pos_neu.GetX(), pos_neu.GetY(), v_neu.GetX(),  v_neu.GetY());
                // }
           }
      }//end parallel
}

Point GompertzModel::ForceDriv(Pedestrian* ped, Room* room) const
{
#define DEBUG 0

     // printf("GompertzModel::ForceDriv\n");
     const Point& target = _direction->GetTarget(room, ped);
     Point F_driv;
     Point e0;
     const Point& pos = ped->GetPos();
     double dist = ped->GetExitLine()->DistTo(pos);
    
     // check if the molified version works
     if (dist > J_EPS_GOAL) {
          e0 = ped->GetV0(target);
          // printf("1 e0 %f %f, target %f %f\n", e0.GetX(), e0.GetY(), target.GetX(), target.GetY());
     } else {
          ped->SetSmoothTurning();
          e0 = ped->GetV0();
            // printf("2 e0 %f %f\n", e0.GetX(), e0.GetY());
     }
      F_driv = ((e0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
  
      //double v =  sqrt(ped->GetV().GetX()*ped->GetV().GetX() +ped->GetV().GetY()*ped->GetV().GetY());
      //double e0norm = sqrt(e0.GetX()*e0.GetX() +e0.GetY()*e0.GetY());

#if DEBUG
      printf( "pos %f %f target %f %f\n", pos.GetX(), pos.GetY(), target.GetX(), target.GetY());
      printf("mass=%f, v0norm=%f, v=%f, e0Norm=%f, tau=%f\n", ped->GetMass(), ped->GetV0Norm(), v , e0norm,ped->GetTau());
      printf("Fdriv=  [%f, %f]\n", F_driv.GetX(), F_driv.GetY());
      fprintf(stderr, "%d   %f    %f    %f    %f    %f    %f\n", ped->GetID(), ped->GetPos().GetX(), ped->GetPos().GetY(), ped->GetV().GetX(), ped->GetV().GetY(), target.GetX(), target.GetY());
      getc(stdin);
#endif

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
     Point AP1inE2 = AP1inE1.CoordTransToEllipse(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
     // ActionPoint von E2 in Koordinaten von E1 (transformieren)
     Point AP2inE1 = AP2inE2.CoordTransToEllipse(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
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
     double tmpv = ped1->GetV().ScalarP(ep12); // < v^0_i , e_ij >
     double ped2IsBehindv = exp(-exp(-5*tmpv)); //step function: continuous version
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
     // if(ped1->GetID() == 1) {
     //      printf("F=[%f, %f] v0=%f, nu=%f, B_ij=%f D=%f, r1=%f, r2=%f\n", F_rep.GetX(), F_rep.GetY(), ped1->GetV0Norm(), _nuPed, B_ij, Distance, r1, r2);
     // }
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
     //first the walls
     const vector<Wall>& walls = subroom->GetAllWalls();
     for (int i = 0; i < subroom->GetNumberOfWalls(); i++) {
          f += ForceRepWall(ped, walls[i]);
     }

     //then the obstacles
     const vector<Obstacle*>& obstacles = subroom->GetAllObstacles();
     for(unsigned int obs=0; obs<obstacles.size(); ++obs) {
          const vector<Wall>&getAllWalls = obstacles[obs]->GetAllWalls();
          for (unsigned int i = 0; i < getAllWalls.size(); i++) {
               f += ForceRepWall(ped, getAllWalls[i]);
          }
     }
     // and finally the closed doors
     const vector<Transition*>& transitions = subroom->GetAllTransitions();
     for (unsigned int i = 0; i < transitions.size(); i++) {
          Transition* goal=transitions[i];
          if(! goal->IsOpen()) {
               f +=  ForceRepWall(ped,*((Wall*)goal));
          }
     }

     return f;
}

Point GompertzModel::ForceRepWall(Pedestrian* ped, const Wall& w) const
{
#define DEBUG 0
     Point F_wrep = Point(0.0, 0.0);
#if DEBUG
     printf("in GompertzWall\n");
#endif
     // getc(stdin);
     // if direction of pedestrians does not intersect walls --> ignore

     Point pt = w.ShortestPoint(ped->GetPos());
     double wlen = w.LengthSquare();
     if (wlen <= 0.03) { // ignore walls smaller than 0.15m  (15cm)
          return F_wrep;
     }
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

     if (Distance < J_EPS) {
          Log->Write("WARNING:\t Gompertz: forceRepWall() ped %d is too near to the wall. Return default values",ped->GetID());
          return Point(0, 0); //quick and dirty. Should react to the warning and fix the model
     }
     e_iw = dist / Distance;
//------------------------- check if others are behind using v0 instead of v
     // tmp = ped->GetV0().ScalarP(e_iw); // < v^0_i , e_iw >
     double tmpv = v.ScalarP(e_iw);
     //double wallIsBehind = exp(-exp(-5*tmp)); //step function: continuous version
     // double wallIsBehindv = exp(-exp(-5*tmpv)); //step function: continuous version
     double wallIsBehindv = (tmpv<=0)?0:1;
#if DEBUG
     
     printf("Distance = %f tmpv=%f\n",Distance, tmpv);
     printf("v = %f, %f \n", v.GetX(), v.GetY());
     printf("pos = %f, %f \n", ped->GetPos().GetX(), ped->GetPos().GetY());
     printf("pt = %f, %f \n", pt.GetX(), pt.GetY());
     printf("e_iw = %f, %f\n",e_iw.GetX(), e_iw.GetY());
     printf("WallIsBehind = %f (%f)\n",wallIsBehindv,J_EPS);
#endif

     if (wallIsBehindv < J_EPS) { // Wall is behind the direction of motion
          return F_wrep;
     }
//------------------------------------------------------------------------
     // pt in coordinate system of Ellipse
     pinE = pt.CoordTransToEllipse(E.GetCenter(), E.GetCosPhi(), E.GetSinPhi());
     // Punkt auf der Ellipse
     r = E.PointOnEllipse(pinE);
     Radius  = (r - E.GetCenter()).Norm();
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

     //TODO: Check later if other values are more appropriate
     //double b = 0.7, c = 3.0;
     double b = _bWall, c = _cWall;
     B_iw = 1.0 - Distance/(Radius);
     B_iw = exp(-b*exp(-c*B_iw));
#if DEBUG
     printf("b=%f, c=%f, a=%f, m=%f\n",b,c,_nuWall, ped->GetMass());
     printf("Distance=%f, Radius=%f, B_iw=%f, G(B_iw)=%f\n",Distance, Radius, 1.0 - Distance/(Radius), B_iw);
     printf("f= %f, e_iw= %f, %f\n",f, e_iw.GetX(), e_iw.GetY() );
     getc(stdin);
#endif
     //f = -ped->GetMass() * _nuWall * ped->GetV0Norm() * K_iw * B_iw;
     f = -ped->GetMass() * _nuWall * B_iw  * ped->GetV0Norm();

     F_wrep = e_iw * f;
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

