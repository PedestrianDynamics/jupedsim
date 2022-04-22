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

#include "CorrectGeometry.hpp"

#include "general/Configuration.hpp"
#include "general/Filesystem.hpp"
#include "geometry/Building.hpp"
#include "geometry/Crossing.hpp"
#include "geometry/Line.hpp"
#include "geometry/Point.hpp"
#include "geometry/Room.hpp"
#include "geometry/SubRoom.hpp"
#include "geometry/Transition.hpp"
#include "geometry/Wall.hpp"

#include <Logger.hpp>
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
std::optional<Point> ComputeSplitPoint(const Wall& wall, const Line& line)
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
    if(std::isnan(intersectionPoint.x) || std::isnan(intersectionPoint.y)) {
        return std::nullopt;
    }

    LOG_DEBUG(
        "BigWall {} will be split due to intersection with Line {} in {}",
        wall.toString(),
        line.toString(),
        intersectionPoint.toString());

    return {intersectionPoint};
}

std::map<int, std::pair<Point, Point>> ComputeTrainDoorCoordinates(
    const TrainType& train,
    const Track& track,
    double trainStartOffset,
    bool fromEnd)
{
    std::map<int, std::pair<Point, Point>> trainDoorCoordinates;

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

    // reverse direction of walls and their points if needed
    if(fromEnd) {
        std::reverse(std::begin(trackWalls), std::end(trackWalls));
        std::for_each(std::begin(trackWalls), std::end(trackWalls), [](Wall& wall) {
            Point tmp{wall.GetPoint1()};
            wall.SetPoint1(wall.GetPoint2());
            wall.SetPoint2(tmp);
        });
    }

    Point start{std::begin(trackWalls)->GetPoint1()};
    for(const auto& [_, trainDoor] : train._doors) {
        double distanceFromTrackStart = trainStartOffset + trainDoor._distance;
        double width = trainDoor._width;
        std::vector<Point> intersectionPoints;

        // Find the door start on track walls
        auto doorStart = FindWallPointWithDistanceOnWall(trackWalls, start, distanceFromTrackStart);

        if(doorStart.has_value()) {
            // Find wall on which door start is located
            auto doorStartWallItr = std::find_if(
                std::begin(trackWalls), std::end(trackWalls), [&doorStart](const Wall& wall) {
                    return wall.IsInLineSegment(doorStart.value()) &&
                           wall.GetPoint2() != doorStart.value();
                });

            // Find all points with distance width from door start
            std::for_each(
                doorStartWallItr,
                std::end(trackWalls),
                [&width, &doorStart, &intersectionPoints](const Wall& wall) {
                    std::vector<Point> intersections =
                        wall.IntersectionPointsWithCircle(doorStart.value(), width);
                    intersectionPoints.insert(
                        std::end(intersectionPoints),
                        std::begin(intersections),
                        std::end(intersections));
                });

            // Check if one of the intersection points is between doorStart and end(trackWalls)
            auto doorEndPointItr = std::find_if(
                std::begin(intersectionPoints),
                std::end(intersectionPoints),
                [&trackWalls, &doorStart, &doorStartWallItr](const Point& intersectionPoint) {
                    // Find wall succeeding doorStartWallItr, where intersectionPoint is located
                    auto endItr = std::find_if(
                        doorStartWallItr,
                        std::end(trackWalls),
                        [&intersectionPoint](const Wall& wall) {
                            return wall.IsInLineSegment(intersectionPoint) &&
                                   wall.GetPoint2() != intersectionPoint;
                        });

                    if(endItr == std::end(trackWalls)) {
                        return false;
                    }

                    if(endItr == doorStartWallItr) {
                        // Check if end point is between doorStart and wall.Point2 for correct
                        // direction
                        Line tmp{doorStart.value(), endItr->GetPoint2(), 0};
                        return tmp.IsInLineSegment(intersectionPoint);
                    }
                    return true;
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

            trainDoorCoordinates.emplace(
                trainDoor._id, std::make_pair(doorStart.value(), *doorEndPointItr));
        }
    }

    return trainDoorCoordinates;
}

void AddTrainDoors(
    int trainId,
    int trackId,
    Building& building,
    const TrainType& train,
    double trainStartOffset,
    bool fromEnd,
    Geometry& geometry)
{
    static int transition_id = 100000; // randomly high number

    const auto track = building.GetTrack(trackId);
    if(!track) {
        throw std::runtime_error(
            fmt::format(FMT_STRING("Could not find track with ID {:d}"), trackId));
    }

    auto* room = building.GetRoom(track->_roomID);
    auto* subroom = room->GetSubRoom(track->_subRoomID);

    // Get the door coordinates
    auto wallDoorIntersectionPoints =
        geometry::helper::ComputeTrainDoorCoordinates(train, *track, trainStartOffset, fromEnd);

    // Create train doors at the computed locations
    std::vector<Transition> trainDoors;
    for(const auto& [trainDoorID, trainDoorCoordinates] : wallDoorIntersectionPoints) {
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
        std::string transType = "Train_" + std::to_string(trainId);
        trainDoor->SetType(transType);
        trainDoor->SetRoom1(room);
        trainDoor->SetSubRoom1(subroom);

        // Set outflow rate of train door
        trainDoor->SetDN(1);
        trainDoor->SetOutflowRate(train._doors.at(trainDoorID)._flow);
        trainDoors.emplace_back(*trainDoor);
    }

    // Add/remove walls to create a valid geometry
    auto [addedWalls, removedWalls] = SplitWalls(track->_walls, trainDoors);

    std::for_each(
        std::begin(addedWalls),
        std::end(addedWalls),
        [trainId, &building, &subroom, &geometry](const Wall& wall) {
            building.AddTrainWallAdded(trainId, wall);
            subroom->AddWall(wall);
            geometry.AddLineSegment(wall);
        });

    std::for_each(
        std::begin(removedWalls),
        std::end(removedWalls),
        [trainId, &building, &subroom, &geometry](const Wall& wall) {
            building.AddTrainWallRemoved(trainId, wall);
            subroom->RemoveWall(wall);
            geometry.RemoveLineSegment(wall);
        });

    // Add doors to geometry
    std::for_each(
        std::begin(trainDoors),
        std::end(trainDoors),
        [trainId, &building, &room, &subroom](const Transition& door) {
            auto trainDoor = new Transition(door);
            building.AddTrainDoorAdded(trainId, door);
            // Important: Door needs to be added to room, subroom, and building!
            room->AddTransitionID(trainDoor->GetUniqueID());
            subroom->AddTransition(trainDoor);
            building.AddTransition(trainDoor);
        });

    subroom->Update();
}

std::tuple<std::vector<Wall>, std::vector<Wall>>
SplitWalls(const std::vector<Wall>& trackWalls, const std::vector<Transition>& doors)
{
    std::vector<Wall> walls{trackWalls};
    std::vector<Wall> addedWalls;
    std::vector<Wall> removedWalls;

    for(auto const& door : doors) {
        // search for wall containing door.Point1
        auto wallStartItr =
            std::find_if(std::begin(walls), std::end(walls), [&door](const Wall& wall) {
                return wall.IsInLineSegment(door.GetPoint1()) &&
                       wall.GetPoint2() != door.GetPoint1();
            });
        if(wallStartItr == std::end(walls)) {
            std::string message{fmt::format(
                FMT_STRING("Point {} does not belong to any track walls."),
                door.GetPoint1().toString())};
            throw std::runtime_error(message);
        }

        // search for wall containing door.Point1
        auto wallEndItr =
            std::find_if(std::begin(walls), std::end(walls), [&door](const Wall& wall) {
                return wall.IsInLineSegment(door.GetPoint2());
            });
        if(wallEndItr == std::end(walls)) {
            std::string message{fmt::format(
                FMT_STRING("Point {} does not belong to any track walls."),
                door.GetPoint2().toString())};
            throw std::runtime_error(message);
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

        // Update walls to use already split walls for next doors
        std::vector<Wall> tmp;
        std::copy(std::begin(walls), wallStartItr, std::back_inserter(tmp));
        tmp.insert(std::end(tmp), std::begin(splittedWalls), std::end(splittedWalls));
        std::copy(wallEndItr + 1, std::end(walls), std::back_inserter(tmp));
        walls.assign(std::begin(tmp), std::end(tmp));
    }

    // Add all walls which have not been in the original track walls to addedWalls
    std::copy_if(
        std::begin(walls),
        std::end(walls),
        std::back_inserter(addedWalls),
        [&trackWalls](const Wall& wall) {
            return std::find(std::begin(trackWalls), std::end(trackWalls), wall) ==
                   std::end(trackWalls);
        });

    // Add all walls which are in the original track walls but not in the split track walls
    // to removedWalls
    std::copy_if(
        std::begin(trackWalls),
        std::end(trackWalls),
        std::back_inserter(removedWalls),
        [&walls](const Wall& wall) {
            return std::find(std::begin(walls), std::end(walls), wall) == std::end(walls);
        });

    return std::tuple{addedWalls, removedWalls};
}

void SortWalls(std::vector<Wall>& walls, const Point& start)
{
    if(walls.size() == 1) {
        return;
    }

    // Find wall containing the starting point
    auto startItr = std::find_if(std::begin(walls), std::end(walls), [&start](const Wall& wall) {
        return wall.HasEndPoint(start);
    });

    if(startItr == std::end(walls)) {
        std::string message{fmt::format(
            FMT_STRING("Track walls could not be sorted. Start {} is not on one of the track "
                       "walls. Please check your geometry."),
            start.toString())};
        throw std::runtime_error(message);
    }

    // move start point to begin
    std::iter_swap(startItr, std::begin(walls));
    bool sane = true;
    std::string message{};

    for(auto compare = std::begin(walls); compare != std::prev(std::end(walls)); ++compare) {
        // find element that succeeds compare
        auto nextItr =
            std::find_if(std::next(compare), std::end(walls), [&compare](const Wall& wall) {
                return wall.ShareCommonPointWith(*compare);
            });

        if(nextItr != std::end(walls)) {
            // move found element such that it follows compare in vector
            std::iter_swap(nextItr, std::next(compare));
        } else {
            // if no succeeding element found, there is an issue
            sane = false;
            message = fmt::format(
                FMT_STRING("Track walls could not be sorted. Could not find a wall succeeding {} "
                           "in track walls. Please check your geometry"),
                compare->toString());
            break;
        }
    }

    if(!sane) {
        throw std::runtime_error(message);
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
}

std::optional<Point> FindWallPointWithDistanceOnWall(
    const std::vector<Wall>& walls,
    const Point& origin,
    double distance)
{
    // find wall segment containing origin
    auto startItr = std::find_if(std::begin(walls), std::end(walls), [&origin](const Wall& wall) {
        return wall.IsInLineSegment(origin) && wall.GetPoint2() != origin;
    });

    if(startItr == std::end(walls)) {
        LOG_WARNING("Starting point does not belong to given walls.");
        return std::nullopt;
    }

    for(auto itr = startItr; itr != std::end(walls); ++itr) {
        Point start;
        if(itr == startItr) {
            start = origin;
        } else {
            start = itr->GetPoint1();
        }

        // check if point is in current wall segment
        if(Distance(start, itr->GetPoint2()) >= distance) {
            Point connection = (itr->GetPoint2() - itr->GetPoint1()).Normalized();
            Point distConnection = connection * distance;
            Point distPoint = start + distConnection;

            return distPoint;
        }
        distance -= Distance(start, itr->GetPoint2());
    }

    return std::nullopt;
}
} // namespace geometry::helper
