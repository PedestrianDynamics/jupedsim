/**
 * \copyright   <2009-2019> Forschungszentrum Jülich GmbH. All rights reserved.
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
 **/

#include "CorrectGeometry.h"

#include "general/Configuration.h"
#include "general/Filesystem.h"
#include "geometry/Building.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/Point.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <fmt/format.h>
#include <map>
#include <memory>
#include <optional>
#include <ratio>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <libshared>

namespace geometry::helper
{
bool RemoveOverlappingWall(const Line & exit, SubRoom & subroom)
{
    auto isOverlapping = [&exit](const auto & wall) {
        return wall.NearlyInLineSegment(exit.GetPoint1()) &&
               wall.NearlyInLineSegment(exit.GetPoint2());
    };

    // search for a wall overlapping with the exit
    const auto & overlappingWallIt =
        std::find_if(subroom.GetAllWalls().begin(), subroom.GetAllWalls().end(), isOverlapping);

    if(overlappingWallIt == subroom.GetAllWalls().end()) {
        return false;
    }

    // Attention: after adding Walls or removing Walls the overlappingWallIt could be invlaid!
    Wall overlappingWall = *overlappingWallIt;

    double dist_pt1 = (overlappingWall.GetPoint1() - exit.GetPoint1()).NormSquare();
    double dist_pt2 = (overlappingWall.GetPoint1() - exit.GetPoint2()).NormSquare();

    Wall w1(overlappingWall.GetPoint1(), exit.GetPoint2());
    Wall w2(overlappingWall.GetPoint2(), exit.GetPoint1());

    if(dist_pt1 < dist_pt2) {
        w1.SetPoint2(exit.GetPoint1());
        w2.SetPoint2(exit.GetPoint2());
    }

    subroom.RemoveWall(overlappingWall);
    subroom.AddWall(w1);
    subroom.AddWall(w2);

    return true;
}


bool RemoveWallsOverlappingWithDoors(SubRoom & subroom)
{
    LOG_DEBUG(
        "Enter RemoveWallsOverlappingWithDoors with SubRoom {}-{}",
        subroom.GetRoomID(),
        subroom.GetSubRoomID());

    bool wallsRemoved = false; // did we remove anything?

    for(Crossing const * const crossing : subroom.GetAllCrossings()) {
        if(RemoveOverlappingWall(*crossing, subroom)) {
            wallsRemoved = true;
        }
    }

    for(Transition const * const transition : subroom.GetAllTransitions()) {
        if(RemoveOverlappingWall(*transition, subroom)) {
            wallsRemoved = true;
        }
    }

    return wallsRemoved;
}


std::optional<Point> ComputeSplitPoint(const Wall & wall, const Line & line)
{
    // Equal lines should not be split.
    if(wall == line) {
        return std::nullopt;
    }

    Point intersectionPoint;

    // Walls without intersection should not be split.
    if(!wall.IntersectionWith(line, intersectionPoint)) {
        return std::nullopt;
    }

    // Walls with intersection at beginning or end should not be split.
    if(intersectionPoint == wall.GetPoint1() || intersectionPoint == wall.GetPoint2()) {
        return std::nullopt;
    }

    // Intersection points with NAN cannot be split, this means lines are parallel
    if(std::isnan(intersectionPoint._x) || std::isnan(intersectionPoint._y)) {
        return std::nullopt;
    }

    LOG_DEBUG(
        "BigWall {} will be split due to intersection with Line {} in {}",
        wall.toString(),
        line.toString(),
        intersectionPoint.toString());

    return {intersectionPoint};
}

std::optional<std::vector<Wall>> SplitWall(const SubRoom & subroom, const Wall & bigWall)
{
    LOG_DEBUG(
        "SplitWall for BigWall {} in SubRoom {}-{}.",
        bigWall.toString(),
        subroom.GetRoomID(),
        subroom.GetSubRoomID());

    // Collecting all split Points
    std::vector<Point> splitPoints;
    for(const auto & crossing : subroom.GetAllCrossings()) {
        if(const auto p = ComputeSplitPoint(bigWall, *crossing)) {
            splitPoints.emplace_back(*p);
        }
    }
    for(const auto & transition : subroom.GetAllTransitions()) {
        if(const auto p = ComputeSplitPoint(bigWall, *transition)) {
            splitPoints.emplace_back(*p);
        }
    }
    for(const auto & wall : subroom.GetAllWalls()) {
        if(const auto p = ComputeSplitPoint(bigWall, wall)) {
            splitPoints.emplace_back(*p);
        }
    }
    LOG_DEBUG("splitpoints: {}", splitPoints.size());

    // No split points found, bigWall cannot be split.
    if(splitPoints.empty()) {
        return std::nullopt;
    }

    // Sort the splitpoints by distance to bigWall Point1
    std::sort(
        splitPoints.begin(), splitPoints.end(), [&bigWall](const Point & p1, const Point & p2) {
            return Distance(bigWall.GetPoint1(), p1) < Distance(bigWall.GetPoint1(), p2);
        });

    std::vector<Wall> wallPieces;
    // We are not interested in all wall segments since we only want to remove outstanding segments
    if(splitPoints.size() == 1) {
        wallPieces.emplace_back(bigWall.GetPoint1(), splitPoints.front());
        wallPieces.emplace_back(splitPoints.back(), bigWall.GetPoint2());
    } else {
        wallPieces.emplace_back(bigWall.GetPoint1(), splitPoints.front());
        wallPieces.emplace_back(splitPoints.front(), splitPoints.back());
        wallPieces.emplace_back(splitPoints.back(), bigWall.GetPoint2());
    }

    return wallPieces;
}

bool IsConnectedWall(const SubRoom & subroom, const Wall & wall)
{
    return subroom.IsPointOnPolygonBoundaries(wall.GetPoint1(), wall) &&
           subroom.IsPointOnPolygonBoundaries(wall.GetPoint2(), wall);
}

int AddConnectedWallsToSubroom(SubRoom & subroom, const std::vector<Wall> & wallPieces)
{
    int wallsAdded = 0;

    for(const auto & w : wallPieces) {
        if(!subroom.HasWall(w) && IsConnectedWall(subroom, w)) {
            subroom.AddWall(w);
            wallsAdded++;
        }
    }

    return wallsAdded;
}

void ReplaceBigWall(SubRoom & subroom, const Wall & bigWall, const std::vector<Wall> & wallPieces)
{
    if(!subroom.RemoveWall(bigWall)) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("Correcting the geometry failed. Could not remove wall: {}"),
            bigWall.toString()));
    }

    if(AddConnectedWallsToSubroom(subroom, wallPieces) <= 0) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("Correcting the geometry failed. Could not add any wall from splitted big "
                       "wall: {}"),
            bigWall.toString()));
    }
}

