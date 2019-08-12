/**
 * \file        KrauszModel.cpp
 * \date        Jul 19, 2016
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
 * 1. Generalized Centrifugal Force Model
 *
 *
 **/
#include "KrauszModel.h"

#include "general/OpenMP.h"
#include "geometry/SubRoom.h"
#include "geometry/Wall.h"
#include "mpi/LCGrid.h"
#include "pedestrian/Pedestrian.h"
#include "direction/DirectionManager.h"
#include "direction/walking/DirectionFloorfield.h"
#include "direction/walking/DirectionLocalFloorfield.h"
#include "direction/walking/DirectionSubLocalFloorfield.h"


KrauszModel::KrauszModel(std::shared_ptr<DirectionManager> dir, double nuped, double nuwall, double dist_effPed,
                     double dist_effWall, double intp_widthped, double intp_widthwall, double maxfped,
                     double maxfwall)
{
     _direction = dir;
     _nuPed = nuped;
     _nuWall = nuwall;
     _intp_widthPed = intp_widthped;
     _intp_widthWall = intp_widthwall;
     _maxfPed = maxfped;
     _maxfWall = maxfwall;
     _distEffMaxPed = dist_effPed;
     _distEffMaxWall = dist_effWall;

}

KrauszModel::~KrauszModel(void)
{

}


