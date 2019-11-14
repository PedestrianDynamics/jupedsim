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
#include "DirectionStrategy.h"

#include "general/Format.h"
#include "general/Logger.h"
#include "geometry/Line.h"
#include "geometry/Point.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "geometry/Wall.h"
#include "pedestrian/Pedestrian.h"
#include "routing/ff_router/FloorfieldViaFM.h"
#include "routing/ff_router/UnivFFviaFM.h"

#include <chrono>

/// 1
Point DirectionMiddlePoint::GetTarget(Room * /*room*/, Pedestrian * ped) const
{
    return (ped->GetExitLine()->GetPoint1() + ped->GetExitLine()->GetPoint2()) * 0.5;
}

/// 2
Point DirectionMinSeperationShorterLine::GetTarget(Room * /*room*/, Pedestrian * ped) const
{
    double d         = ped->GetEllipse().GetBmin() + 0.1; // shoulder//0.5;
    const Point & p1 = ped->GetExitLine()->GetPoint1();
    const Point & p2 = ped->GetExitLine()->GetPoint2();

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
    const Point & p1 = ped->GetExitLine()->GetPoint1();
    const Point & p2 = ped->GetExitLine()->GetPoint2();
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


/// 4
Point DirectionGeneral::GetTarget(Room * room, Pedestrian * ped) const
{
    const Point & p1      = ped->GetExitLine()->GetPoint1();
    const Point & p2      = ped->GetExitLine()->GetPoint2();
    Line ExitLine         = Line(p1, p2, 0);
    double d              = 0.2; //shorten the line by  20 cm
    Point diff            = (p1 - p2).Normalized() * d;
    Line e_neu            = Line(p1 - diff, p2 + diff, 0);
    Point NextPointOnLine = e_neu.ShortestPoint(ped->GetPos());

    Line tmpDirection = Line(ped->GetPos(), NextPointOnLine, 0); //This direction will be rotated if
    // it intersects a wall || obstacle.
    // check for intersection with walls
    // @todo: make a FUNCTION of this

    Logging::Debug(fmt::format(check_fmt("Enter GetTarget() with PED = {}."), ped->GetID()));
    Logging::Debug(fmt::format(
        check_fmt("nextPointOn Line: {:.2f} {:.2f}."), NextPointOnLine._x, NextPointOnLine._y));

    double dist;
    unsigned long inear = std::numeric_limits<unsigned long>::max();
    unsigned long iObs  = std::numeric_limits<unsigned long>::max();
    double minDist      = 20001;
    int subroomId       = ped->GetSubRoomID();
    SubRoom * subroom   = room->GetSubRoom(subroomId);

    //============================ WALLS ===========================
    const std::vector<Wall> & walls = subroom->GetAllWalls();
    for(unsigned long i = 0; i < walls.size(); i++) {
        dist = tmpDirection.GetDistanceToIntersectionPoint(walls[i]);
        if(dist < minDist) {
            inear   = i;
            minDist = dist;
            Logging::Debug(fmt::format(
                check_fmt("Check wall number {:d}. Dist = {:.2f} ({:.2f})."), i, dist, minDist));
            Logging::Debug(fmt::format(
                check_fmt("{:.2f} {:.2f} --- {:.2f} {:.2f}."),
                walls[i].GetPoint1()._x,
                walls[i].GetPoint1()._y,
                walls[i].GetPoint2()._x,
                walls[i].GetPoint2()._y));
        }
    } //walls
    //============================ WALLS ===========================

    //============================ OBST ===========================
    const std::vector<Obstacle *> & obstacles = subroom->GetAllObstacles();
    for(unsigned long obs = 0; obs < obstacles.size(); ++obs) {
        const std::vector<Wall> & owalls = obstacles[obs]->GetAllWalls();
        for(unsigned long i = 0; i < owalls.size(); i++) {
            dist = tmpDirection.GetDistanceToIntersectionPoint(owalls[i]);
            if(dist < minDist) {
                inear   = i;
                minDist = dist;
                iObs    = obs;
                Logging::Debug(fmt::format(
                    check_fmt("Check OBS: obs={:d},  {:d}. Dist = {:.2f} ({:.2f})."),
                    obs,
                    i,
                    dist,
                    minDist));
                Logging::Debug(fmt::format(
                    check_fmt("{:.2f} {:.2f} --- {:.2f} {:.2f}."),
                    owalls[i].GetPoint1()._x,
                    owalls[i].GetPoint1()._y,
                    owalls[i].GetPoint2()._x,
                    owalls[i].GetPoint2()._y));
            }
        } //walls of obstacle
    }     // obstacles
    //============================ OBST ===========================

    double angle = 0;
    if(inear != std::numeric_limits<unsigned long>::max()) {
        ped->SetNewOrientationFlag(true); //Mark this pedestrian for next target calculation
        ped->SetDistToBlockade(minDist);
        if(iObs != std::numeric_limits<unsigned long>::max()) { // obstacle is nearest
            const std::vector<Wall> & owalls = obstacles[iObs]->GetAllWalls();
            angle = tmpDirection.GetObstacleDeviationAngle(owalls, walls);

            // angle =  tmpDirection.GetDeviationAngle(owalls[inear].enlarge(2*ped->GetLargerAxis()));
            Logging::Debug(fmt::format(
                check_fmt("COLLISION WITH OBSTACLE {:.2f} {:.2f} --- {:.2f} {:.2f}."),
                owalls[inear].GetPoint1()._x,
                owalls[inear].GetPoint1()._y,
                owalls[inear].GetPoint2()._x,
                owalls[inear].GetPoint2()._y));
        }      //iObs
        else { // wall is nearest
            angle = tmpDirection.GetDeviationAngle(walls[inear].Enlarge(2 * ped->GetLargerAxis()));
            Logging::Debug(fmt::format(
                check_fmt("COLLISION WITH WALL {:.2f} {:.2f} --- {:.2f} {:.2f}."),
                walls[inear].GetPoint1()._x,
                walls[inear].GetPoint1()._y,
                walls[inear].GetPoint2()._x,
                walls[inear].GetPoint2()._y));
        } //else
    }     //inear
    else {
        if(ped->GetNewOrientationFlag()) { //this pedestrian could not see the target and now he can see it clearly.
            ped->SetSmoothTurning(); // so the turning should be adapted accordingly.
            ped->SetNewOrientationFlag(false);
        }
    }
    ////////////////////////////////////////////////////////////
    Point G;
    if(fabs(angle) > J_EPS)
        //G  =  tmpDirection.GetPoint2().Rotate(cos(angle), sin(angle)) ;
        G = (NextPointOnLine - ped->GetPos()).Rotate(cos(angle), sin(angle)) + ped->GetPos();
    else {
        if(ped->GetNewOrientationFlag()) //this pedestrian could not see the target and now he can see it clearly.
            ped->SetSmoothTurning(); // so the turning should be adapted accordingly.

        G = NextPointOnLine;
    }

    Logging::Debug(
        fmt::format(check_fmt("inear={:d}, iObs={:d}, minDist={:.2f}"), inear, iObs, minDist));
    Logging::Debug(fmt::format(check_fmt("Ped={:d}"), ped->GetID()));
    Logging::Debug(fmt::format(
        check_fmt("MC Posx = {:.2f}, Posy={:.2f}, Lot=[{:.2f}, {:.2f}]"),
        ped->GetPos()._x,
        ped->GetPos()._y,
        NextPointOnLine._x,
        NextPointOnLine._y));
    Logging::Debug(fmt::format(check_fmt("angle={:.2f}, G=[{:.2f}, {:.2f}]"), angle, G._x, G._y));

    return G;
}

/// 6
Point DirectionFloorfield::GetTarget(Room *, Pedestrian * ped) const
{
    Point p;
    _ffviafm->getDirectionToDestination(ped, p);
    p = p.Normalized(); // @todo: argraf : scale with costvalue: " * ffviafm->getCostToTransition(ped->GetTransitionID(), ped->GetPos()) "
    return (p + ped->GetPos());
}

Point DirectionFloorfield::GetDir2Wall(Pedestrian * ped) const
{
    Point p;
    _ffviafm->getDir2WallAt(ped->GetPos(), p);
    return p;
}

double DirectionFloorfield::GetDistance2Wall(Pedestrian * ped) const
{
    return _ffviafm->getDistance2WallAt(ped->GetPos());
}

void DirectionFloorfield::Init(Building * building)
{
    double stepsize          = building->GetConfig()->get_deltaH();
    double wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
    bool useDistancefield    = building->GetConfig()->get_use_wall_avoidance();

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start    = std::chrono::system_clock::now();
    _ffviafm = new FloorfieldViaFM(
        building, stepsize, stepsize, wallAvoidDistance, useDistancefield, false);
    end                                           = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    Logging::Info(fmt::format(
        check_fmt("Time to construct FF in DirectionFloorfield: {:.2f}"), elapsed_seconds.count()));
}


DirectionFloorfield::~DirectionFloorfield()
{
    delete _ffviafm;
}

/// 8
Point DirectionLocalFloorfield::GetTarget(Room * room, Pedestrian * ped) const
{
    Point p;
    UnivFFviaFM * floorfield = _locffviafm.at(room->GetID());

    floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(), p);

    return (p + ped->GetPos());
}

