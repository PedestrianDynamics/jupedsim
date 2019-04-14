//
// Created by Tobias Schrödter on 2019-04-14.
//

#include "DirectionInRangeBottleneck.h"

#include "../../../geometry/Line.h"
#include "../../../geometry/NavLine.h"
#include "../../../geometry/Building.h"
#include "../../../pedestrian/Pedestrian.h"
#include "../../../geometry/SubRoom.h"

/// 3
Point DirectionInRangeBottleneck::GetTarget(Room* room, Pedestrian* ped) const
{
     UNUSED(room); // suppress the unused warning

     const Point& p1 = ped->GetExitLine()->GetPoint1();
     const Point& p2 = ped->GetExitLine()->GetPoint2();
     Line ExitLine = Line(p1, p2, 0);
     Point Lot = ExitLine.LotPoint( ped->GetPos() );
     Point ExitMiddle = (p1+p2)*0.5;
     double d = 0.2;
     // fix when using triangulation to avoid steering too near to the walls
     // double lenSq = ExitLine.LengthSquare();
     // if (lenSq >= 19 && lenSq < 50)
     //   d = 1;
     // else if(lenSq >= 50 &&lenSq < 100)
     //   d = 2;
     // else if(lenSq >= 100)
     //   d = 3;


     Point diff = (p1 - p2).Normalized() * d;
     Line e_neu = Line(p1 - diff, p2 + diff, 0);

     // if(ped->GetID() == )
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
