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
