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

#include <Logger.h>
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

std::vector<std::pair<Point, Point>> ComputeTrainDoorCoordinates(
    const TrainType & train,
    const Track & track,
    double trainStartOffset,
    bool fromEnd)
{
    std::vector<std::pair<Point, Point>> trainDoorCoordinates;

    if(track._walls.empty()) {
        LOG_WARNING(
            "Compute train door coordinates of train {} at track {}: no tracks wall given. Please "
            "check your "
            "input files (geometry, train time table).",
            train._type,
            track._id);
        return trainDoorCoordinates;
    }

    std::vector<Wall> trackWalls{track._walls};
    if(fromEnd) {
        std::reverse(std::begin(trackWalls), std::end(trackWalls));
        std::for_each(std::begin(trackWalls), std::end(trackWalls), [](Wall & wall) {
            Point tmp{wall.GetPoint1()};
            wall.SetPoint1(wall.GetPoint2());
            wall.SetPoint2(tmp);
        });
    }
    Point start{std::begin(trackWalls)->GetPoint1()};

    for(const auto trainDoor : train._doors) {
        double distanceFromTrackStart = trainStartOffset + trainDoor._distance;
        double width                  = trainDoor._width;
        std::vector<Point> intersectionPoints;

        auto doorStart = FindWallPointWithDistanceOnWall(trackWalls, start, distanceFromTrackStart);

        if(doorStart.has_value()) {
            auto doorStartWallItr = std::find_if(
                std::begin(trackWalls), std::end(trackWalls), [&doorStart](const Wall & wall) {
                    return wall.IsInLineSegment(doorStart.value()) &&
                           wall.GetPoint2() != doorStart.value();
                });

            std::for_each(
                doorStartWallItr,
                std::end(trackWalls),
                [&width, &doorStart, &intersectionPoints](const Wall & wall) {
                    std::vector<Point> intersections;
                    wall.IntersectionWithCircle(doorStart.value(), width, intersections);
                    intersectionPoints.insert(
                        std::end(intersectionPoints),
                        std::begin(intersections),
                        std::end(intersections));
                });

            auto doorEndPointItr = std::find_if(
                std::begin(intersectionPoints),
                std::end(intersectionPoints),
                [&trackWalls, &doorStart](const Point & intersectionPoint) {
                    auto endItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&intersectionPoint](const Wall & wall) {
                            return wall.IsInLineSegment(intersectionPoint) &&
                                   wall.GetPoint2() != intersectionPoint;
                        });
                    if(endItr == std::end(trackWalls)) {
                        return false;
                    }
                    Line tmp{doorStart.value(), endItr->GetPoint2(), 0};
                    return tmp.IsInLineSegment(intersectionPoint);
                });

            // No intersection point in correct direction found
            if(doorEndPointItr == std::end(intersectionPoints)) {
                LOG_WARNING(
                    "Compute train door coordinates of train {} at track {}: train door with "
                    "distance {} and width {} could not be placed on track walls. Please check "
                    "your geometry.",
                    train._type,
                    track._id,
                    trainDoor._distance,
                    trainDoor._width);
                continue;
            }

            trainDoorCoordinates.emplace_back(doorStart.value(), *doorEndPointItr);
        }
    }

    return trainDoorCoordinates;
}

void AddTrainDoors(
    int trainID,
    Building & building,
    SubRoom & subroom,
    const TrainType & train,
    const Track & track,
    double trainStartOffset,
    bool fromEnd)
{
    static int transition_id = 10000; // randomly high number

    auto wallDoorIntersectionPoints =
        geometry::helper::ComputeTrainDoorCoordinates(train, track, trainStartOffset, fromEnd);

    auto room = building.GetRoom(subroom.GetRoomID());

    std::vector<Transition> trainDoors;
    for(auto & trainDoorCoordinates : wallDoorIntersectionPoints) {
        auto trainDoor = new Transition();
        trainDoor->SetID(transition_id++);
        trainDoor->SetCaption(train._type);
        if(fromEnd) {
            trainDoor->SetPoint1(trainDoorCoordinates.second);
            trainDoor->SetPoint2(trainDoorCoordinates.first);
        } else {
            trainDoor->SetPoint1(trainDoorCoordinates.first);
            trainDoor->SetPoint2(trainDoorCoordinates.second);
        }
        std::string transType = "Train_" + std::to_string(trainID);
        trainDoor->SetType(transType);
        trainDoor->SetRoom1(room);
        trainDoor->SetSubRoom1(&subroom);

        trainDoors.emplace_back(*trainDoor);
    }

    auto [addedWalls, removedWalls] = SplitWalls(track._walls, trainDoors);

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

    std::for_each(
        std::begin(trainDoors),
        std::end(trainDoors),
        [trainID, &building, &room, &subroom](const Transition & door) {
            auto trainDoor = new Transition(door);
            building.AddTrainDoorAdded(trainID, door);
            room->AddTransitionID(trainDoor->GetUniqueID()); // danger area
            subroom.AddTransition(trainDoor);                // danger area
            building.AddTransition(trainDoor);               // danger area
        });

    subroom.Update();
}

