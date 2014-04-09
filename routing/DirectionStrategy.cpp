/**
 * File:   DirectionStrategy.cpp
 *
 * Created on 13. Dezember 2010, 17:02
 *@section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */
#include "../geometry/Building.h"
#include "DirectionStrategy.h"
#include "../geometry/NavLine.h"
#include "../geometry/Room.h"
#include "../pedestrian/Pedestrian.h"

DirectionStrategy::DirectionStrategy() {
}

DirectionStrategy::DirectionStrategy(const DirectionStrategy& orig) {
}

DirectionStrategy::~DirectionStrategy() {
}

Point DirectionMiddlePoint::GetTarget(Room* room, Pedestrian* ped) const {
    return (ped->GetExitLine()->GetPoint1() + ped->GetExitLine()->GetPoint2())*0.5;
}

Point DirectionMinSeperation::GetTarget(Room* room, Pedestrian* ped) const {
    return ped->GetExitLine()->ShortestPoint(ped->GetPos());
}

Point DirectionMinSeperationShorterLine::GetTarget(Room* room, Pedestrian* ped) const {

    double d = 0.2; // beide Seiten um 20 cm verkürzen

    const Point& p1 = ped->GetExitLine()->GetPoint1();
    const Point& p2 = ped->GetExitLine()->GetPoint2();
    Point diff = (p1 - p2).Normalized() * d;
    Line e_neu = Line(p1 - diff, p2 + diff);

    // kürzester Punkt auf der Linie
    return  e_neu.ShortestPoint(ped->GetPos());

}

Point DirectionInRangeBottleneck::GetTarget(Room* room, Pedestrian* ped) const {
    const Point& p1 = ped->GetExitLine()->GetPoint1();
    const Point& p2 = ped->GetExitLine()->GetPoint2();
	Line ExitLine = Line(p1, p2);
	Point Lot = ExitLine.LotPoint( ped->GetPos() );
	Point ExitMiddle = (p1+p2)*0.5;
	double d = 0.05;
	Point diff = (p1 - p2).Normalized() * d;
    Line e_neu = Line(p1 - diff, p2 + diff);


	if ( e_neu.IsInLineSegment(Lot) )
	{
		return Lot;
	}
	else
	{
		return ExitMiddle;
	}

}


// this strategy should work without Hlines for a general geometry.
Point DirectionGeneral::GetTarget(Room* room, Pedestrian* ped) const {
using namespace std;
    const Point& p1 = ped->GetExitLine()->GetPoint1();
    const Point& p2 = ped->GetExitLine()->GetPoint2();
    Line ExitLine = Line(p1, p2);
    Point Lot = ExitLine.LotPoint( ped->GetPos() );
    double d = 0.2; //shorten the line by  20 cm
    Point diff = (p1 - p2).Normalized() * d;
    Line e_neu = Line(p1 - diff, p2 + diff);

    // kürzester Punkt auf der Linie
    Point NextPointOnLine =  e_neu.ShortestPoint(ped->GetPos());

    Line tmpDirection = Line(ped->GetPos(), NextPointOnLine );//This direction will be rotated if 
    //printf("nextPointOn Line: %f %f\n", NextPointOnLine.GetX(), NextPointOnLine.GetY());
//it intersect a wall/obstacle.
// check for intersection with walls
//todo: make a FUNCTION of this
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
        // printf("Check wall %d. Dist = %f (%f)\n", i, dist, minDist);
        // printf("%f    %f --- %f    %f\n===========\n",walls[i].GetPoint1().GetX(),walls[i].GetPoint1().GetY(), walls[i].GetPoint2().GetX(),walls[i].GetPoint2().GetY());
        if (dist < minDist)
        {
            inear = i;
            minDist = dist;
        }
    }//walls
    //============================ WALLS ===========================
    
    //============================ OBST ===========================
    const vector<Obstacle*>& obstacles = subroom->GetAllObstacles();
    for(unsigned int obs=0; obs<obstacles.size(); ++obs){
        const vector<Wall>& owalls = obstacles[obs]->GetAllWalls();
        for (unsigned int i = 0; i < owalls.size(); i++) {
            dist = tmpDirection.GetIntersectionDistance(owalls[i]);
            printf("Check OBS:obs=%d, i=%d Dist = %f (%f)\n", obs, i, dist, minDist);
            if (dist < minDist)
            {
                inear = i;
                minDist = dist;
                iObs = obs;
            }
        }//walls of obstacle
    }// obstacles
    //============================ OBST ===========================


    double angle = 0;
    if (inear >= 0)
    { 
        if(iObs >= 0)
        {
            const vector<Wall>& owalls = obstacles[iObs]->GetAllWalls();
            angle =  tmpDirection.GetAngle(owalls[inear]);
            
        }
        else
            angle =  tmpDirection.GetAngle(walls[inear]);
    }
////////////////////////////////////////////////////////////
//    printf("inear=%d, iObs=%d, minDist=%f\n", inear, iObs, minDist);    
    Point  G;
    if (fabs(angle) > J_EPS)
        //G  =  tmpDirection.GetPoint2().Rotate(cos(angle), sin(angle)) ;
        G  = (NextPointOnLine-ped->GetPos()).Rotate(cos(angle), sin(angle))+ped->GetPos() ;
    else
        //G  =  tmpDirection.GetPoint2();
        G  =  NextPointOnLine;
    // printf("PED=%d\n",  ped->GetID());
    // printf ("MC Posx = %.2f, Posy=%.2f, Lot=[%.2f, %.2f]\n", ped->GetPos().GetX(), ped->GetPos().GetY(), NextPointOnLine.GetX(), NextPointOnLine.GetY());
    // printf("MC p1=[%.2f, %.2f] p2=[%.2f, %.2f]\n", p1.GetX(), p1.GetY(),  p2.GetX(), p2.GetY());
    // printf("angle=%f, G=[%.2f, %.2f]\n", angle, G.GetX(), G.GetY());

    fprintf(stderr, "%.2f %.2f %.2f %.2f %f %f %d\n", NextPointOnLine.GetX(), NextPointOnLine.GetY(), ped->GetPos().GetX(), ped->GetPos().GetY(), G.GetX(), G.GetY(), ped->GetID());

    //if(angle)
        //     getc(stdin);
    return G;
}