bool RemoveBigWalls(SubRoom & subroom)
{
    bool wallsRemoved = false;

    // Need to copy walls here, because we are going to change subrooms wall container
    const auto walls = subroom.GetAllWalls();

    for(auto const & bigWall : walls) {
        const auto splitWallPieces = SplitWall(subroom, bigWall);

        if(splitWallPieces) {
            assert(!splitWallPieces->empty());
            LOG_DEBUG(
                "Splitting big wall {} in SubRoom {}-{} into {} pieces.",
                bigWall.toString(),
                subroom.GetRoomID(),
                subroom.GetSubRoomID(),
                splitWallPieces->size());

            // remove big wall and add one wallpiece to walls
            ReplaceBigWall(subroom, bigWall, *splitWallPieces);
            wallsRemoved = true;
        } else {
            if(!IsConnectedWall(subroom, bigWall)) {
                subroom.RemoveWall(bigWall);
                wallsRemoved = true;
            }
        }
    }

    return wallsRemoved;
}

void CorrectInputGeometry(Building & building)
{
    Logging::Info("Starting geometry::helper::correct to fix the geometry ...");

    bool geometry_changed = false;

    for(const auto & room : building.GetAllRooms()) {
        for(const auto & keySubroomPair : room.second->GetAllSubRooms()) {
            SubRoom & subroom = *keySubroomPair.second;

            // remove wall parts overlapping with exits
            geometry_changed = RemoveWallsOverlappingWithDoors(subroom) || geometry_changed;

            // remove walls reaching out of the subroom
            geometry_changed = RemoveBigWalls(subroom) || geometry_changed;
        }
    }

    if(geometry_changed) {
        if(building.GetConfig() == nullptr) {
            throw std::runtime_error(
                "Cannot write corrected geometry. No Configuration object found.");
        }
        fs::path newGeometryFile =
            building.GetConfig()->GetOutputPath() / building.GetGeometryFilename();
        fs::path newFilename("correct_");
        newFilename += newGeometryFile.filename();
        newGeometryFile.replace_filename(newFilename);

        if(building.SaveGeometry(newGeometryFile)) {
            building.GetConfig()->SetGeometryFile(newGeometryFile);
            LOG_INFO("Geometry was fixed, new file is stored in: {}.", newGeometryFile.string());
        }
    }
}

