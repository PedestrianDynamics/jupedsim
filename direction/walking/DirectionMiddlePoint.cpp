//
// Created by Tobias Schrödter on 2019-04-14.
//

#include "DirectionMiddlePoint.h"
#define UNUSED(x) [&x]{}()  // c++11 silence warnings

#include "geometry/Line.h"
#include "geometry/NavLine.h"
#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"
#include "geometry/SubRoom.h"

Point DirectionMiddlePoint::GetTarget(Room* room, Pedestrian* ped) const
{
     UNUSED(room); // suppress the unused warning
     return (ped->GetExitLine()->GetPoint1() + ped->GetExitLine()->GetPoint2())*0.5;
}
