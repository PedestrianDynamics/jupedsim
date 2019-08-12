/**
 * \file        GradientModel.cpp
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
#include "GradientModel.h"

#include "general/OpenMP.h"
#include "geometry/SubRoom.h"
#include "geometry/Wall.h"
#include "mpi/LCGrid.h"
#include "pedestrian/Pedestrian.h"
#include "direction/DirectionManager.h"
#include "direction/walking/DirectionFloorfield.h"
#include "direction/walking/DirectionLocalFloorfield.h"
#include "direction/walking/DirectionSubLocalFloorfield.h"

#include <math.h>

GradientModel::GradientModel(std::shared_ptr<DirectionManager> dir, double nuped, double aped, double bped, double cped,
                             double nuwall, double awall, double bwall, double cwall,
                             double deltaH, double wallAvoidDistance, bool useWallAvoidance,
                             double slowDownDistance)
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
     // floorfield Parameter
     _deltaH = deltaH;
     _wallAvoidDistance = wallAvoidDistance;
     _useWallAvoidance = useWallAvoidance;
     // anti clipping
     _slowDownDistance = slowDownDistance;

     over = new long int;
     under = new long int;
     redircnt = new long int;
     slowcnt = new long int;
     overlapcnt = new long int;

     *over = 0;   //analyze code only - can be removed
     *under = 0;  //analyze code only - can be removed
     *redircnt = 0;  //analyze code only - can be removed
     *slowcnt = 0;  //analyze code only - can be removed
     *overlapcnt = 0;
}


GradientModel::~GradientModel()
{
    delete over;
    delete under;
    delete redircnt;
    delete slowcnt;
    delete overlapcnt;
}

bool GradientModel::Init (Building* building)
{

     _direction->Init(building);
//     if(auto dirff = dynamic_cast<DirectionFloorfield*>(_direction.get())){
//          Log->Write("INFO:\t Init DirectionFloorfield starting ...");
//          double _deltaH = building->GetConfig()->get_deltaH();
//          double _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
//          bool _useWallAvoidance = building->GetConfig()->get_use_wall_avoidance();
//          dirff->Init(building, _deltaH, _wallAvoidDistance, _useWallAvoidance);
//          Log->Write("INFO:\t Init DirectionFloorfield done");
//     }
//
//     if(auto dirlocff = dynamic_cast<DirectionLocalFloorfield*>(_direction.get())){
//          Log->Write("INFO:\t Init DirectionLOCALFloorfield starting ...");
//          double _deltaH = building->GetConfig()->get_deltaH();
//          double _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
//          bool _useWallAvoidance = building->GetConfig()->get_use_wall_avoidance();
//          dirlocff->Init(building, _deltaH, _wallAvoidDistance, _useWallAvoidance);
//          Log->Write("INFO:\t Init DirectionLOCALFloorfield done");
//     }
//
//     if(auto dirsublocff = dynamic_cast<DirectionSubLocalFloorfield*>(_direction.get())){
//          Log->Write("INFO:\t Init DirectionSubLOCALFloorfield starting ...");
//          double _deltaH = building->GetConfig()->get_deltaH();;
//          double _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
//          bool _useWallAvoidance = building->GetConfig()->get_use_wall_avoidance();
//          dirsublocff->Init(building, _deltaH, _wallAvoidDistance, _useWallAvoidance);
//          Log->Write("INFO:\t Init DirectionSubLOCALFloorfield done");
//     }

     const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

     std::vector<Pedestrian*> pedsToRemove;
     pedsToRemove.clear();
     bool error_occurred = false;
#pragma omp parallel for
    for(size_t p=0;p<allPeds.size();p++) {
         Pedestrian* ped = allPeds[p];
         double cosPhi = 0;
         double sinPhi = 0;
         //a destination could not be found for that pedestrian
         if (ped->FindRoute() == -1) {
              Log->Write(
                   "ERROR:\tGradientModel::Init() cannot initialise route. ped %d is scheduled for deletion.\n",ped->GetID());
             //building->DeletePedestrian(ped);
#pragma omp critical(GradientModel_Init_pedsToRemove)
              pedsToRemove.emplace_back(ped);
              continue;
         }

         Line* e = ped->GetExitLine();
         const Point& e1 = e->GetPoint1();
         const Point& e2 = e->GetPoint2();
         Point target = (e1 + e2) * 0.5;
         Point d = target - ped->GetPos();
         double dist = d.Norm();
         if (dist != 0.0) {
              cosPhi = d._x / dist;
              sinPhi = d._y / dist;
         } else {
              Log->Write(
                   "ERROR: \allPeds::Init() cannot initialise phi! "
                   "dist to target is 0\n");
#pragma omp critical(GradientModel_Init_error_occurred)
              error_occurred = true;
              //return false;
         }
         // This skips the rest of the initialization if any pedestrian could not be initialized
         if (error_occurred) continue;

         ped->InitV0(target);

         JEllipse E = ped->GetEllipse();
         E.SetCosPhi(cosPhi);
         E.SetSinPhi(sinPhi);
         ped->SetEllipse(E);
    }

     for (auto ped : pedsToRemove) {
          building->DeletePedestrian(ped);
     }
     return !error_occurred;
}

void GradientModel::ComputeNextTimeStep(double current, double deltaT, Building* building, int periodic)
{
     double delta = 0.5;
      // collect all pedestrians in the simulation.
      const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

     unsigned long nSize;
     nSize = allPeds.size();
     Pedestrian* minAddress = nullptr; //non-elegant fix for faulty neighbours[...] ptr avoidance
     Pedestrian* maxAddress = nullptr;

     if (nSize >= 1) {
        minAddress = allPeds[0];
        maxAddress = allPeds[0];
        for (auto& pedptr:allPeds) {
            if (&(*pedptr) < minAddress) {
                minAddress = &(*pedptr);
            }
            if (pedptr > maxAddress) {
                maxAddress = &(*pedptr);
            }
        }
     }

     int nThreads = omp_get_max_threads();

     int partSize;
     partSize = (int) (nSize / nThreads);

      #pragma omp parallel  default(shared) num_threads(nThreads)
      {
           std::vector< Point > result_acc = std::vector<Point > ();
           result_acc.reserve(nSize);

           const int threadID = omp_get_thread_num();

           int start = threadID*partSize;
           int end;
           end = (threadID + 1) * partSize - 1;
           if ((threadID == nThreads - 1)) end = (int) (nSize - 1);

//DEBUG start
//           start = 0;
//           end = nSize-1; // loop till p<= end !!!
//DEBUG end
           for (int p = start; p <= end; ++p) {

                Pedestrian* ped = allPeds[p];
                Room* room = building->GetRoom(ped->GetRoomID());
                SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());

                double normVi = ped->GetV().ScalarProduct(ped->GetV()); //squared
                double HighVel = (ped->GetV0Norm() + delta) * (ped->GetV0Norm() + delta); //(v0+delta)^2
                if (normVi > HighVel && true) {
                     fprintf(stderr, "GradientModel::calculateForce_LC() WARNING: actual velocity (%f) of iped %d "
                             "is bigger than desired velocity (%f) at time: %fs (periodic=%d)\n",
                             sqrt(normVi), ped->GetID(), ped->GetV0Norm(), current, periodic);

                     // remove the pedestrian and abort
                     Log->Write("\tERROR: ped [%d] was removed due to high velocity",ped->GetID());
                     building->DeletePedestrian(ped);
                     Log->incrementDeletedAgents();
                     //continue;  //FIXME tolerate first
                     exit(EXIT_FAILURE);
                }

                Point repPed = Point(0,0);
                std::vector<Pedestrian*> neighbours;
                building->GetGrid()->GetNeighbourhood(ped,neighbours);
                int size = (int) neighbours.size();
                for (int i = 0; i < size; i++) {
                     Pedestrian* ped1 = neighbours[i];
                     if ((minAddress > neighbours[i]) || (maxAddress < neighbours[i])) {
                        std::cerr << "## Skiped " << i << " of " << size << " #### " << ped1 << " " << minAddress << " " << maxAddress << std::endl;
                        continue;
                     }
                     //if they are in the same subroom
                     Point p1 = ped->GetPos();
                     Point p2 = ped1->GetPos();

                     //subrooms to consider when looking for neighbour for the 3d visibility
                     std::vector<SubRoom*> emptyVector;
                     emptyVector.push_back(subroom);
                     emptyVector.push_back(building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID()));

                     bool isVisible = building->IsVisible(p1, p2, emptyVector, false);
                     if (!isVisible) {
                          continue;
                     }
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
                result_acc.push_back(repPed); //only orientation is of interest
           }

           // update

           for (int p = start; p <= end; ++p) {
                Pedestrian* ped = allPeds[p];
                if (result_acc[p-start].Norm() > 1) {
                    ++(*over);
                } else {
                    ++(*under);
                }
                Point movDirection = (result_acc[p-start].Norm() > 1) ? result_acc[p - start].Normalized() : result_acc[p-start];
                Point toTarget = (_direction->GetTarget(building->GetRoom(ped->GetRoomID()), ped));
                toTarget = toTarget - ped->GetPos();
                if (toTarget.NormSquare() == 0.) {                // this if overcomes shortcomming of floorfield (neggrad[targetpoints] == Point(0., 0.))
                    toTarget += ped->GetV().Normalized();
                }

                movDirection = (movDirection + toTarget);
                movDirection = (movDirection.Norm() > 1.) ? movDirection.Normalized() : movDirection;

                double desired_speed = ped->GetV0Norm();
                Point oldMov = Point(0., 0.);
                if (desired_speed > 0.) {
                    oldMov = ped->GetV() / desired_speed; // (GetV() returns a vector: distance travelled in one time-unit (s)
                }                                         // so oldMov is now: vector distance travelled in one time-unit with unit speed = sth around unit-vector or smaller

                //anti jitter               //_V0 = _V0 + (new_v0 - _V0)*( 1 - exp(-t/_tau) );
                oldMov = (oldMov.Norm() > 1.)? oldMov.Normalized() : oldMov; //on the safe side ... should not be necessary as it must be [0, 1]
                Point diff = ( oldMov - movDirection) * (.2); // .2 also 80% alte Richtung, 20% neue Richtung
                movDirection = oldMov - diff;
                movDirection = (movDirection.Norm() > 1.) ? movDirection.Normalized() : movDirection;

                //redirect near wall mechanics:
                Point dir2Wall = Point{0., 0.};
                double distance2Wall = -1.;
                if (auto dirff = dynamic_cast<DirectionFloorfield*>(_direction->GetDirectionStrategy().get())) {
                     dir2Wall = dirff->GetDir2Wall(ped);
                     distance2Wall = dirff->GetDistance2Wall(ped);
                } else if (auto dirlocff = dynamic_cast<DirectionLocalFloorfield*>(_direction->GetDirectionStrategy().get())) {
                     dir2Wall = dirlocff->GetDir2Wall(ped);
                     distance2Wall = dirlocff->GetDistance2Wall(ped);
                } else if (auto dirsublocff = dynamic_cast<DirectionSubLocalFloorfield*>(_direction->GetDirectionStrategy().get())) {
                     dir2Wall = dirsublocff->GetDir2Wall(ped);
                     distance2Wall = dirsublocff->GetDistance2Wall(ped);
                } else {
                     Log->Write("ERROR: \t GradNav Model (4) requires any floor field (exit-strat {6,8,9}). None found!");
                }
                double dotProduct = 0;
                double antiClippingFactor = 1;
                if (distance2Wall < _slowDownDistance) {
                    dotProduct = movDirection.ScalarProduct(dir2Wall);
                    if ((dotProduct > 0) && (distance2Wall < .5 * _slowDownDistance)) { //acute angle && really close to wall
                        movDirection = movDirection - (dir2Wall*dotProduct); //remove walldirection from movDirection
                        ++(*redircnt);
                    }
                    antiClippingFactor = ( 1 - .5*(dotProduct + fabs(dotProduct)) );
                    ++(*slowcnt);
                }

                movDirection = movDirection * (antiClippingFactor * ped->GetV0Norm() * deltaT);

                Point pos_neu = ped->GetPos() + movDirection;

                ped->SetPos(pos_neu);
                ped->SetV(movDirection/deltaT);
                ped->SetPhiPed();
           }

      }//end parallel
      //std::cerr << "Over : Under  " << *over << " : " << *under << "    (" << *redircnt << ")" << "    (" << *slowcnt << ")" << "    (" << *overlapcnt << ")" << std::endl;
}

Point GradientModel::ForceDriv(Pedestrian* ped, Room* room) const
{
#define DEBUG 0

     //printf("GradientModel::ForceDriv\n");
     const Point& target = _direction->GetTarget(room, ped);
     Point F_driv;
     //Point e0;
     //const Point& pos = ped->GetPos();
     //double dist = ped->GetExitLine()->DistTo(pos);

     // check if the molified version works
     //if (dist > J_EPS_GOAL) {
     //     e0 = ped->GetV0(target);
     //} else {
     //     ped->SetSmoothTurning();
     //     e0 = ped->GetV0();
     //}
     F_driv = ((target.Normalized() * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();

      //double v =  sqrt(ped->GetV()._x*ped->GetV()._x +ped->GetV()._y*ped->GetV()._y);
      //double e0norm = sqrt(e0._x*e0._x +e0._y*e0._y);

#if DEBUG
      printf( "pos %f %f target %f %f\n", pos._x, pos._y, target._x, target._y);
      printf("mass=%f, v0norm=%f, v=%f, e0Norm=%f, tau=%f\n", ped->GetMass(), ped->GetV0Norm(), v , e0norm,ped->GetTau());
      printf("Fdriv=  [%f, %f]\n", F_driv._x, F_driv._y);
      fprintf(stderr, "%d   %f    %f    %f    %f    %f    %f\n", ped->GetID(), ped->GetPos()._x, ped->GetPos()._y, ped->GetV()._x, ped->GetV()._y, target._x, target._y);
      getc(stdin);
#endif

     //return F_driv;
     return target;
}

Point GradientModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const
{
     Point F_rep(0.0, 0.0);
     // x- and y-coordinate of the distance between p1 and p2
     Point distp12 = ped2->GetPos() - ped1->GetPos();
     //const Point& vp1 = ped1->GetV(); // v Ped1
     //const Point& vp2 = ped2->GetV(); // v Ped2
     Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
     //double K_ij;
     double B_ij, f; // tmp2;
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
          //printf("ERROR: \tin GradientModel::forcePedPed() ep12 can not be calculated!!!\n");
          Log->Write("WARNING: \tin GradientModel::forcePedPed() ep12 can not be calculated!!!\n");
          Log->Write("\t\t Pedestrians are too near to each other.");
          Log->Write("\t\t Get your model right. Going to exit.");
          return F_rep; // should never happen
          exit(EXIT_FAILURE);
     }
//------------------------- check if others are behind using v0 instead of v
     double tmpv = ped1->GetV().ScalarProduct(ep12); // < v^0_i , e_ij >
     double ped2IsBehindv = exp(-exp(-5*tmpv)); //step function: continuous version
     if (ped2IsBehindv < J_EPS) {
     //if (tmpv < 0) {
          return F_rep; // ignore ped2
     }

//--------------------------check if overlapping
     //double* unused = new double;
     //double tmpover = E1.EffectiveDistanceToEllipse(E2, unused);
//     if (tmpover < 0) {
//         std::cerr << "Fehler aaaaa " << std::endl;
//         getc(stdin);
//         ++(*overlapcnt);
//         return ep12 * (-5);
//     }
//--------------------------check speed diff
//     tmp2 = (vp1 - vp2).ScalarProduct(ep12); // < v_ij , e_ij >
//     if (tmp2 < 0) {
//          return F_rep; // ignore ped2
//     }

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
     //check isNan
     if (F_rep._x != F_rep._x || F_rep._y != F_rep._y) {
          char tmp[CLENGTH];
          sprintf(tmp, "\nNAN return ----> p1=%d p2=%d Frepx=%f, Frepy=%f\n", ped1->GetID(),
                  ped2->GetID(), F_rep._x, F_rep._y);
          Log->Write(tmp);
          Log->Write("ERROR:\t fix this as soon as possible");
          return Point(0,0); // FIXME: should never happen
          exit(EXIT_FAILURE);
     }
     return F_rep;
}//END Gompertz:ForceRepPed()

Point GradientModel::ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const
{
     Point f(0., 0.);
     //first the walls
     for(const auto & wall: subroom->GetAllWalls())
     {
          f += ForceRepWall(ped, wall);
     }
     //then the obstacles

     for(const auto & obst: subroom->GetAllObstacles())
     {
          for(const auto & wall: obst->GetAllWalls())
          {
               f += ForceRepWall(ped, wall);
          }
     }

     // and finally the closed doors
     for(auto & goal: subroom->GetAllTransitions())
     {
          if(! goal->IsOpen()) {
               f +=  ForceRepWall(ped,*(static_cast<Line*>(goal)));
          }
     }

     return f;
}

Point GradientModel::ForceRepWall(Pedestrian* ped, const Line& w) const
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
     // tmp = ped->GetV0().ScalarProduct(e_iw); // < v^0_i , e_iw >
     double tmpv = v.ScalarProduct(e_iw);
     //double wallIsBehind = exp(-exp(-5*tmp)); //step function: continuous version
     // double wallIsBehindv = exp(-exp(-5*tmpv)); //step function: continuous version
     double wallIsBehindv = (tmpv<=0)?0:1;
#if DEBUG

     printf("Distance = %f tmpv=%f\n",Distance, tmpv);
     printf("v = %f, %f \n", v._x, v._y);
     printf("pos = %f, %f \n", ped->GetPos()._x, ped->GetPos()._y);
     printf("pt = %f, %f \n", pt._x, pt._y);
     printf("e_iw = %f, %f\n",e_iw._x, e_iw._y);
     printf("WallIsBehind = %f (%f)\n",wallIsBehindv,J_EPS);
#endif

     if (wallIsBehindv < J_EPS) { // Wall is behind the direction of motion
          return F_wrep;
     }
//------------------------------------------------------------------------
     // pt in coordinate system of Ellipse
     pinE = pt.TransformToEllipseCoordinates(E.GetCenter(), E.GetCosPhi(), E.GetSinPhi());
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
     printf("f= %f, e_iw= %f, %f\n",f, e_iw._x, e_iw._y );
     getc(stdin);
#endif
     //f = -ped->GetMass() * _nuWall * ped->GetV0Norm() * K_iw * B_iw;
     f = -ped->GetMass() * _nuWall * B_iw  * ped->GetV0Norm();

     F_wrep = e_iw * f;
     return F_wrep;
}

std::string GradientModel::GetDescription()
{
     std::string rueck;
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

//std::shared_ptr<DirectionStrategy> GradientModel::GetDirection() const
//{
//     return _direction;
//}

double GradientModel::GetNuPed() const
{
     return _nuPed;
}

double GradientModel::GetaPed() const
{
     return _aPed;
}

double GradientModel::GetbPed() const
{
     return _bPed;
}

double GradientModel::GetcPed() const
{
     return _cPed;
}

double GradientModel::GetNuWall() const
{
     return _nuWall;
}


double GradientModel::GetaWall() const
{
     return _aWall;
}

double GradientModel::GetbWall() const
{
     return _bWall;
}

double GradientModel::GetcWall() const
{
     return _cWall;
}