bool CorrectSubRoom(SubRoom & subroom)
{
    bool geometry_changed = false;
    geometry_changed      = RemoveWallsOverlappingWithDoors(subroom) || geometry_changed;

    // remove walls reaching out of the subroom
    geometry_changed = RemoveBigWalls(subroom) || geometry_changed;

    return geometry_changed;
}

std::vector<std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>>> ComputeTrainDoorCoordinates(
    const std::vector<Wall> & trackWalls,
    const std::vector<Transition> & trainDoors)
{
    const int scaleFactor = 1000; // very long orthogonal walls to train's doors
    std::vector<std::pair<PointWall, PointWall>> pws;
    // every door has two points.
    // for every point -> get pair<P, W>
    // collect pairs of pairs
    for(const auto & door : trainDoors) {
        PointWall pw1, pw2;
        int nintersections = 0;
        auto n             = door.NormalVec();
        auto p11           = door.GetPoint1() + n * scaleFactor;
        auto p12           = door.GetPoint1() - n * scaleFactor;
        auto p21           = door.GetPoint2() + n * scaleFactor;
        auto p22           = door.GetPoint2() - n * scaleFactor;
        auto normalWall1   = Wall(p11, p12);
        auto normalWall2   = Wall(p21, p22);
        for(const auto & twall : trackWalls) {
            Point interPoint1, interPoint2;
            auto res  = normalWall1.IntersectionWith(twall, interPoint1);
            auto res2 = normalWall2.IntersectionWith(twall, interPoint2);
            if(res == 1) {
                if(!twall.NearlyHasEndPoint(interPoint1)) {
                    pw1 = std::make_pair(interPoint1, twall);
                    nintersections++;
                } else // end point
                {
                    if(res2 == 0) {
                    } else {
                        pw1 = std::make_pair(interPoint1, twall);
                        nintersections++;
                    }
                }
            }
            if(res2 == 1) {
                if(!twall.NearlyHasEndPoint(interPoint2)) {
                    pw2 = std::make_pair(interPoint2, twall);
                    nintersections++;
                } else {
                    if(res == 0) {
                    } else {
                        pw2 = std::make_pair(interPoint2, twall);
                        nintersections++;
                    }
                }
            }


        } // tracks

        if(nintersections == 2) {
            pws.emplace_back(std::make_pair(pw1, pw2));
        } else {
            std::string message{fmt::format(
                FMT_STRING("Error in GetIntersection. Should be 2 but got {}."), nintersections)};
            throw std::runtime_error(message);
        }
    } // doors

    return pws;
}

void AddTrainDoors(
    int trainID,
    Building & building,
    SubRoom & subroom,
    const std::vector<Wall> & trackWalls,
    const std::vector<Transition> & trainDoors)
{
    auto wallDoorIntersectionPoints =
        geometry::helper::ComputeTrainDoorCoordinates(trackWalls, trainDoors);

    static int transition_id = 10000; // randomly high number

    auto room      = building.GetRoom(subroom.GetRoomID());
    auto trainType = building.GetTrain(trainID);

    auto walls = subroom.GetAllWalls();
    //---
    for(const auto & wallDoorIntersection : wallDoorIntersectionPoints) {
        //------------ transition --------
        auto * trainDoor = new Transition();
        trainDoor->SetID(transition_id++);
        trainDoor->SetCaption(trainType._type);
        trainDoor->SetPoint1(wallDoorIntersection.first.first);
        trainDoor->SetPoint2(wallDoorIntersection.second.first);
        std::string transType = "Train_" + std::to_string(trainID);
        trainDoor->SetType(transType);
        trainDoor->SetRoom1(room);
        trainDoor->SetSubRoom1(&subroom);

        room->AddTransitionID(trainDoor->GetUniqueID()); // danger area
        subroom.AddTransition(trainDoor);                // danger area
        building.AddTransition(trainDoor);               // danger area

        auto [addedWalls, removedWalls] = SplitWall(wallDoorIntersection, trackWalls, *trainDoor);

        std::for_each(
            std::begin(addedWalls),
            std::end(addedWalls),
            [trainID, &building, &subroom](const Wall & wall) {
                building.AddTrainWallAdded(trainID, wall);
                subroom.AddWall(wall);
            });
        std::for_each(
            std::begin(removedWalls),
            std::end(removedWalls),
            [trainID, &building, &subroom](const Wall & wall) {
                building.AddTrainWallRemoved(trainID, wall);
                subroom.RemoveWall(wall);
            });
        building.AddTrainDoorAdded(trainID, *trainDoor);
    }
    subroom.Update();
}