bool KrauszModel::Init (Building* building)
{

     _direction->Init(building);

     const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();
     size_t peds_size = allPeds.size();
     for(unsigned int p=0;p<peds_size;p++)
     {
          Pedestrian* ped = allPeds[p];
          double cosPhi, sinPhi;
          //a destination could not be found for that pedestrian
          if (ped->FindRoute() == -1) {
               building->DeletePedestrian(ped);
               Log->incrementDeletedAgents();
               --p;
               --peds_size;
               continue;
          }

          Point target = ped->GetExitLine()->LotPoint(ped->GetPos());
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

void KrauszModel::ComputeNextTimeStep(double current, double deltaT, Building* building, int periodic)
{
     double delta = 1.5;

     // collect all pedestrians in the simulation.
     const std::vector< Pedestrian* >& allPeds = building->GetAllPedestrians();

     unsigned int nSize = allPeds.size();
     int nThreads = omp_get_max_threads();


     int partSize;
     partSize = ((int)nSize > nThreads)? (int) (nSize / nThreads):(int)nSize;
     if(partSize == (int)nSize)
          nThreads = 1; // not worthy to parallelize

     int debugPed = -10;
     //building->GetGrid()->HighlightNeighborhood(debugPed, building);
#pragma omp parallel  default(shared) num_threads(nThreads)
     {
          std::vector< Point > result_acc = std::vector<Point > ();
          result_acc.reserve(2200);

          const int threadID = omp_get_thread_num();

          int start = threadID*partSize;
          int end;
          end = (threadID < nThreads - 1) ? (threadID + 1) * partSize - 1: (int) (nSize - 1);

          for (int p = start; p <= end; ++p) {

               Pedestrian* ped = allPeds[p];
               Room* room = building->GetRoom(ped->GetRoomID());
               SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
               //if(subroom->GetType()=="cellular") continue;


               double normVi = ped->GetV().ScalarProduct(ped->GetV());
               double tmp = (ped->GetV0Norm() + delta) * (ped->GetV0Norm() + delta);
               if (normVi > tmp && ped->GetV0Norm() > 0) {
                    fprintf(stderr, "KrauszModel::calculateForce() WARNING: actual velocity (%f) of iped %d "
                                    "is bigger than desired velocity (%f) at time: %fs (periodic=%d)\n",
                            sqrt(normVi), ped->GetID(), ped->GetV0Norm(), current, periodic);
                    // remove the pedestrian and abort
                    building->DeletePedestrian(ped);
                    Log->incrementDeletedAgents();
                    Log->Write("\tERROR: one ped was removed due to high velocity");
                    //exit(EXIT_FAILURE);
               }

               Point F_rep;
               std::vector<Pedestrian*> neighbours;
               building->GetGrid()->GetNeighbourhood(ped,neighbours);
               //if(ped->GetID()==61) building->GetGrid()->HighlightNeighborhood(ped,building);
               std::vector<SubRoom*> emptyVector;

               int neighborsSize = neighbours.size();
               for (int i = 0; i < neighborsSize; i++) {
                    Pedestrian* ped1 = neighbours[i];
                    Point p1 = ped->GetPos();
                    Point p2 = ped1->GetPos();
                    bool ped_is_visible = building->IsVisible(p1, p2, emptyVector, false);
                    if (!ped_is_visible)
                         continue;
                    // if(debugPed == ped->GetID())
                    // {
                    //      fprintf(stdout, "t=%f     %f    %f    %f     %f   %d  %d  %d\n", time,  p1._x, p1._y, p2._x, p2._y, isVisible, ped->GetID(), ped1->GetID());
                    // }
                    //if they are in the same subroom
                    if (ped->GetUniqueRoomID() == ped1->GetUniqueRoomID()) {
                         F_rep = F_rep + ForceRepPed(ped, ped1);
                    } else {
                         // or in neighbour subrooms
                         SubRoom* sb2=building->GetRoom(ped1->GetRoomID())->GetSubRoom(ped1->GetSubRoomID());
                         if(subroom->IsDirectlyConnectedWith(sb2)) {
                              F_rep = F_rep + ForceRepPed(ped, ped1);
                         }
                    }
               }//for peds

               //lateral swaying of pedestrians
               Point oscil = AccelOscil(ped);

               //repulsive forces to the walls and transitions that are not my target
               Point repwall = ForceRepRoom(allPeds[p], subroom);
               Point fd = ForceDriv(ped, room);
               // Point acc = (ForceDriv(ped, room) + F_rep + repwall) / ped->GetMass();
               Point acc = (fd + F_rep + repwall) / ped->GetMass() + oscil;

               // if(ped->GetID() ==  debugPed){
               //      printf("%f   %f    %f    %f   %f   %f\n", fd._x, fd._y, F_rep._x, F_rep._y, repwall._x, repwall._y);

               // }
               if(ped->GetID() == debugPed ) {
                    //printf("\nt=%f, Pos1 =[%f, %f]\n", current,ped->GetPos()._x, ped->GetPos()._y);
                    printf("\nacc= %f %f, fd= %f, %f,  repPed = %f %f, repWall= %f, %f\n", acc._x, acc._y, fd._x, fd._y, F_rep._x, F_rep._y, repwall._x, repwall._y);
               }

               result_acc.push_back(acc);
          }

#pragma omp barrier
          // update
          for (int p = start; p <= end; ++p) {
               Pedestrian* ped = allPeds[p];
               Point v_neu = ped->GetV() + result_acc[p - start] * deltaT;
               Point pos_neu = ped->GetPos() + v_neu * deltaT;

               //Room* room = building->GetRoom(ped->GetRoomID());
               //SubRoom* subroom = room->GetSubRoom(ped->GetSubRoomID());
               //if(subroom->GetType()=="cellular") continue;

               //Jam is based on the current velocity
               if (v_neu.Norm() >= J_EPS_V) {
                    ped->ResetTimeInJam();
               } else {
                    ped->UpdateTimeInJam();
               }

               ped->SetPos(pos_neu);
               ped->SetV(v_neu);
               ped->SetPhiPed();
          }

     }//end parallel

}


inline  Point KrauszModel::ForceDriv(Pedestrian* ped, Room* room) const
{
     const Point& target = _direction->GetTarget(room, ped);
     Point F_driv;
     const Point& pos = ped->GetPos();
     double dist = ped->GetExitLine()->DistTo(pos);
     Point lastE0 = ped->GetLastE0();
     ped->SetLastE0(target-pos);

     if (  (dynamic_cast<DirectionFloorfield*>(_direction->GetDirectionStrategy().get())) ||
           (dynamic_cast<DirectionLocalFloorfield*>(_direction->GetDirectionStrategy().get())) ||
           (dynamic_cast<DirectionSubLocalFloorfield*>(_direction->GetDirectionStrategy().get()))  ) {
          if (dist > 50*J_EPS_GOAL) {
               const Point& v0 = ped->GetV0(target);
               F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
          } else {
               F_driv = ((lastE0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
               ped->SetLastE0(lastE0);
          }
     }
     else if (dist > J_EPS_GOAL) {
          const Point& v0 = ped->GetV0(target);
          F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
     } else {
          const Point& v0 = ped->GetV0();
          F_driv = ((v0 * ped->GetV0Norm() - ped->GetV()) * ped->GetMass()) / ped->GetTau();
     }

     return F_driv;
}

Point KrauszModel::ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const
{
     Point F_rep;
     // x- and y-coordinate of the distance between p1 and p2
     Point distp12 = ped2->GetPos() - ped1->GetPos();
     const Point& vp1 = ped1->GetV(); // v Ped1
     const Point& vp2 = ped2->GetV(); // v Ped2
     Point ep12; // x- and y-coordinate of the normalized vector between p1 and p2
     double tmp, tmp2;
     double v_ij;
     double K_ij;
     //double r1, r2;
     double nom; //nominator of Frep
     double px; // hermite Interpolation value
     const JEllipse& E1 = ped1->GetEllipse();
     const JEllipse& E2 = ped2->GetEllipse();
     double distsq;
     double dist_eff = E1.EffectiveDistanceToEllipse(E2, &distsq);


     //          smax    dist_intpol_left      dist_intpol_right       dist_eff_max
     //       ----|-------------|--------------------------|--------------|----
     //       5   |     4       |            3             |      2       | 1

     // If the pedestrian is outside the cutoff distance, the force is zero.
     if (dist_eff >= _distEffMaxPed) {
          F_rep = Point(0.0, 0.0);
          return F_rep;
     }
     //Point AP1inE1 = Point(E1.GetXp(), 0); // ActionPoint von E1 in Koordinaten von E1
     //Point AP2inE2 = Point(E2.GetXp(), 0); // ActionPoint von E2 in Koordinaten von E2
     // ActionPoint von E1 in Koordinaten von E2 (transformieren)
     //Point AP1inE2 = AP1inE1.TransformToEllipseCoordinates(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
     // ActionPoint von E2 in Koordinaten von E1 (transformieren)
     //Point AP2inE1 = AP2inE2.TransformToEllipseCoordinates(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
     //r1 = (AP1inE1 - E1.PointOnEllipse(AP2inE1)).Norm();
     //r2 = (AP2inE2 - E2.PointOnEllipse(AP1inE2)).Norm();

     //%------- Free parameter --------------
     Point p1, p2; // "Normale" Koordinaten
     double mindist;


     p1 = Point(E1.GetXp(), 0).TransformToCartesianCoordinates(E1.GetCenter(), E1.GetCosPhi(), E1.GetSinPhi());
     p2 = Point(E2.GetXp(), 0).TransformToCartesianCoordinates(E2.GetCenter(), E2.GetCosPhi(), E2.GetSinPhi());
     distp12 = p2 - p1;
     //mindist = E1.MinimumDistanceToEllipse(E2); //ONE
     mindist = 0.5; //for performance reasons, it is assumed that this distance is about 50 cm
     double dist_intpol_left = mindist + _intp_widthPed; // lower cut-off for Frep (modCFM)
     double dist_intpol_right = _distEffMaxPed - _intp_widthPed; //upper cut-off for Frep (modCFM)
     double smax = mindist - _intp_widthPed; //max overlapping
     double f = 0.0f, f1 = 0.0f; //function value and its derivative at the interpolation point'

     //todo: runtime normsquare?
     if (distp12.Norm() >= J_EPS) {
          ep12 = distp12.Normalized();

     } else {
          Log->Write("ERROR: \tin KrauszModel::forcePedPed() ep12 kann nicht berechnet werden!!!\n");
          Log->Write("ERROR:\t fix this as soon as possible");
          return F_rep; // FIXME: should never happen
          exit(EXIT_FAILURE);

     }
     // calculate the parameter (whatever dist is)
     tmp = (vp1 - vp2).ScalarProduct(ep12); // < v_ij , e_ij >
     v_ij = 0.5 * (tmp + fabs(tmp));
     tmp2 = vp1.ScalarProduct(ep12); // < v_i , e_ij >

     //todo: runtime normsquare?
     if (vp1.Norm() < J_EPS) { // if(norm(v_i)==0)
          K_ij = 0;
     } else {
          double bla = tmp2 + fabs(tmp2);
          K_ij = 0.25 * bla * bla / vp1.ScalarProduct(vp1); //squared

          if (K_ij < J_EPS * J_EPS) {
               F_rep = Point(0.0, 0.0);
               return F_rep;
          }
     }
     nom = _nuPed * ped1->GetV0Norm() + v_ij; // Nu: 0=CFM, 0.28=modifCFM;
     nom *= nom;

     K_ij = sqrt(K_ij);
     if (dist_eff <= smax) { //5
          f = -ped1->GetMass() * K_ij * nom / dist_intpol_left;
          F_rep = ep12 * _maxfPed * f;
          return F_rep;
     }

     //          smax    dist_intpol_left           dist_intpol_right       dist_eff_max
     //           ----|-------------|--------------------------|--------------|----
     //           5   |     4       |            3             |      2       | 1

     if (dist_eff >= dist_intpol_right) { //2
          f = -ped1->GetMass() * K_ij * nom / dist_intpol_right; // abs(NR-Dv(i)+Sa)
          f1 = -f / dist_intpol_right;
          px = hermite_interp(dist_eff, dist_intpol_right, _distEffMaxPed, f, 0, f1, 0);
          F_rep = ep12 * px;
     } else if (dist_eff >= dist_intpol_left) { //3
          f = -ped1->GetMass() * K_ij * nom / fabs(dist_eff); // abs(NR-Dv(i)+Sa)
          F_rep = ep12 * f;
     } else {//4
          f = -ped1->GetMass() * K_ij * nom / dist_intpol_left;
          f1 = -f / dist_intpol_left;
          px = hermite_interp(dist_eff, smax, dist_intpol_left, _maxfPed*f, f, 0, f1);
          F_rep = ep12 * px;
     }
     if (F_rep._x != F_rep._x || F_rep._y != F_rep._y) {
          char tmp1[CLENGTH];
          sprintf(tmp1, "\nNAN return ----> p1=%d p2=%d Frepx=%f, Frepy=%f\n", ped1->GetID(),
                  ped2->GetID(), F_rep._x, F_rep._y);
          Log->Write(tmp1);
          Log->Write("ERROR:\t fix this as soon as possible");
          printf("K_ij=%f\n", K_ij);
          //return Point(0,0); // FIXME: should never happen
          //exit(EXIT_FAILURE);
     }
     return F_rep;
}

/* abstoßende Kraft zwischen ped und subroom
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - subroom: SubRoom für den alle abstoßende Kräfte von Wänden berechnet werden
 * Rückgabewerte:
 *   - Vektor(x,y) mit Summe aller abstoßenden Kräfte im SubRoom
 * */

inline Point KrauszModel::ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const
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
          if(obst->Contains(ped->GetPos()))
          {
               Log->Write("ERROR:\t Agent [%d] is trapped in obstacle in room/subroom [%d/%d]",ped->GetID(),subroom->GetRoomID(), subroom->GetSubRoomID());
               exit(EXIT_FAILURE);
          }
          else
               for(const auto & wall: obst->GetAllWalls())
               {
                    f += ForceRepWall(ped, wall);
               }
     }

     // and finally the closed doors
     for(auto & goal: subroom->GetAllTransitions())
     {
          if(! goal->IsOpen())
          {
               f +=  ForceRepWall(ped,*(static_cast<Line*>(goal)));
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

/* Oscillation acceleration for modelling lateral swaying
 * Parameters:
 *   - ped: Pedestrian whose AccelOscil is calculated
 * returns:
 *   - vector(x,y) of the acceleration
 * */
inline Point KrauszModel::AccelOscil(Pedestrian* ped) const
{
     double v = ped->GetV().Norm();
     double omega = 2*M_PI*OscilFreq(ped->GetSwayFreqA(), ped->GetSwayFreqB(), v);
     //omega^2 A sin(omega t)
     double amplitude = OscilAmp(ped->GetSwayAmpA(), ped->GetSwayAmpB(), v);
     double accel = omega*omega * amplitude * sin(omega*ped->GetGlobalTime()); //TODO: inclue phase of oscillations
     return v == 0 ? 0 : Point(-ped->GetV()._y, ped->GetV()._x) / v * accel;
}


inline double KrauszModel::OscilFreq(double a, double b, double v) const
{
     return a*v +b;
}

inline double KrauszModel::OscilAmp(double a, double b, double v) const
{
     double A_ = a*v + b;
     return A_ > 0 ? A_ : 0;
}


inline Point KrauszModel::ForceRepWall(Pedestrian* ped, const Line& w) const
{
     Point F = Point(0.0, 0.0);
     Point pt = w.ShortestPoint(ped->GetPos());
     double wlen = w.LengthSquare();

     if (wlen < 0.01) { // ignore walls smaller than 10 cm
          return F;
     }
     // Kraft soll nur orthgonal wirken
     // ???
     if (fabs((w.GetPoint1() - w.GetPoint2()).ScalarProduct(ped->GetPos() - pt)) > J_EPS) {
          return F;
     }
     //double mind = ped->GetEllipse().MinimumDistanceToLine(w);
     double mind = 0.5; //for performance reasons this distance is assumed to be constant
     double vn = w.NormalComp(ped->GetV()); //normal component of the velocity on the wall
     F = ForceRepStatPoint(ped, pt, mind, vn);

     if(ped->GetID() == -33 )
     {
          printf("wall = [%f, %f]--[%f, %f] F= [%f %f]\n", w.GetPoint1()._x,  w.GetPoint1()._y, w.GetPoint2()._x, w.GetPoint2()._y, F._x, F._y);
     }

     return  F; //line --> l != 0

}

/* abstoßende Punktkraft zwischen ped und Punkt p
 * Parameter:
 *   - ped: Fußgänger für den die Kraft berechnet wird
 *   - p: Punkt von dem die Kaft wirkt
 *   - l: Parameter zur Käfteinterpolation
 *   - vn: Parameter zur Käfteinterpolation
 * Rückgabewerte:
 *   - Vektor(x,y) mit abstoßender Kraft
 * */
//TODO: use effective DistanceToEllipse and simplify this function.
Point KrauszModel::ForceRepStatPoint(Pedestrian* ped, const Point& p, double l, double vn) const
{
     Point F_rep = Point(0.0, 0.0);
     const Point& v = ped->GetV();
     Point dist = p - ped->GetPos(); // x- and y-coordinate of the distance between ped and p
     double d = dist.Norm(); // distance between the centre of ped and point p
     Point e_ij; // x- and y-coordinate of the normalized vector between ped and p

     double tmp;
     double bla;
     Point r;
     Point pinE; // vorher x1, y1
     const JEllipse& E = ped->GetEllipse();

     if (d < J_EPS )
          return Point(0.0, 0.0);
     e_ij = dist / d;
     tmp = v.ScalarProduct(e_ij); // < v_i , e_ij >;
     bla = (tmp + fabs(tmp));
     if (!bla) // Fussgaenger nicht im Sichtfeld
          return Point(0.0, 0.0);
     if (fabs(v._x) < J_EPS && fabs(v._y) < J_EPS) // v==0)
          return Point(0.0, 0.0);
     double K_ij;
     K_ij= 0.5 * bla / v.Norm(); // K_ij
     // Punkt auf der Ellipse
     pinE = p.TransformToEllipseCoordinates(E.GetCenter(), E.GetCosPhi(), E.GetSinPhi());
     // Punkt auf der Ellipse
     r = E.PointOnEllipse(pinE);
     //interpolierte Kraft

     // double a = 6., b= 25.;
     // double dist_eff = d - (r - E.GetCenter()).Norm();

     // if(ped->GetID() == -9 )
     //      printf("dist=%f\n", dist_eff);


     // F_rep = e_ij* (-sigmoid(a, b, dist_eff ));

     F_rep = ForceInterpolation(ped->GetV0Norm(), K_ij, e_ij, vn, d, (r - E.GetCenter()).Norm(), l);
     return F_rep;
}

Point KrauszModel::ForceInterpolation(double v0, double K_ij, const Point& e, double vn, double d, double r, double l) const
{
     Point F_rep;
     double nominator = _nuWall * v0 + vn;
     nominator *= nominator*K_ij;
     double f = 0, f1 = 0; //function value and its derivative at the interpolation point
     //BEGIN ------- interpolation parameter
     double smax = l - _intp_widthWall; // max overlapping radius
     double dist_intpol_left = l + _intp_widthWall; //r_eps
     double dist_intpol_right = _distEffMaxWall - _intp_widthWall;
     //END ------- interpolation parameter

     double dist_eff = d - r;

     //         smax    dist_intpol_left      dist_intpol_right       dist_eff_max
     //           ----|-------------|--------------------------|--------------|----
     //       5   |     4       |            3             |      2       | 1

     double px = 0; //value of the interpolated function
     double tmp1 = _distEffMaxWall;
     double tmp2 = dist_intpol_right;
     double tmp3 = dist_intpol_left;
     double tmp5 = smax + r;

     if (dist_eff >= tmp1) { // 1
          //F_rep = Point(0.0, 0.0);
          return F_rep;
     }

     if (dist_eff <= tmp5) { // 5
          F_rep = e * (-_maxfWall);
          return F_rep;
     }

     if (dist_eff > tmp2) { //2
          f = -nominator / dist_intpol_right;
          f1 = -f / dist_intpol_right; // nominator / (dist_intpol_right^2) = derivativ of f
          px = hermite_interp(dist_eff, dist_intpol_right, _distEffMaxWall, f, 0, f1, 0);
          F_rep = e * px;
     } else if (dist_eff >= tmp3) { //3
          f = -nominator / fabs(dist_eff); //devided by abs f the effective distance
          F_rep = e * f;
     } else { //4 d > smax FIXME
          f = -nominator / dist_intpol_left;
          f1 = -f / dist_intpol_left;
          px = hermite_interp(dist_eff, smax, dist_intpol_left, _maxfWall*f, f, 0, f1);
          F_rep = e * px;
     }
     return F_rep;
}





// Getter-Funktionen

//std::shared_ptr<DirectionStrategy> KrauszModel::GetDirection() const
//{
//     return _direction;
//}
//
double KrauszModel::GetNuPed() const
{
     return _nuPed;
}

double KrauszModel::GetNuWall() const
{
     return _nuWall;
}

double KrauszModel::GetIntpWidthPed() const
{
     return _intp_widthPed;
}

double KrauszModel::GetIntpWidthWall() const
{
     return _intp_widthWall;
}

double KrauszModel::GetMaxFPed() const
{
     return _maxfPed;
}

double KrauszModel::GetMaxFWall() const
{
     return _maxfWall;
}

double KrauszModel::GetDistEffMaxPed() const
{
     return _distEffMaxPed;
}

double KrauszModel::GetDistEffMaxWall() const
{
     return _distEffMaxWall;
}

std::string KrauszModel::GetDescription()
{
     std::string rueck;
     char tmp[CLENGTH];

     sprintf(tmp, "\t\tNu: \t\tPed: %f \tWall: %f\n", _nuPed, _nuWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\tInterp. Width: \tPed: %f \tWall: %f\n", _intp_widthPed, _intp_widthWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\tMaxF: \t\tPed: %f \tWall: %f\n", _maxfPed, _maxfWall);
     rueck.append(tmp);
     sprintf(tmp, "\t\tDistEffMax: \tPed: %f \tWall: %f\n", _distEffMaxPed, _distEffMaxWall);
     rueck.append(tmp);

     return rueck;
}