Point DirectionLocalFloorfield::GetDir2Wall(Pedestrian * ped) const
{
    Point p;
    int roomID = ped->GetRoomID();
    _locffviafm.at(roomID)->getDir2WallAt(ped->GetPos(), p);
    return p;
}

double DirectionLocalFloorfield::GetDistance2Wall(Pedestrian * ped) const
{
    return _locffviafm.at(ped->GetRoomID())->getDistance2WallAt(ped->GetPos());
}

double DirectionLocalFloorfield::GetDistance2Target(Pedestrian * ped, int UID) const
{
    int roomID = ped->GetRoomID();
    return _locffviafm.at(roomID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionLocalFloorfield::Init(Building * building)
{
    _building          = building;
    _stepsize          = building->GetConfig()->get_deltaH();
    _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
    _useDistancefield  = building->GetConfig()->get_use_wall_avoidance();

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for(auto & roomPair : _building->GetAllRooms()) {
        auto newfield = new UnivFFviaFM(
            roomPair.second.get(), _building, _stepsize, _wallAvoidDistance, _useDistancefield);
        _locffviafm[roomPair.first] = newfield;
        newfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
        newfield->setMode(LINESEGMENT);
        if(_useDistancefield) {
            newfield->setSpeedMode(FF_WALL_AVOID);
        } else {
            newfield->setSpeedMode(FF_HOMO_SPEED);
        }
        newfield->addAllTargetsParallel();
    }

    //TODO check writing of ff (TS)
    if(_building->GetConfig()->get_write_VTK_files_direction()) {
        for(auto locff : _locffviafm) {
            int roomNr = locff.first;
            locff.second->writeFF(
                "direction" + std::to_string(roomNr) + ".vtk", locff.second->getKnownDoorUIDs());
        }
    }

    end                                           = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    Logging::Info(fmt::format(
        check_fmt("Time to construct FF in DirectionLocalFloorfield: {:.2f}"),
        elapsed_seconds.count()));
}

DirectionLocalFloorfield::~DirectionLocalFloorfield()
{
    for(auto pair : _locffviafm) {
        delete pair.second;
    }
}

///9
Point DirectionSubLocalFloorfield::GetTarget(Room *, Pedestrian * ped) const
{
    Point p;
    UnivFFviaFM * floorfield = _locffviafm.at(ped->GetSubRoomUID());

    floorfield->getDirectionToUID(ped->GetExitIndex(), ped->GetPos(), p);
    return (p + ped->GetPos());
}

Point DirectionSubLocalFloorfield::GetDir2Wall(Pedestrian * ped) const
{
    Point p;
    int key = ped->GetSubRoomUID();
    _locffviafm.at(key)->getDir2WallAt(ped->GetPos(), p);
    return p;
}

double DirectionSubLocalFloorfield::GetDistance2Wall(Pedestrian * ped) const
{
    return _locffviafm.at(ped->GetSubRoomUID())->getDistance2WallAt(ped->GetPos());
}

double DirectionSubLocalFloorfield::GetDistance2Target(Pedestrian * ped, int UID) const
{
    int subroomUID = ped->GetSubRoomUID();
    return _locffviafm.at(subroomUID)->getCostToDestination(UID, ped->GetPos());
}

void DirectionSubLocalFloorfield::Init(Building * building)
{
    _building          = building;
    _stepsize          = building->GetConfig()->get_deltaH();
    _wallAvoidDistance = building->GetConfig()->get_wall_avoid_distance();
    _useDistancefield  = building->GetConfig()->get_use_wall_avoidance();

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    for(auto & roomPair : _building->GetAllRooms()) {
        for(auto & subPair : roomPair.second->GetAllSubRooms()) {
            int subUID      = subPair.second->GetUID();
            auto floorfield = new UnivFFviaFM(
                subPair.second.get(), _building, _stepsize, _wallAvoidDistance, _useDistancefield);
            _locffviafm[subUID] = floorfield;
            floorfield->setUser(DISTANCE_AND_DIRECTIONS_USED);
            floorfield->setMode(LINESEGMENT);
            if(_useDistancefield) {
                floorfield->setSpeedMode(FF_WALL_AVOID);
            } else {
                floorfield->setSpeedMode(FF_HOMO_SPEED);
            }
            floorfield->addAllTargetsParallel();
        }
    }

    //TODO check writing of ff (TS)
    if(_building->GetConfig()->get_write_VTK_files_direction()) {
        for(auto locff : _locffviafm) {
            int roomNr = locff.first;
            locff.second->writeFF(
                "direction" + std::to_string(roomNr) + ".vtk", locff.second->getKnownDoorUIDs());
        }
    }

    end                                           = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    Logging::Info(fmt::format(
        check_fmt("Time to construct FF in DirectionSubLocalFloorfield: {:.2f}"),
        elapsed_seconds.count()));
}

DirectionSubLocalFloorfield::~DirectionSubLocalFloorfield()
{
    for(auto pair : _locffviafm) {
        delete pair.second;
    }
}

// 12
Point DirectionTrain::GetTarget(Room * /*room*/, Pedestrian * ped) const
{
    Point p1             = ped->GetExitLine()->GetPoint1();
    Point p2             = ped->GetExitLine()->GetPoint2();
    Line ExitLine        = Line(p1, p2, 0);
    auto TrainTypes      = ped->GetBuilding()->GetTrainTypes();
    auto TrainTimeTables = ped->GetBuilding()->GetTrainTimeTables();
    auto now             = Pedestrian::GetGlobalTime();
    std::string type_delme;

    for(auto && t : TrainTimeTables) {
        if(ped->GetRoomID() != t.second->rid)
            continue;

        if((now >= t.second->tin) && (now <= t.second->tout)) {
            auto doors = TrainTypes[t.second->type]->doors;
            int i = -1, imin = 0;
            double dist_min = 10000;
            for(const auto & door : doors) {
                i++;
                const Point & d1 = door.GetPoint1();
                const Point & d2 = door.GetPoint2();
                const Point & c  = (d1 + d2) * 0.5;

                double dist = (ped->GetPos() - c).Norm();

                if(dist <= dist_min) {
                    dist_min   = dist;
                    imin       = i;
                    type_delme = t.second->type;
                }
            } // doors
            p1 = doors[imin].GetPoint1();
            p2 = doors[imin].GetPoint2();

        } // if time in
    }

    return (p1 + p2) * 0.5;
}