std::tuple<std::vector<Wall>, std::vector<Wall>> SplitWall(
    const std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> & wallDoorIntersectionPoints,
    const std::vector<Wall> & trackWalls,
    const Transition & door)
{
    std::vector<Wall> addedWalls;
    std::vector<Wall> removedWalls;

    auto pw1 = wallDoorIntersectionPoints.first;
    auto pw2 = wallDoorIntersectionPoints.second;
    auto p1  = pw1.first;
    auto w1  = pw1.second;
    auto p2  = pw2.first;
    auto w2  = pw2.second;

    // case 1
    Point P;
    if(w1 == w2) {
        double dist_pt1 = (w1.GetPoint1() - door.GetPoint1()).NormSquare();
        double dist_pt2 = (w1.GetPoint1() - door.GetPoint2()).NormSquare();
        Point A, B;

        if(dist_pt1 < dist_pt2) {
            A = door.GetPoint1();
            B = door.GetPoint2();
        } else {
            A = door.GetPoint2();
            B = door.GetPoint1();
        }

        Wall NewWall(w1.GetPoint1(), A);
        Wall NewWall1(w1.GetPoint2(), B);
        NewWall.SetType(w1.GetType());
        NewWall1.SetType(w1.GetType());

        // add new lines to be controled against overlap with exits
        if(NewWall.GetLength() > J_EPS_DIST) {
            addedWalls.emplace_back(NewWall);
        }


        if(NewWall1.GetLength() > J_EPS_DIST) {
            addedWalls.emplace_back(NewWall1);
        }

        removedWalls.emplace_back(w1);

    } else if(w1.ShareCommonPointWith(w2, P)) {
        //--------------------------------
        Point N, M;
        if(w1.GetPoint1() == P) {
            N = w1.GetPoint2();
        } else {
            N = w1.GetPoint1();
        }

        if(w2.GetPoint1() == P) {
            M = w2.GetPoint2();
        } else {
            M = w2.GetPoint1();
        }
        Wall NewWall(N, p1);
        Wall NewWall1(M, p2);
        NewWall.SetType(w1.GetType());
        NewWall1.SetType(w2.GetType());
        // changes to building
        addedWalls.emplace_back(NewWall);
        addedWalls.emplace_back(NewWall1);

        removedWalls.emplace_back(w1);
        removedWalls.emplace_back(w2);
    } else // disjoint
    {
        //--------------------------------
        // find points on w1 and w2 between p1 and p2
        // (A, B)
        Point A, B;
        if(door.isBetween(w1.GetPoint1())) {
            A = w1.GetPoint2();
        } else {
            A = w1.GetPoint1();
        }

        if(door.isBetween(w2.GetPoint1())) {
            B = w2.GetPoint2();
        } else {
            B = w2.GetPoint1();
        }
        Wall NewWall(A, p1);
        Wall NewWall1(B, p2);
        NewWall.SetType(w1.GetType());
        NewWall1.SetType(w2.GetType());
        // remove walls between
        for(const auto & wall : trackWalls) {
            if(door.isBetween(wall.GetPoint1()) || door.isBetween(wall.GetPoint2())) {
                removedWalls.emplace_back(wall);
            }
        }
        // changes to building
        addedWalls.emplace_back(NewWall);
        addedWalls.emplace_back(NewWall1);
    }

    return std::tuple{addedWalls, removedWalls};
}
} // namespace geometry::helper
