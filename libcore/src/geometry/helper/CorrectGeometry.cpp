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

#include "general/Configuration.h" // for Configuration
#include "general/Filesystem.h"    // for add_prefix_to_filename
#include "general/Logger.h"        // for Debug, Warning, Info
#include "geometry/Building.h"     // for Building
#include "geometry/Crossing.h"     // for Crossing
#include "geometry/Line.h"         // for Line
#include "geometry/Point.h"        // for Point, Distance
#include "geometry/Room.h"         // for Room
#include "geometry/SubRoom.h"      // for SubRoom
#include "geometry/Transition.h"   // for Transition
#include "geometry/Wall.h"         // for Wall

#include <algorithm> // for max, find, find_if, sort
#include <cassert>
#include <chrono> // for operator-, duration, high_resolut...
#include <cmath>  // for isnan
#include <fmt/format.h>
#include <map>       // for map
#include <memory>    // for shared_ptr, __shared_ptr_access
#include <optional>  // for optional, nullopt
#include <ratio>     // for milli
#include <stdexcept> // for runtime_error
#include <string>    // for basic_string, operator+
#include <utility>   // for pair
#include <vector>    // for vector<>::iterator, vector<>::con...

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
        wallsRemoved = RemoveOverlappingWall(*crossing, subroom) || wallsRemoved;
    }

    for(Transition const * const transition : subroom.GetAllTransitions()) {
        wallsRemoved = RemoveOverlappingWall(*transition, subroom) || wallsRemoved;
    }

    return wallsRemoved;
}


std::optional<Point> ComputeSplitPoint(const Wall & wall, const Line & line)
{
    // Equal lines should not be split.
    if(wall == line) {
        return std::nullopt;
    }

    // Walls starting or ending in common point should not be split.
    if(wall.ShareCommonPointWith(line)) {
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

    // Intersection points with NAN cannot be split
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

bool IsPointAndSubroomIncident(const SubRoom & subroom, const Point & point, const Wall & wall)
{
    // Incident with wall at point 1 or 2
    for(const auto & checkWall : subroom.GetAllWalls()) {
        if(wall != checkWall &&
           (checkWall.GetPoint1() == point || checkWall.GetPoint2() == point)) {
            return true;
        }
    }
    // Incident with transition
    for(const auto & transition : subroom.GetAllTransitions()) {
        if(transition->GetPoint1() == point || transition->GetPoint2() == point) {
            return true;
        }
    }
    //Incident with crossing
    for(const auto & crossing : subroom.GetAllCrossings()) {
        if(crossing->GetPoint1() == point || crossing->GetPoint2() == point) {
            return true;
        }
    }
    // Incident with wall in any point
    for(const auto & checkWall : subroom.GetAllWalls()) {
        if(wall != checkWall && checkWall.IsInLineSegment(point)) {
            return true;
        }
    }
    return false;
}

bool IsConnectedWall(const SubRoom & subroom, const Wall & wall)
{
    return IsPointAndSubroomIncident(subroom, wall.GetPoint1(), wall) &&
           IsPointAndSubroomIncident(subroom, wall.GetPoint2(), wall);
}

int AddWallToSubroom(SubRoom & subroom, const std::vector<Wall> & wallPieces)
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
    LOG_DEBUG(
        "Replacing big wall {} in SubRoom {}-{}",
        bigWall.toString(),
        subroom.GetRoomID(),
        subroom.GetSubRoomID());

    if(!subroom.RemoveWall(bigWall)) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("Correcting the geometry failed. Could not remove wall: {}"),
            bigWall.toString()));
    }

    if(AddWallToSubroom(subroom, wallPieces) <= 0) {
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
    const auto t_start = std::chrono::high_resolution_clock::now();
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
        }
    }

    const auto t_end = std::chrono::high_resolution_clock::now();
    LOG_INFO(
        "Leave geometry correct with success ({:.3f}s)",
        std::chrono::duration<double, std::milli>(t_end - t_start).count());
}
} // namespace geometry::helper