std::tuple<std::vector<Wall>, std::vector<Wall>>
SplitWalls(const std::vector<Wall> & trackWalls, const std::vector<Transition> & doors)
{
    std::vector<Wall> walls{trackWalls};
    std::vector<Wall> addedWalls;
    std::vector<Wall> removedWalls;

    for(auto const & door : doors) {
        auto wallStartItr =
            std::find_if(std::begin(walls), std::end(walls), [&door](const Wall & wall) {
                return wall.IsInLineSegment(door.GetPoint1()) &&
                       wall.GetPoint2() != door.GetPoint1();
            });
        if(wallStartItr == std::end(walls)) {
            throw std::runtime_error("");
        }

        auto wallEndItr =
            std::find_if(std::begin(walls), std::end(walls), [&door](const Wall & wall) {
                return wall.IsInLineSegment(door.GetPoint2());
            });
        if(wallEndItr == std::end(walls)) {
            throw std::runtime_error("");
        }

        std::vector<Wall> splittedWalls;
        if(wallStartItr->HasEndPoint(door.GetPoint1())) {
            splittedWalls.emplace_back(Wall{door.GetPoint2(), wallEndItr->GetPoint2()});
        } else if(wallEndItr->HasEndPoint(door.GetPoint2())) {
            splittedWalls.emplace_back(Wall{wallStartItr->GetPoint1(), door.GetPoint1()});
        } else {
            Wall wallSplitStart{wallStartItr->GetPoint1(), door.GetPoint1()};
            Wall wallSplitEnd{door.GetPoint2(), wallEndItr->GetPoint2()};
            splittedWalls.insert(std::end(splittedWalls), {wallSplitStart, wallSplitEnd});
        }

        std::vector<Wall> tmp;
        std::copy(std::begin(walls), wallStartItr, std::back_inserter(tmp));
        tmp.insert(std::end(tmp), std::begin(splittedWalls), std::end(splittedWalls));
        std::copy(wallEndItr + 1, std::end(walls), std::back_inserter(tmp));

        walls.assign(std::begin(tmp), std::end(tmp));
    }


    std::copy_if(
        std::begin(walls),
        std::end(walls),
        std::back_inserter(addedWalls),
        [&trackWalls](const Wall & wall) {
            return std::find(std::begin(trackWalls), std::end(trackWalls), wall) ==
                   std::end(trackWalls);
        });

    std::copy_if(
        std::begin(trackWalls),
        std::end(trackWalls),
        std::back_inserter(removedWalls),
        [&walls](const Wall & wall) {
            return std::find(std::begin(walls), std::end(walls), wall) == std::end(walls);
        });

    return std::tuple{addedWalls, removedWalls};
}

void SortWalls(std::vector<Wall> & walls, const Point & start)
{
    auto startItr = std::find_if(std::begin(walls), std::end(walls), [&start](const Wall & wall) {
        return wall.HasEndPoint(start);
    });

    if(startItr == std::end(walls)) {
        std::string message{fmt::format(
            FMT_STRING("Track walls could not be sorted. Start {} is not on one of the track "
                       "walls. Please check your geometry."),
            start.toString())};
        throw std::runtime_error(message);
    }

    std::rotate(startItr, std::begin(walls), std::begin(walls) + 1);

    bool sane = true;
    std::string message{};

    for(size_t i = 1; i < walls.size(); ++i) {
        auto compare = std::begin(walls) + i - 1;

        // find element that succeeds compare
        auto nextItr = std::find_if(compare + 1, std::end(walls), [&compare](const Wall & wall) {
            return wall.ShareCommonPointWith(*compare);
        });

        if(nextItr != std::end(walls)) {
            // move found element such that it follows compare in vector
            std::rotate(nextItr, compare + 1, compare + 2);
        } else {
            // if no suceeding element found, there is an issue
            sane    = false;
            message = fmt::format(
                FMT_STRING("Track walls could not be sorted. Could not find a wall succeeding {} "
                           "in track walls. Please check your geometry"),
                compare->toString());
        }
    }

    // rotate walls such that walls[n]->P2 == wall[n+1]->P1
    auto wallStart = std::begin(walls);
    if(wallStart->GetPoint2() == start) {
        Point tmp = wallStart->GetPoint2();
        wallStart->SetPoint2(wallStart->GetPoint1());
        wallStart->SetPoint1(tmp);
    }

    for(auto wallItr = std::begin(walls) + 1; wallItr != std::end(walls); ++wallItr) {
        auto prev = std::prev(wallItr);
        if(prev->GetPoint2() != wallItr->GetPoint1()) {
            Point tmp = wallItr->GetPoint2();
            wallItr->SetPoint2(wallItr->GetPoint1());
            wallItr->SetPoint1(tmp);
        }
    }

    if(!sane) {
        throw std::runtime_error(message);
    }
}

std::optional<Point> FindWallPointWithDistanceOnWall(
    const std::vector<Wall> & walls,
    const Point & origin,
    double distance)
{
    auto startItr = std::find_if(std::begin(walls), std::end(walls), [&origin](const Wall & wall) {
        return wall.IsInLineSegment(origin) && wall.GetPoint2() != origin;
    });

    if(startItr == std::end(walls)) {
        LOG_WARNING("Starting point does not belong to given walls.");
        return std::nullopt;
    }

    std::vector<Point> intersections;

    for(auto itr = startItr; itr != std::end(walls); ++itr) {
        Point start;
        if(itr == startItr) {
            start = origin;
        } else {
            start = itr->GetPoint1();
        }

        if(Distance(start, itr->GetPoint2()) >= distance) {
            Point connection     = (itr->GetPoint2() - itr->GetPoint1()).Normalized();
            Point distConnection = connection * distance;
            Point distPoint      = start + distConnection;

            return distPoint;
        }
        distance -= Distance(start, itr->GetPoint2());
    }

    return std::nullopt;
}
} // namespace geometry::helper
