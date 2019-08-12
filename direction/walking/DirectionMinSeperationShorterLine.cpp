//
// Created by Tobias Schrödter on 2019-04-14.
//

#include "DirectionMinSeperationShorterLine.h"

#include "geometry/Line.h"
#include "geometry/NavLine.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/SubRoom.h"

/// 2
Point DirectionMinSeperationShorterLine::GetTarget(Room* , Pedestrian* ped) const
{
     double d = ped->GetEllipse().GetBmin() + 0.1 ; // shoulder//0.5;
     const Point& p1 = ped->GetExitLine()->GetPoint1();
     const Point& p2 = ped->GetExitLine()->GetPoint2();
     if(p1 == p2) return p1;

     double length = (p1 - p2).Norm();
     if(d >= 0.5*length) return (p1 + p2)*0.5; // return the middle point, since line is anyway too short
     double u = d/length; // d is supposed to be smaller than length, then u is in [0, 1]
     //Point diff = (p1 - p2).Normalized() * d;
     Line e_neu = Line(p1 + (p2-p1)*u, p1 + (p2-p1)*(1-u), 0);
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
