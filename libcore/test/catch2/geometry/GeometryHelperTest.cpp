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
#include <cmath>

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
                double expectedY1 = computedWall1->GetPoint1()._y + slope * 0.5;
                REQUIRE(trainDoor1.first._y == Approx(expectedY1));

                double expectedX2 = offset + 2. * index + 1.5;
                REQUIRE(trainDoor2.first._x == Approx(expectedX2));
                double expectedY2 = computedWall1->GetPoint1()._y + slope * 1.5;
                REQUIRE(trainDoor2.first._y == Approx(expectedY2));
            }
        }

        SECTION("Each door on seperate track wall")
        {
            double offset = -10.;
            std::vector<Transition> trainDoors;
            for(size_t i = 0; i < trackWalls.size() - 1; ++i) {
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
                double expectedY1 = computedWall1->GetPoint1()._y + slope1 * 1.75;
                REQUIRE(trainDoor1.first._y == Approx(expectedY1));

                double slope2 = (computedWall2->GetPoint2()._y - computedWall2->GetPoint1()._y) /
                                (computedWall2->GetPoint2()._x - computedWall2->GetPoint1()._x);
                double expectedX2 = offset + 2. * index + 2.25;
                REQUIRE(trainDoor2.first._x == Approx(expectedX2));
                double expectedY2 = computedWall2->GetPoint1()._y + slope2 * 0.25;
                REQUIRE(trainDoor2.first._y == Approx(expectedY2));
            }
        }
    }
}

