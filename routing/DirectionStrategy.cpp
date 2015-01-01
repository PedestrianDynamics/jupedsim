/**
 * \file        DirectionStrategy.cpp
 * \date        Dec 13, 2010
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
 *
 *
 **/


#include "../geometry/Line.h"
#include "../geometry/NavLine.h"
#include "../geometry/Building.h"
#include "../geometry/Room.h"
#include "../pedestrian/Pedestrian.h"
#include "../geometry/SubRoom.h"
#include "../geometry/Wall.h"
#include "DirectionStrategy.h"



DirectionStrategy::DirectionStrategy()
{
}


DirectionStrategy::~DirectionStrategy()
{
}
/// 1
Point DirectionMiddlePoint::GetTarget(Room* room, Pedestrian* ped) const
{
    (void)room; // suppress the unused warning
    return (ped->GetExitLine()->GetPoint1() + ped->GetExitLine()->GetPoint2())*0.5;
}
/// 2
Point DirectionMinSeperationShorterLine::GetTarget(Room* room, Pedestrian* ped) const
{
    (void)room; // suppress the unused warning

     double d = ped->GetEllipse().GetBmin() + 0.1 ; // shoulder//0.5;
     const Point& p1 = ped->GetExitLine()->GetPoint1();
     const Point& p2 = ped->GetExitLine()->GetPoint2();
     if(p1 == p2) return p1;

     double length = (p1 - p2).Norm(); 
     if(d >= 0.5*length) return (p1 + p2)*0.5; // return the middle point, since line is anyway too short
     double u = d/length; // d is supposed to be smaller than length, then u is in [0, 1]
     //Point diff = (p1 - p2).Normalized() * d;
     Line e_neu = Line(p1 + (p2-p1)*u, p1 + (p2-p1)*(1-u));
     Point target = e_neu.ShortestPoint(ped->GetPos());
     // if(ped->GetID() == 81)
     // {
      // printf("=======\nX=[%.2f], Y=[%.2f]\n", ped->GetPos().GetX(), ped->GetPos().GetY());
      //     printf("p1=[%.2f, %.2f], p2=[%.2f, %.2f]\n", p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY());
      //     printf("u= %f length=%f\n", u, length);
      //     printf("e_neu=[%.2f, %.2f]===[%.2f, %.2f]\n", e_neu.GetPoint1().GetX(), e_neu.GetPoint1().GetY(), e_neu.GetPoint2().GetX(), e_neu.GetPoint2().GetY() );

     // printf("target=[%.2f, %.2f]\n====\n\n", target.GetX(), target.GetY());
     //getc(stdin);
     //}
     // kürzester Punkt auf der Linie

     return  target;

}
/// 3
Point DirectionInRangeBottleneck::GetTarget(Room* room, Pedestrian* ped) const
{
   (void)room; // suppress the unused warning

    const Point& p1 = ped->GetExitLine()->GetPoint1();
    const Point& p2 = ped->GetExitLine()->GetPoint2();
    Line ExitLine = Line(p1, p2);
    Point Lot = ExitLine.LotPoint( ped->GetPos() );
    Point ExitMiddle = (p1+p2)*0.5;
    double d = 0.2;
    Point diff = (p1 - p2).Normalized() * d;
    Line e_neu = Line(p1 - diff, p2 + diff);

    // if(ped->GetID() == -10)
    // {
    //     printf("=======\nX=[%.2f], Y=[%.2f]\n", ped->GetPos().GetX(), ped->GetPos().GetY());
    //     printf("p1=[%.2f, %.2f], p2=[%.2f, %.2f]\n", p1.GetX(), p1.GetY(), p2.GetX(), p2.GetY());
    //     printf("e_neu=[%.2f, %.2f]===[%.2f, %.2f]\n", e_neu.GetPoint1().GetX(), e_neu.GetPoint1().GetY(), e_neu.GetPoint2().GetX(), e_neu.GetPoint2().GetY() );
    // }

    if ( e_neu.IsInLineSegment(Lot) ) {
        // if(ped->GetID() == -10){
        //     printf("Return Lot=[%.2f, %.2f]\n", Lot.GetX(), Lot.GetY() );
        //     if(0 && ped->GetPos().GetX() > 56)
        //         getc(stdin);}
        return Lot;
    } else {
        // if(ped->GetID() == -10){
        //     printf("Return Middle=[%.2f, %.2f]\n", ExitMiddle.GetX(), ExitMiddle.GetY() );
        //     if(0 && ped->GetPos().GetX() > 56)
        //         getc(stdin);}
        return ExitMiddle;
    }

}


