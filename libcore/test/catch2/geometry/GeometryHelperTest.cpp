/*
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
#include "geometry/Building.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/SubRoom.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"
#include "geometry/helper/CorrectGeometry.h"

#include <algorithm>
#include <catch2/catch.hpp>

TEST_CASE(
    "geometry/helper/ComputeTrainDoorCoordinates",
    "[geometry][helper][ComputeTrainDoorCoordinates]")
{
    SECTION("ComputeTrainDoorCoordinates")
    {
        Wall wall1{{-10., 10.}, {-8., 11.}};
        Wall wall2{{-8., 11.}, {-6., 11.5}};
        Wall wall3{{-6., 11.5}, {-4., 12}};
        Wall wall4{{-4., 12.}, {-2., 12.25}};
        Wall wall5{{-2., 12.25}, {0, 12.25}};

        std::vector<Wall> trackWalls{wall1, wall2, wall3, wall4, wall5};

        SECTION("Each door on one track wall")
        {
            double offset = -10.;
            std::vector<Transition> trainDoors;
            for(size_t i = 0; i < trackWalls.size(); ++i) {
                Transition door;

                door.SetPoint1({offset + 2. * i + 0.5, 0.});
                door.SetPoint2({offset + 2. * i + 1.5, 0.});
                trainDoors.emplace_back(door);
            }

            auto doorCoordinates =
                geometry::helper::ComputeTrainDoorCoordinates(trackWalls, trainDoors);
            REQUIRE(doorCoordinates.size() == trainDoors.size());

            for(auto doorCoordinate = std::begin(doorCoordinates);
                doorCoordinate != std::end(doorCoordinates);
                ++doorCoordinate) {
                auto trainDoor1 = doorCoordinate->first;
                auto trainDoor2 = doorCoordinate->second;
                auto index      = std::distance(std::begin(doorCoordinates), doorCoordinate);

                // check if point is really on line
                REQUIRE(trainDoor1.second.IsInLineSegment(trainDoor1.first));
                REQUIRE(trainDoor2.second.IsInLineSegment(trainDoor2.first));

                // check if both points of door are on the same line
                auto computedWall1 =
                    std::find(std::begin(trackWalls), std::end(trackWalls), trainDoor1.second);
                auto computedWall2 =
                    std::find(std::begin(trackWalls), std::end(trackWalls), trainDoor2.second);
                REQUIRE(computedWall1 == computedWall2);

                // check if both points are correct
                double slope = (computedWall1->GetPoint2()._y - computedWall1->GetPoint1()._y) /
                               (computedWall1->GetPoint2()._x - computedWall1->GetPoint1()._x);

                double expectedX1 = offset + 2. * index + 0.5;
                REQUIRE(trainDoor1.first._x == Approx(expectedX1));
                double expectedY1    = computedWall1->GetPoint1()._y + slope * 0.5;
                REQUIRE(trainDoor1.first._y == Approx(expectedY1));

                double expectedX2 = offset + 2. * index + 1.5;
                REQUIRE(trainDoor2.first._x == Approx(expectedX2));
                double expectedY2    = computedWall1->GetPoint1()._y + slope * 1.5;
                REQUIRE(trainDoor2.first._y == Approx(expectedY2));
            }
        }

        SECTION("Each door on seperate track wall")
        {
            double offset = -10.;
            std::vector<Transition> trainDoors;
            for(size_t i = 0; i < trackWalls.size()-1; ++i) {
                Transition door;

                door.SetPoint1({offset + 2. * i + 1.75, 0.});
                door.SetPoint2({offset + 2. * i + 2.25, 0.});
                trainDoors.emplace_back(door);
            }

            auto doorCoordinates =
                geometry::helper::ComputeTrainDoorCoordinates(trackWalls, trainDoors);
            REQUIRE(doorCoordinates.size() == trainDoors.size());

            for(auto doorCoordinate = std::begin(doorCoordinates);
                doorCoordinate != std::end(doorCoordinates);
                ++doorCoordinate) {
                auto trainDoor1 = doorCoordinate->first;
                auto trainDoor2 = doorCoordinate->second;
                auto index      = std::distance(std::begin(doorCoordinates), doorCoordinate);

                // check if point is really on line
                REQUIRE(trainDoor1.second.IsInLineSegment(trainDoor1.first));
                REQUIRE(trainDoor2.second.IsInLineSegment(trainDoor2.first));

                // check if both points of door are on the same line
                auto computedWall1 =
                    std::find(std::begin(trackWalls), std::end(trackWalls), trainDoor1.second);
                auto computedWall2 =
                    std::find(std::begin(trackWalls), std::end(trackWalls), trainDoor2.second);
                REQUIRE(computedWall1 != computedWall2);

                // check if both points are correct
                double slope1 = (computedWall1->GetPoint2()._y - computedWall1->GetPoint1()._y) /
                               (computedWall1->GetPoint2()._x - computedWall1->GetPoint1()._x);

                double expectedX1 = offset + 2. * index + 1.75;
                REQUIRE(trainDoor1.first._x == Approx(expectedX1));
                double expectedY1    = computedWall1->GetPoint1()._y + slope1 * 1.75;
                REQUIRE(trainDoor1.first._y == Approx(expectedY1));

                double slope2 = (computedWall2->GetPoint2()._y - computedWall2->GetPoint1()._y) /
                                (computedWall2->GetPoint2()._x - computedWall2->GetPoint1()._x);
                double expectedX2 = offset + 2. * index + 2.25;
                REQUIRE(trainDoor2.first._x == Approx(expectedX2));
                double expectedY2    = computedWall2->GetPoint1()._y + slope2 * 0.25;
                REQUIRE(trainDoor2.first._y == Approx(expectedY2));
            }
        }

    }
}