TEST_CASE("geometry/helper/SplitWall", "[geometry][helper][SplitWall]")
{
    SECTION("Straight track")
    {
        SECTION("ordered track walls")
        {
            Wall trackWall1{{-10., -10.}, {-8., -8.}};
            Wall trackWall2{{-8., -8.}, {-6., -6.}};
            Wall trackWall3{{-6., -6.}, {-4., -4.}};
            Wall trackWall4{{-4., -4.}, {-2., -2.}};
            Wall trackWall5{{-2., -2.}, {0., 0.}};

            std::vector<Wall> trackWalls{
                trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

            SECTION("Door on one wall element")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    Point doorVector{trackWalls[i].GetPoint2() - trackWalls[i].GetPoint1()};
                    Point doorVectorNormalized{doorVector.Normalized()};

                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWalls[i].GetCentre() - doorVectorNormalized * 0.5);
                    trainDoor.SetPoint2(trackWalls[i].GetCentre() + doorVectorNormalized * 0.5);

                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWalls[i]},
                         {trainDoor.GetPoint2(), trackWalls[i]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);

                    Wall newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWalls[i].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));
                    REQUIRE(removedWalls.size() == 1);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i]));
                }
            }

            SECTION("Door on neighboring wall elements")
            {
                for(size_t i = 0; i < trackWalls.size() - 1; ++i) {
                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWalls[i].GetCentre());
                    trainDoor.SetPoint2(trackWalls[i + 1].GetCentre());

                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWalls[i]},
                         {trainDoor.GetPoint2(), trackWalls[i + 1]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);

                    Wall newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWalls[i + 1].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));
                    REQUIRE(removedWalls.size() == 2);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i]));
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i + 1]));
                }
            }

            SECTION("Door on not neighboring wall elements")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    for(size_t j = 0; j < trackWalls.size(); ++j) {
                        if(std::abs(static_cast<int>(i) - static_cast<int>(j)) <= 1) {
                            continue;
                        }
                        Transition trainDoor;
                        trainDoor.SetPoint1(trackWalls[i].GetCentre());
                        trainDoor.SetPoint2(trackWalls[j].GetCentre());

                        std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>>
                            wallDoorIntersection = {
                                {trainDoor.GetPoint1(), trackWalls[i]},
                                {trainDoor.GetPoint2(), trackWalls[j]}};

                        auto [addedWalls, removedWalls] = geometry::helper::SplitWall(
                            wallDoorIntersection, trackWalls, trainDoor);

                        Wall newWall1, newWall2;

                        if(i < j) {
                            newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                            newWall2 = {trackWalls[j].GetPoint2(), trainDoor.GetPoint2()};
                        } else {
                            newWall1 = {trackWalls[j].GetPoint1(), trainDoor.GetPoint2()};
                            newWall2 = {trackWalls[i].GetPoint2(), trainDoor.GetPoint1()};
                        }

                        REQUIRE(addedWalls.size() == 2);
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));

                        unsigned int numRemovedWalls =
                            1 + std::abs(static_cast<int>(i) - static_cast<int>(j));
                        REQUIRE(removedWalls.size() == numRemovedWalls);

                        for(auto k = std::min(i, j); k <= std::max(i, j); ++k) {
                            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[k]));
                        }
                    }
                }
            }
        }

        SECTION("unordered track walls")
        {
            Wall trackWall1{{-10., -10.}, {-8., -8.}};
            Wall trackWall2{{-8., -8.}, {-6., -6.}};
            Wall trackWall3{{-6., -6.}, {-4., -4.}};
            Wall trackWall4{{-4., -4.}, {-2., -2.}};
            Wall trackWall5{{-2., -2.}, {0., 0.}};

            std::vector<Wall> trackWallsOrdered{
                trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

            std::vector<Wall> trackWalls{
                trackWall2, trackWall5, trackWall1, trackWall4, trackWall3};

            std::map<int, int> zuordnung{{0, 1}, {1, 4}, {2, 0}, {3, 3}, {4, 2}};

            SECTION("Door on one wall element")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    Point doorVector{trackWalls[i].GetPoint2() - trackWalls[i].GetPoint1()};
                    Point doorVectorNormalized{doorVector.Normalized()};

                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWalls[i].GetCentre() - doorVectorNormalized * 0.5);
                    trainDoor.SetPoint2(trackWalls[i].GetCentre() + doorVectorNormalized * 0.5);

                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWalls[i]},
                         {trainDoor.GetPoint2(), trackWalls[i]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);

                    Wall newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWalls[i].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));
                    REQUIRE(removedWalls.size() == 1);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i]));
                }
            }

            SECTION("Door on neighboring wall elements")
            {
                for(size_t i = 0; i < trackWallsOrdered.size() - 1; ++i) {
                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWallsOrdered[i].GetCentre());
                    trainDoor.SetPoint2(trackWallsOrdered[i + 1].GetCentre());

                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWallsOrdered[i]},
                         {trainDoor.GetPoint2(), trackWallsOrdered[i + 1]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);

                    Wall newWall1 = {trackWallsOrdered[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWallsOrdered[i + 1].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));
                    REQUIRE(removedWalls.size() == 2);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWallsOrdered[i]));
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWallsOrdered[i + 1]));
                }
            }

            SECTION("Door on not neighboring wall elements")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    for(size_t j = 0; j < trackWalls.size(); ++j) {
                        if(std::abs(static_cast<int>(i) - static_cast<int>(j)) <= 1) {
                            continue;
                        }
                        Transition trainDoor;
                        trainDoor.SetPoint1(trackWallsOrdered[i].GetCentre());
                        trainDoor.SetPoint2(trackWallsOrdered[j].GetCentre());

                        std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>>
                            wallDoorIntersection = {
                                {trainDoor.GetPoint1(), trackWallsOrdered[i]},
                                {trainDoor.GetPoint2(), trackWallsOrdered[j]}};

                        auto [addedWalls, removedWalls] = geometry::helper::SplitWall(
                            wallDoorIntersection, trackWalls, trainDoor);

                        Wall newWall1, newWall2;

                        if(i < j) {
                            newWall1 = {trackWallsOrdered[i].GetPoint1(), trainDoor.GetPoint1()};
                            newWall2 = {trackWallsOrdered[j].GetPoint2(), trainDoor.GetPoint2()};
                        } else {
                            newWall1 = {trackWallsOrdered[j].GetPoint1(), trainDoor.GetPoint2()};
                            newWall2 = {trackWallsOrdered[i].GetPoint2(), trainDoor.GetPoint1()};
                        }

                        REQUIRE(addedWalls.size() == 2);
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));

                        unsigned int numRemovedWalls =
                            1 + std::abs(static_cast<int>(i) - static_cast<int>(j));
                        REQUIRE(removedWalls.size() == numRemovedWalls);

                        for(auto k = std::min(i, j); k <= std::max(i, j); ++k) {
                            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWallsOrdered[k]));
                        }
                    }
                }
            }
        }
    }

    SECTION("Curved track")
    {
        SECTION("ordered track walls")
        {
            Wall trackWall1{{-10., -10.}, {-8., -8.}};
            Wall trackWall2{{-8., -8.}, {-6., -7.}};
            Wall trackWall3{{-6., -7.}, {-4., -2.}};
            Wall trackWall4{{-4., -2.}, {-2., -0.}};
            Wall trackWall5{{-2., 0.}, {0., 0.}};

            std::vector<Wall> trackWalls{
                trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

            SECTION("Door on one wall element")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    Point doorVector{trackWalls[i].GetPoint2() - trackWalls[i].GetPoint1()};
                    Point doorVectorNormalized{doorVector.Normalized()};

                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWalls[i].GetCentre() - doorVectorNormalized * 0.5);
                    trainDoor.SetPoint2(trackWalls[i].GetCentre() + doorVectorNormalized * 0.5);
                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWalls[i]},
                         {trainDoor.GetPoint2(), trackWalls[i]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);


                    Wall newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWalls[i].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));
                    REQUIRE(removedWalls.size() == 1);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i]));
                }
            }

            SECTION("Door on neighboring wall elements")
            {
                for(size_t i = 0; i < trackWalls.size() - 1; ++i) {
                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWalls[i].GetCentre());
                    trainDoor.SetPoint2(trackWalls[i + 1].GetCentre());

                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWalls[i]},
                         {trainDoor.GetPoint2(), trackWalls[i + 1]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);

                    Wall newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWalls[i + 1].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE(addedWalls[0] == newWall1);
                    REQUIRE(addedWalls[1] == newWall2);
                    REQUIRE(removedWalls.size() == 2);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i]));
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i + 1]));
                }
            }

            SECTION("Door on not neighboring wall elements")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    for(size_t j = 0; j < trackWalls.size(); ++j) {
                        if(std::abs(static_cast<int>(i) - static_cast<int>(j)) <= 1) {
                            continue;
                        }
                        Transition trainDoor;
                        trainDoor.SetPoint1(trackWalls[i].GetCentre());
                        trainDoor.SetPoint2(trackWalls[j].GetCentre());

                        std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>>
                            wallDoorIntersection = {
                                {trainDoor.GetPoint1(), trackWalls[i]},
                                {trainDoor.GetPoint2(), trackWalls[j]}};

                        auto [addedWalls, removedWalls] = geometry::helper::SplitWall(
                            wallDoorIntersection, trackWalls, trainDoor);

                        Wall newWall1, newWall2;

                        if(i < j) {
                            newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                            newWall2 = {trackWalls[j].GetPoint2(), trainDoor.GetPoint2()};
                        } else {
                            newWall1 = {trackWalls[j].GetPoint1(), trainDoor.GetPoint2()};
                            newWall2 = {trackWalls[i].GetPoint2(), trainDoor.GetPoint1()};
                        }

                        REQUIRE(addedWalls.size() == 2);
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));

                        unsigned int numRemovedWalls =
                            1 + std::abs(static_cast<int>(i) - static_cast<int>(j));
                        REQUIRE(removedWalls.size() == numRemovedWalls);

                        for(auto k = std::min(i, j); k <= std::max(i, j); ++k) {
                            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[k]));
                        }
                    }
                }
            }
        }

        SECTION("unordered track walls")
        {
            Wall trackWall1{{-10., -10.}, {-8., -8.}};
            Wall trackWall2{{-8., -8.}, {-6., -7.}};
            Wall trackWall3{{-6., -7.}, {-4., -2.}};
            Wall trackWall4{{-4., -2.}, {-2., -0.}};
            Wall trackWall5{{-2., 0.}, {0., 0.}};

            std::vector<Wall> trackWallsOrdered{
                trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

            std::vector<Wall> trackWalls{
                trackWall2, trackWall5, trackWall1, trackWall4, trackWall3};

            std::map<int, int> zuordnung{{0, 1}, {1, 4}, {2, 0}, {3, 3}, {4, 2}};

            SECTION("Door on one wall element")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    Point doorVector{trackWalls[i].GetPoint2() - trackWalls[i].GetPoint1()};
                    Point doorVectorNormalized{doorVector.Normalized()};

                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWalls[i].GetCentre() - doorVectorNormalized * 0.5);
                    trainDoor.SetPoint2(trackWalls[i].GetCentre() + doorVectorNormalized * 0.5);

                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWalls[i]},
                         {trainDoor.GetPoint2(), trackWalls[i]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);

                    Wall newWall1 = {trackWalls[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWalls[i].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));
                    REQUIRE(removedWalls.size() == 1);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWalls[i]));
                }
            }

            SECTION("Door on neighboring wall elements")
            {
                for(size_t i = 0; i < trackWallsOrdered.size() - 1; ++i) {
                    Transition trainDoor;
                    trainDoor.SetPoint1(trackWallsOrdered[i].GetCentre());
                    trainDoor.SetPoint2(trackWallsOrdered[i + 1].GetCentre());

                    std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>> wallDoorIntersection =
                        {{trainDoor.GetPoint1(), trackWallsOrdered[i]},
                         {trainDoor.GetPoint2(), trackWallsOrdered[i + 1]}};

                    auto [addedWalls, removedWalls] =
                        geometry::helper::SplitWall(wallDoorIntersection, trackWalls, trainDoor);

                    Wall newWall1 = {trackWallsOrdered[i].GetPoint1(), trainDoor.GetPoint1()};
                    Wall newWall2 = {trackWallsOrdered[i + 1].GetPoint2(), trainDoor.GetPoint2()};

                    REQUIRE(addedWalls.size() == 2);
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                    REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));
                    REQUIRE(removedWalls.size() == 2);
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWallsOrdered[i]));
                    REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWallsOrdered[i + 1]));
                }
            }

            SECTION("Door on not neighboring wall elements")
            {
                for(size_t i = 0; i < trackWalls.size(); ++i) {
                    for(size_t j = 0; j < trackWalls.size(); ++j) {
                        if(std::abs(static_cast<int>(i) - static_cast<int>(j)) <= 1) {
                            continue;
                        }
                        Transition trainDoor;
                        trainDoor.SetPoint1(trackWallsOrdered[i].GetCentre());
                        trainDoor.SetPoint2(trackWallsOrdered[j].GetCentre());

                        std::pair<std::pair<Point, Wall>, std::pair<Point, Wall>>
                            wallDoorIntersection = {
                                {trainDoor.GetPoint1(), trackWallsOrdered[i]},
                                {trainDoor.GetPoint2(), trackWallsOrdered[j]}};

                        auto [addedWalls, removedWalls] = geometry::helper::SplitWall(
                            wallDoorIntersection, trackWalls, trainDoor);

                        Wall newWall1, newWall2;

                        if(i < j) {
                            newWall1 = {trackWallsOrdered[i].GetPoint1(), trainDoor.GetPoint1()};
                            newWall2 = {trackWallsOrdered[j].GetPoint2(), trainDoor.GetPoint2()};
                        } else {
                            newWall1 = {trackWallsOrdered[j].GetPoint1(), trainDoor.GetPoint2()};
                            newWall2 = {trackWallsOrdered[i].GetPoint2(), trainDoor.GetPoint1()};
                        }

                        REQUIRE(addedWalls.size() == 2);
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall1));
                        REQUIRE_THAT(addedWalls, Catch::VectorContains(newWall2));

                        unsigned int numRemovedWalls =
                            1 + std::abs(static_cast<int>(i) - static_cast<int>(j));
                        REQUIRE(removedWalls.size() == numRemovedWalls);

                        for(auto k = std::min(i, j); k <= std::max(i, j); ++k) {
                            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWallsOrdered[k]));
                        }
                    }
                }
            }
        }
    }
}
