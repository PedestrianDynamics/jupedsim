/*
 * File:   DirectionStrategy.cpp
 * Author: andrea
 *
 * Created on 13. Dezember 2010, 17:02
 */

#include "DirectionStrategy.h"

DirectionStrategy::DirectionStrategy() {
}

DirectionStrategy::DirectionStrategy(const DirectionStrategy& orig) {
}

DirectionStrategy::~DirectionStrategy() {
}

Point DirectionMiddlePoint::GetTarget(Room* room, Pedestrian* ped) const {

    //The pedestrian knows which exit it wants.
    Line* exitLine = ped->GetExitLine();
    return (exitLine->GetPoint1() + exitLine->GetPoint2())*0.5;
}

Point DirectionMinSeperation::GetTarget(Room* room, Pedestrian* ped) const {
    Line* exitLine = ped->GetExitLine();
    return exitLine->ShortestPoint(ped->GetPos());
}

Point DirectionMinSeperationShorterLine::GetTarget(Room* room, Pedestrian* ped) const {
    Line* exitLine = ped->GetExitLine();

    double d = 0.2; // beide Seiten um 20 cm verkürzen

    if(exitLine->Length()>4.0){
    	d=0.7;
    }

    const Point& p1 = exitLine->GetPoint1();
    const Point& p2 = exitLine->GetPoint2();
    Point diff = (p1 - p2).Normalized() * d;
    Line e_neu = Line(p1 - diff, p2 + diff);

    // kürzester Punkt auf der Linie
    return  e_neu.ShortestPoint(ped->GetPos());

}