/**
 * this strategy is designed to work without Hlines for a general geometry.
 * First tested for bottlenecks and corners.
 * number 4
 * @param room Pointer
 * @param ped Pointer to Pedestrians
 *
 * @todo Need more tests e.g. for complex geometries.
 * @todo Need refactoring: Put the WALL and OBS loops in appropriate functions
 * @return Target (Point)
 */Point DirectionGeneral::GetTarget(Room* room, Pedestrian* ped) const
{
#define DEBUG 0
     using namespace std;
     const Point& p1 = ped->GetExitLine()->GetPoint1();
     const Point& p2 = ped->GetExitLine()->GetPoint2();
     Line ExitLine = Line(p1, p2);
     //Point Lot = ExitLine.LotPoint( ped->GetPos() );
     double d = 0.2; //shorten the line by  20 cm
     Point diff = (p1 - p2).Normalized() * d;
     Line e_neu = Line(p1 - diff, p2 + diff);
     Point NextPointOnLine =  e_neu.ShortestPoint(ped->GetPos());

     Line tmpDirection = Line(ped->GetPos(), NextPointOnLine );//This direction will be rotated if
                   // it intersects a wall || obstacle.
                   // check for intersection with walls
                   // @todo: make a FUNCTION of this

#if DEBUG
          printf("\n----------\nEnter GetTarget() with PED=%d\n----------\n",ped->GetID());
          printf("nextPointOn Line: %f %f\n", NextPointOnLine.GetX(), NextPointOnLine.GetY());
#endif
     double dist;
     int inear = -1;
     int iObs = -1;
     double minDist = 20001;
     int subroomId = ped->GetSubRoomID();
     SubRoom * subroom = room->GetSubRoom(subroomId);

     //============================ WALLS ===========================
     const vector<Wall>& walls = subroom->GetAllWalls();
     for (int i = 0; i < subroom->GetNumberOfWalls(); i++) {
          dist = tmpDirection.GetIntersectionDistance(walls[i]);
          if (dist < minDist) {
               inear = i;
               minDist = dist;
#if DEBUG
                    printf("Check wall %d. Dist = %f (%f)\n", i, dist, minDist);
                    printf("%f    %f --- %f    %f\n===========\n",walls[i].GetPoint1().GetX(),walls[i].GetPoint1().GetY(), walls[i].GetPoint2().GetX(),walls[i].GetPoint2().GetY());
#endif
 
          }
     }//walls
     //============================ WALLS ===========================

     //============================ OBST ===========================
     const vector<Obstacle*>& obstacles = subroom->GetAllObstacles();
     for(unsigned int obs=0; obs<obstacles.size(); ++obs) {
          const vector<Wall>& owalls = obstacles[obs]->GetAllWalls();
          for (unsigned int i = 0; i < owalls.size(); i++) {
               dist = tmpDirection.GetIntersectionDistance(owalls[i]);
               if (dist < minDist) {
                    inear = i;
                    minDist = dist;
                    iObs = obs;

                         // printf("Check OBS:obs=%d, i=%d Dist = %f (%f)\n", obs, i, dist, minDist);
                         // printf("%f    %f --- %f    %f\n===========\n",owalls[i].GetPoint1().GetX(),owalls[i].GetPoint1().GetY(), owalls[i].GetPoint2().GetX(),owalls[i].GetPoint2().GetY());

               }
          }//walls of obstacle
     }// obstacles
     //============================ OBST ===========================


     double angle = 0;
     if (inear >= 0) {
          ped->SetNewOrientationFlag(true); //Mark this pedestrian for next target calculation
          ped->SetDistToBlockade(minDist);
          if(iObs >= 0) {
               const vector<Wall>& owalls = obstacles[iObs]->GetAllWalls();
               angle =  tmpDirection.GetDeviationAngle(owalls[inear].enlarge(2*ped->GetLargerAxis()));
#if DEBUG
                    printf("COLLISION WITH %f    %f --- %f    %f\n===========\n",owalls[inear].GetPoint1().GetX(),owalls[inear].GetPoint1().GetY(), owalls[inear].GetPoint2().GetX(),owalls[inear].GetPoint2().GetY());
               
#endif 

          } else{
               angle =  tmpDirection.GetDeviationAngle(walls[inear].enlarge(2*ped->GetLargerAxis()));
#if DEBUG
                    printf("COLLISION WITH %f    %f --- %f    %f\n===========\n",walls[inear].GetPoint1().GetX(),walls[inear].GetPoint1().GetY(), walls[inear].GetPoint2().GetX(),walls[inear].GetPoint2().GetY());
#endif
          }
     }//inear
     else{

          if(ped->GetNewOrientationFlag()){ //this pedestrian could not see the target and now he can see it clearly.
                         // printf("ped->GetNewOrientationFlag()=%d\n",ped->GetNewOrientationFlag());getc(stdin);
               ped->SetSmoothTurning(); // so the turning should be adapted accordingly.
               ped->SetNewOrientationFlag(false);
          }
     }
////////////////////////////////////////////////////////////
    
     Point  G;
     if (fabs(angle) > J_EPS)
          //G  =  tmpDirection.GetPoint2().Rotate(cos(angle), sin(angle)) ;
          G  = (NextPointOnLine-ped->GetPos()).Rotate(cos(angle), sin(angle))+ped->GetPos() ;
     else {
          if(ped->GetNewOrientationFlag()) //this pedestrian could not see the target and now he can see it clearly.
               ped->SetSmoothTurning(); // so the turning should be adapted accordingly.
          
          G  =  NextPointOnLine;
     }
#if DEBUG
          printf("inear=%d, iObs=%d, minDist=%f\n", inear, iObs, minDist);
          printf("PED=%d\n",  ped->GetID());
          printf ("MC Posx = %.2f, Posy=%.2f, Lot=[%.2f, %.2f]\n", ped->GetPos().GetX(), ped->GetPos().GetY(), NextPointOnLine.GetX(), NextPointOnLine.GetY());
          printf("MC p1=[%.2f, %.2f] p2=[%.2f, %.2f]\n", p1.GetX(), p1.GetY(),  p2.GetX(), p2.GetY());
          printf("angle=%f, G=[%.2f, %.2f]\n", angle, G.GetX(), G.GetY());
          printf("\n----------\nLEAVE function with PED=%d\n----------\n",ped->GetID());

#endif

          // if( ped->GetID() == 12)
          // fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f %d %.2f %.2f %.2f\n", NextPointOnLine.GetX(), NextPointOnLine.GetY(), 
// ped->GetPos().GetX(), ped->GetPos().GetY(), G.GetX(), G.GetY(), ped->GetID(), ped->GetV0().GetX(), ped->GetV0().GetY(), ped->GetGlobalTime());
// this stderr output can be used with plot_desired_velocity.py

     return G;
}













