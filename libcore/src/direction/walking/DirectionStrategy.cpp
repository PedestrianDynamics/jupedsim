/**
 * \file        DirectionStrategy.cpp
 * \date        Dec 13, 2010
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
 *
 *
 **/
#include "DirectionStrategy.hpp"

#include "geometry/Line.hpp"
#include "geometry/Point.hpp"
#include "geometry/Room.hpp"
#include "geometry/SubRoom.hpp"
#include "geometry/Wall.hpp"
#include "pedestrian/Pedestrian.hpp"
#include "routing/ff_router/UnivFFviaFM.hpp"

#include <Logger.hpp>
#include <chrono>

/// 1
Point DirectionMiddlePoint::GetTarget(Room * /*room*/, Pedestrian * ped) const
{
    return (ped->GetExitLine().GetPoint1() + ped->GetExitLine().GetPoint2()) * 0.5;
}

/// 2
Point DirectionMinSeperationShorterLine::GetTarget(Room * /*room*/, Pedestrian * ped) const
{
    double d         = ped->GetEllipse().GetBmin() + 0.1; // shoulder//0.5;
    const Point & p1 = ped->GetExitLine().GetPoint1();
    const Point & p2 = ped->GetExitLine().GetPoint2();

    if(p1 == p2) {
        return p1;
    }

    double length = (p1 - p2).Norm();
    if(d >= 0.5 * length) {
        return (p1 + p2) * 0.5; // return the middle point, since line is anyway too short
    }
    double u     = d / length; // d is supposed to be smaller than length, then u is in [0, 1]
    Line e_neu   = Line(p1 + (p2 - p1) * u, p1 + (p2 - p1) * (1 - u), 0);
    Point target = e_neu.ShortestPoint(ped->GetPos());

    return target;
}

/// 3
Point DirectionInRangeBottleneck::GetTarget(Room * /*room*/, Pedestrian * ped) const
{
    const Point & p1 = ped->GetExitLine().GetPoint1();
    const Point & p2 = ped->GetExitLine().GetPoint2();
    Line ExitLine    = Line(p1, p2, 0);
    Point Lot        = ExitLine.LotPoint(ped->GetPos());
    Point ExitMiddle = (p1 + p2) * 0.5;
    double d         = 0.2;

    Point diff = (p1 - p2).Normalized() * d;
    Line e_neu = Line(p1 - diff, p2 + diff, 0);

    if(e_neu.IsInLineSegment(Lot)) {
        return Lot;
    } else {
        return ExitMiddle;
    }
}

/// 8
Point DirectionLocalFloorfield::GetTarget(Room * room, Pedestrian * ped) const
{
    Point p;
    UnivFFviaFM * floorfield = _locffviafm.at(room->GetID());

    floorfield->GetDirectionToUID(ped->GetDestination(), ped->GetPos(), p);

    return (p + ped->GetPos());
}

Point DirectionLocalFloorfield::GetDir2Wall(Pedestrian * ped) const
{
    Point p;
    auto [roomID, _1, _2] = _building->GetRoomAndSubRoomIDs(ped->GetPos());
    _locffviafm.at(roomID)->GetDir2WallAt(ped->GetPos(), p);
    return p;
}

double DirectionLocalFloorfield::GetDistance2Wall(Pedestrian * ped) const
{
    auto [roomID, _1, _2] = _building->GetRoomAndSubRoomIDs(ped->GetPos());
    return _locffviafm.at(roomID)->GetDistance2WallAt(ped->GetPos());
}

double DirectionLocalFloorfield::GetDistance2Target(Pedestrian * ped, int UID) const
{
    auto [roomID, _1, _2] = _building->GetRoomAndSubRoomIDs(ped->GetPos());
    return _locffviafm.at(roomID)->GetCostToDestination(UID, ped->GetPos());
}

void DirectionLocalFloorfield::Init(Building * building, const Configuration & config)
{
    _wasInitialized    = true;
    _building          = building;
    _stepsize          = config.deltaH;
    _wallAvoidDistance = config.wallAvoidDistance;
    _useDistancefield  = config.useWallAvoidance;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for(auto & roomPair : _building->GetAllRooms()) {
        auto newfield = new UnivFFviaFM(
            roomPair.second.get(), _building, _stepsize, _wallAvoidDistance, _useDistancefield);
        _locffviafm[roomPair.first] = newfield;
        newfield->SetUser(DISTANCE_AND_DIRECTIONS_USED);
        newfield->SetMode(LINESEGMENT);
        if(_useDistancefield) {
            newfield->SetSpeedMode(FF_WALL_AVOID);
        } else {
            newfield->SetSpeedMode(FF_HOMO_SPEED);
        }
        newfield->AddAllTargetsParallel();
    }

    end                                           = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    LOG_INFO("Time to construct FF in DirectionLocalFloorfield: {:.2f}", elapsed_seconds.count());
}

DirectionLocalFloorfield::~DirectionLocalFloorfield()
{
    for(auto pair : _locffviafm) {
        delete pair.second;
    }
}
