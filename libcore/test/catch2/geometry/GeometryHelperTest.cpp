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
#include <numeric>
#include <utility>
#include <vector>

TEST_CASE(
    "geometry/helper/ComputeTrainDoorCoordinates",
    "[geometry][helper][ComputeTrainDoorCoordinates]")
{
    SECTION("ComputeTrainDoorCoordinates")
    {
        Wall trackWall1{{-10., -10.}, {-8., -8.}};
        Wall trackWall2{{-8., -8.}, {-6., -6.}};
        Wall trackWall3{{-6., -6.}, {-4., -4.}};
        Wall trackWall4{{-4., -4.}, {-2., -2.}};
        Wall trackWall5{{-2., -2.}, {0., 0.}};
        std::vector<Wall> trackWalls{trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};
        double trainStartOffSet = 0.;

        Track track{1, 0, 0, trackWalls};

        SECTION("From beginning")
        {
            SECTION("Doors on separate track wall")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::begin(trackWalls); wallItr != std::end(trackWalls);
                    ++wallItr) {
                    double width    = 0.5 * wallItr->GetLength();
                    double distance = std::accumulate(
                        std::begin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.25 * wallItr->GetLength();
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }
                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, false);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::begin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint1(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Doors on neighboring track walls")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::begin(trackWalls); wallItr != std::end(trackWalls) - 1;
                    ++wallItr) {
                    double distance = std::accumulate(
                        std::begin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.25 * wallItr->GetLength();
                    double distanceDoorEnd = std::accumulate(
                        std::begin(trackWalls),
                        std::next(wallItr),
                        0.,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceDoorEnd += 0.25 * std::next(wallItr)->GetLength();
                    double width = distanceDoorEnd - distance;
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }
                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, false);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::begin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint1(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Doors not on neighboring track walls")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(size_t i = 0; i < trainDoors.size() - 2; i += 2) {
                    auto wallItr = std::begin(trackWalls);
                    std::advance(wallItr, i);
                    double distance = std::accumulate(
                        std::begin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.5 * wallItr->GetLength();
                    std::advance(wallItr, 2);

                    double distanceDoorEnd = std::accumulate(
                        std::begin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distanceDoorEnd += 0.5 * wallItr->GetLength();
                    double width = distanceDoorEnd - distance;
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }

                TrainType train{"TEST", 20, 10, trainDoors};
                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, false);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::begin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint1(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Door starts or ends on wall points")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(size_t i = 0; i < trainDoors.size() - 2; i += 2) {
                    auto wallItr = std::begin(trackWalls);
                    std::advance(wallItr, i);
                    double distance = std::accumulate(
                        std::begin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += wallItr->GetLength();

                    double width = std::next(wallItr)->GetLength();
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }

                TrainType train{"TEST", 20, 10, trainDoors};
                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, false);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::begin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint1(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("with train start offset")
            {
                trainStartOffSet = trackWall1.GetLength() + 0.25 * trackWall2.GetLength();

                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID      = 0;
                double distance = 0;
                double width    = 0.25 * trackWall2.GetLength();
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                distance = distance + width + 0.25 * trackWall2.GetLength();
                width    = 0.25 * trackWall2.GetLength() + 0.25 * trackWall3.GetLength();
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                distance = distance + width + 0.5 * trackWall3.GetLength();
                width    = 0.25 * trackWall3.GetLength() + trackWall4.GetLength() +
                        0.25 * trackWall5.GetLength();
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, false);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceIs = std::accumulate(
                        std::begin(trackWalls),
                        wallDoorStartItr,
                        0.,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceIs += Distance(wallDoorStartItr->GetPoint1(), doorCoordinate.first);
                    double distanceShould = trainDoors[i]._distance + trainStartOffSet;
                    REQUIRE(distanceIs == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Multiple doors in one wall element")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::begin(trackWalls); wallItr != std::end(trackWalls);
                    ++wallItr) {
                    double width    = 0.3 * wallItr->GetLength();
                    double distance = std::accumulate(
                        std::begin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    trainDoors.emplace(
                        doorID,
                        TrainDoor{doorID, distance + 0.1 * wallItr->GetLength(), width, flow});
                    doorID++;
                    trainDoors.emplace(
                        doorID,
                        TrainDoor{doorID, distance + 0.4 * wallItr->GetLength(), width, flow});
                    doorID++;
                }

                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, false);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::begin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint1(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Train longer than platform")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::begin(trackWalls); wallItr != std::end(trackWalls);
                    ++wallItr) {
                    double width    = 0.5 * wallItr->GetLength();
                    double distance = std::accumulate(
                        std::begin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.25 * wallItr->GetLength();
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }

                // Create door not on track walls
                double distance =
                    std::accumulate(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        0.,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); }) +
                    1.;
                double width = 1.;
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, false);

                REQUIRE(doorCoordinates.size() == trainDoors.size() - 1);
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::begin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint1(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }
        }

        SECTION("from end")
        {
            SECTION("Doors on separate track wall")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::rbegin(trackWalls); wallItr != std::rend(trackWalls);
                    ++wallItr) {
                    double width    = 0.5 * wallItr->GetLength();
                    double distance = std::accumulate(
                        std::rbegin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.25 * wallItr->GetLength();
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }
                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, true);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::rbegin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint2(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Doors on neighboring track walls")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::rbegin(trackWalls); wallItr != std::rend(trackWalls) - 1;
                    ++wallItr) {
                    double distance = std::accumulate(
                        std::rbegin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.25 * wallItr->GetLength();
                    double distanceDoorEnd = std::accumulate(
                        std::rbegin(trackWalls),
                        std::next(wallItr),
                        0.,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceDoorEnd += 0.25 * std::next(wallItr)->GetLength();
                    double width = distanceDoorEnd - distance;
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }
                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, true);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::rbegin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint2(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Doors not on neighboring track walls")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(size_t i = 0; i < trainDoors.size() - 2; i += 2) {
                    auto wallItr = std::rbegin(trackWalls);
                    std::advance(wallItr, i);
                    double distance = std::accumulate(
                        std::rbegin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.5 * wallItr->GetLength();
                    std::advance(wallItr, 2);

                    double distanceDoorEnd = std::accumulate(
                        std::rbegin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distanceDoorEnd += 0.5 * wallItr->GetLength();
                    double width = distanceDoorEnd - distance;
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }

                TrainType train{"TEST", 20, 10, trainDoors};
                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, true);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::rbegin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint2(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Door starts or ends on wall points")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(size_t i = 0; i < trainDoors.size() - 2; i += 2) {
                    auto wallItr = std::rbegin(trackWalls);
                    std::advance(wallItr, i);
                    double distance = std::accumulate(
                        std::rbegin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += wallItr->GetLength();

                    double width = std::next(wallItr)->GetLength();
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }

                TrainType train{"TEST", 20, 10, trainDoors};
                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, true);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::rbegin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint2(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("with train start offset")
            {
                trainStartOffSet = trackWall5.GetLength() + 0.25 * trackWall4.GetLength();

                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;

                double distance = 0;
                double width    = 0.25 * trackWall4.GetLength();
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                distance = distance + width + 0.25 * trackWall4.GetLength();
                width    = 0.25 * trackWall4.GetLength() + 0.25 * trackWall3.GetLength();
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                distance = distance + width + 0.5 * trackWall3.GetLength();
                width    = 0.25 * trackWall3.GetLength() + trackWall2.GetLength() +
                        0.25 * trackWall1.GetLength();
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, true);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceIs = std::accumulate(
                        std::rbegin(trackWalls),
                        wallDoorStartItr,
                        0.,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceIs += Distance(wallDoorStartItr->GetPoint2(), doorCoordinate.first);
                    double distanceShould = trainDoors[i]._distance + trainStartOffSet;
                    REQUIRE(distanceIs == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Multiple doors in one wall element")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::rbegin(trackWalls); wallItr != std::rend(trackWalls);
                    ++wallItr) {
                    double width    = 0.3 * wallItr->GetLength();
                    double distance = std::accumulate(
                        std::rbegin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    trainDoors.emplace(
                        doorID,
                        TrainDoor{doorID, distance + 0.1 * wallItr->GetLength(), width, flow});
                    doorID++;
                    trainDoors.emplace(
                        doorID,
                        TrainDoor{doorID, distance + 0.4 * wallItr->GetLength(), width, flow});
                    doorID++;
                }

                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, true);

                REQUIRE(doorCoordinates.size() == trainDoors.size());
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::rbegin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint2(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }

            SECTION("Train longer than platform")
            {
                double flow = std::numeric_limits<double>::max();

                std::map<int, TrainDoor> trainDoors;
                int doorID = 0;
                for(auto wallItr = std::rbegin(trackWalls); wallItr != std::rend(trackWalls);
                    ++wallItr) {
                    double width    = 0.5 * wallItr->GetLength();
                    double distance = std::accumulate(
                        std::rbegin(trackWalls), wallItr, 0., [](double & sum, const Wall & wall) {
                            return sum + wall.GetLength();
                        });
                    distance += 0.25 * wallItr->GetLength();
                    trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                    doorID++;
                }

                // Create door not on track walls
                double distance =
                    std::accumulate(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        0.,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); }) +
                    1.;
                double width = 1.;
                trainDoors.emplace(doorID, TrainDoor{doorID, distance, width, flow});
                doorID++;

                TrainType train{"TEST", 20, 10, trainDoors};

                auto doorCoordinates = geometry::helper::ComputeTrainDoorCoordinates(
                    train, track, trainStartOffSet, true);

                REQUIRE(doorCoordinates.size() == trainDoors.size() - 1);
                for(size_t i = 0; i < doorCoordinates.size(); ++i) {
                    // check distance to start
                    auto doorCoordinate   = doorCoordinates[i];
                    auto wallDoorStartItr = std::find_if(
                        std::rbegin(trackWalls),
                        std::rend(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    double distanceShould = std::accumulate(
                        std::rbegin(trackWalls),
                        wallDoorStartItr,
                        trainStartOffSet,
                        [](double & sum, const Wall & wall) { return sum + wall.GetLength(); });
                    distanceShould += Distance(wallDoorStartItr->GetPoint2(), doorCoordinate.first);
                    REQUIRE(trainDoors[i]._distance == Approx(distanceShould));

                    // check distance of door elements
                    REQUIRE(
                        trainDoors[i]._width ==
                        Approx(Distance(doorCoordinate.first, doorCoordinate.second)));

                    // check if on any of these lines
                    auto wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.first);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));

                    wallPointItr = std::find_if(
                        std::begin(trackWalls),
                        std::end(trackWalls),
                        [&doorCoordinate](const Wall & wall) {
                            return wall.IsInLineSegment(doorCoordinate.second);
                        });
                    REQUIRE_FALSE(wallPointItr == std::end(trackWalls));
                }
            }
        }
    }
}

TEST_CASE("geometry/helper/SortTrackWalls", "[geometry][helper][SortWalls]")
{
    SECTION("straight walls")
    {
        Wall trackWall1{{-10., -10.}, {-8., -8.}};
        Wall trackWall2{{-6., -6.}, {-8., -8.}};
        Wall trackWall3{{-6., -6.}, {-4., -4.}};
        Wall trackWall4{{-2., -2.}, {-4., -4.}};
        Wall trackWall5{{-2., -2.}, {0., 0.}};

        std::vector<Wall> trackWallsOrdered{
            trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

        Point trackStart = trackWall1.GetPoint1();

        SECTION("one wall")
        {
            std::vector<Wall> trackWallOne{trackWall1};
            std::vector<Wall> trackWalls{trackWall1};
            REQUIRE_NOTHROW(geometry::helper::SortWalls(trackWalls, trackStart));
            CHECK_THAT(trackWalls, Catch::Equals(trackWallOne));
            REQUIRE(trackWallOne.begin()->GetPoint1() == trackStart);
        }

        SECTION("continuous wall")
        {
            std::vector<Wall> trackWalls{
                trackWall2, trackWall5, trackWall1, trackWall4, trackWall3};

            REQUIRE_NOTHROW(geometry::helper::SortWalls(trackWalls, trackStart));
            CHECK_THAT(trackWalls, Catch::Equals(trackWallsOrdered));
            for(auto wallItr = std::begin(trackWalls); wallItr != std::end(trackWalls) - 1;
                ++wallItr) {
                REQUIRE(wallItr->GetPoint2() == std::next(wallItr)->GetPoint1());
            }


            std::vector<Wall> trackWallsReversed{
                trackWall5, trackWall4, trackWall3, trackWall2, trackWall1};

            REQUIRE_NOTHROW(geometry::helper::SortWalls(trackWallsReversed, trackStart));
            CHECK_THAT(trackWallsReversed, Catch::Equals(trackWallsOrdered));
            for(auto wallItr = std::begin(trackWallsReversed);
                wallItr != std::end(trackWallsReversed) - 1;
                ++wallItr) {
                REQUIRE(wallItr->GetPoint2() == std::next(wallItr)->GetPoint1());
            }
        }

        SECTION("non-continuous wall")
        {
            std::vector<Wall> trackWalls{trackWall5, trackWall1, trackWall4, trackWall3};

            REQUIRE_THROWS_WITH(
                geometry::helper::SortWalls(trackWalls, trackStart),
                "Track walls could not be sorted. Could not find a wall succeeding ( -10 : -10 "
                ")--( -8 : -8 ) in track walls. Please check your geometry");
        }

        SECTION("Start not on track wall")
        {
            std::vector<Wall> trackWalls{
                trackWall2, trackWall5, trackWall1, trackWall4, trackWall3};
            Point start{-25., 10.};

            REQUIRE_THROWS_WITH(
                geometry::helper::SortWalls(trackWalls, start),
                "Track walls could not be sorted. Start ( -25 : 10 ) is not on one of the track "
                "walls. Please check your geometry.");
        }
    }

    SECTION("curved walls")
    {
        Wall trackWall1{{-8., -8.}, {-10., -10.}};
        Wall trackWall2{{-8., -8.}, {-6., -7.}};
        Wall trackWall3{{-6., -7.}, {-4., -2.}};
        Wall trackWall4{{-2., -0.}, {-4., -2.}};
        Wall trackWall5{{0., 0.}, {-2., 0.}};

        std::vector<Wall> trackWallsOrdered{
            trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

        Point trackStart = trackWall1.GetPoint2();

        SECTION("continuous wall")
        {
            std::vector<Wall> trackWalls{
                trackWall2, trackWall5, trackWall1, trackWall4, trackWall3};

            REQUIRE_NOTHROW(geometry::helper::SortWalls(trackWalls, trackStart));
            CHECK_THAT(trackWalls, Catch::Equals(trackWallsOrdered));
            for(auto wallItr = std::begin(trackWalls); wallItr != std::end(trackWalls) - 1;
                ++wallItr) {
                REQUIRE(wallItr->GetPoint2() == std::next(wallItr)->GetPoint1());
            }

            std::vector<Wall> trackWallsReversed{
                trackWall5, trackWall4, trackWall3, trackWall2, trackWall1};

            REQUIRE_NOTHROW(geometry::helper::SortWalls(trackWallsReversed, trackStart));
            CHECK_THAT(trackWallsReversed, Catch::Equals(trackWallsOrdered));
            for(auto wallItr = std::begin(trackWallsReversed);
                wallItr != std::end(trackWallsReversed) - 1;
                ++wallItr) {
                REQUIRE(wallItr->GetPoint2() == std::next(wallItr)->GetPoint1());
            }
        }
    }
}

TEST_CASE(
    "geometry/helper/FindWallPointWithDistanceOnWall",
    "[geometry][helper][FindWallPointWithDistanceOnWall]")
{
    SECTION("Error handling")
    {
        Wall trackWall1{{-10., -10.}, {-8., -8.}};
        Wall trackWall2{{-8., -8.}, {-6., -6.}};
        Wall trackWall3{{-6., -6.}, {-4., -4.}};
        Wall trackWall4{{-4., -4.}, {-2., -2.}};
        Wall trackWall5{{-2., -2.}, {0., 0.}};

        std::vector<Wall> trackWalls{trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

        SECTION("trackWalls empty")
        {
            auto result = geometry::helper::FindWallPointWithDistanceOnWall(
                std::vector<Wall>{}, Point{0, 0}, 0);
            REQUIRE_FALSE(result.has_value());
        }

        SECTION("starting point not in track walls")
        {
            std::pair<Point, Wall> start{{-5., -5.}, {{-6., -5.}, {-4., -5.}}};
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, Point{0, 0}, 0);
            REQUIRE_FALSE(result.has_value());
        }

        SECTION("distance too large")
        {
            std::pair<Point, Wall> start{{-5., -5.}, {{-6., -5.}, {-4., -5.}}};
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, Point{0, 0}, 20.);
            REQUIRE_FALSE(result.has_value());
        }
    }

    SECTION("straight walls")
    {
        Wall trackWall1{{-10., -10.}, {-8., -8.}};
        Wall trackWall2{{-8., -8.}, {-6., -6.}};
        Wall trackWall3{{-6., -6.}, {-4., -4.}};
        Wall trackWall4{{-4., -4.}, {-2., -2.}};
        Wall trackWall5{{-2., -2.}, {0., 0.}};
        std::vector<Wall> trackWalls{trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

        SECTION("point on same wall")
        {
            Point start{-5., -5.};
            double distance = 0.5;
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, start, distance);
            REQUIRE(result.has_value());
            auto point = result.value();
            REQUIRE(Distance(start, point) == Approx(distance));
            REQUIRE(trackWall3.IsInLineSegment(point));
        }

        SECTION("point on neighbor wall")
        {
            Point start{-5., -5.};
            double distance = 2.;
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, start, distance);
            REQUIRE(result.has_value());
            auto point = result.value();
            REQUIRE(trackWall4.IsInLineSegment(point));
            double computedDistance = 0.;
            Point startPoint        = start;
            auto startItr           = std::find_if(
                std::begin(trackWalls), std::end(trackWalls), [&start](const Wall & wall) {
                    return wall.IsInLineSegment(start) && wall.GetPoint2() != start;
                });

            for(auto wallItr = startItr; wallItr != std::end(trackWalls); ++wallItr) {
                if(wallItr->IsInLineSegment(point)) {
                    computedDistance += Distance(startPoint, point);
                    break;
                }
                computedDistance += Distance(startPoint, wallItr->GetPoint2());
                startPoint = wallItr->GetPoint2();
            }
            REQUIRE(computedDistance == Approx(distance));
        }

        SECTION("point on not neighbor wall")
        {
            Point start{-5., -5.};
            double distance = 5.;
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, start, distance);

            REQUIRE(result.has_value());

            auto point = result.value();
            REQUIRE(trackWall5.IsInLineSegment(point));

            double computedDistance = 0.;
            Point startPoint        = start;
            auto startItr           = std::find_if(
                std::begin(trackWalls), std::end(trackWalls), [&start](const Wall & wall) {
                    return wall.IsInLineSegment(start) && wall.GetPoint2() != start;
                });
            for(auto wallItr = startItr; wallItr != std::end(trackWalls); ++wallItr) {
                if(wallItr->IsInLineSegment(point)) {
                    computedDistance += Distance(startPoint, point);
                    break;
                }
                computedDistance += Distance(startPoint, wallItr->GetPoint2());
                startPoint = wallItr->GetPoint2();
            }
            REQUIRE(computedDistance == Approx(distance));
        }
    }

    SECTION("curved walls")
    {
        Wall trackWall1{{-10., -10.}, {-8., -8.}};
        Wall trackWall2{{-8., -8.}, {-6., -7.}};
        Wall trackWall3{{-6., -7.}, {-4., -2.}};
        Wall trackWall4{{-4., -2.}, {-2., -0.}};
        Wall trackWall5{{-2., 0.}, {0., 0.}};

        std::vector<Wall> trackWalls{trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

        SECTION("point on same wall")
        {
            Point start{-5., -4.5};
            double distance = 0.5;
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, start, distance);
            REQUIRE(result.has_value());
            auto point = result.value();
            REQUIRE(Distance(start, point) == Approx(distance));
            REQUIRE(trackWall3.IsInLineSegment(point));
        }

        SECTION("point on neighbor wall")
        {
            Point start{-5., -4.5};
            double distance = 4.;
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, start, distance);
            REQUIRE(result.has_value());
            auto point = result.value();
            REQUIRE(trackWall4.IsInLineSegment(point));
            double computedDistance = 0.;
            Point startPoint        = start;
            auto startItr           = std::find_if(
                std::begin(trackWalls), std::end(trackWalls), [&start](const Wall & wall) {
                    return wall.IsInLineSegment(start) && wall.GetPoint2() != start;
                });
            for(auto wallItr = startItr; wallItr != std::end(trackWalls); ++wallItr) {
                if(wallItr->IsInLineSegment(point)) {
                    computedDistance += Distance(startPoint, point);
                    break;
                }
                computedDistance += Distance(startPoint, wallItr->GetPoint2());
                startPoint = wallItr->GetPoint2();
            }
            REQUIRE(computedDistance == Approx(distance));
        }

        SECTION("point on not neighbor wall")
        {
            Point start{-5., -4.5};
            double distance = 7.;
            auto result =
                geometry::helper::FindWallPointWithDistanceOnWall(trackWalls, start, distance);

            REQUIRE(result.has_value());

            auto point = result.value();
            REQUIRE(trackWall5.IsInLineSegment(point));

            double computedDistance = 0.;
            Point startPoint        = start;
            auto startItr           = std::find_if(
                std::begin(trackWalls), std::end(trackWalls), [&start](const Wall & wall) {
                    return wall.IsInLineSegment(start) && wall.GetPoint2() != start;
                });
            for(auto wallItr = startItr; wallItr != std::end(trackWalls); ++wallItr) {
                if(wallItr->IsInLineSegment(point)) {
                    computedDistance += Distance(startPoint, point);
                    break;
                }
                computedDistance += Distance(startPoint, wallItr->GetPoint2());
                startPoint = wallItr->GetPoint2();
            }
            REQUIRE(computedDistance == Approx(distance));
        }
    }
}

TEST_CASE("geometry/helper/SplitWalls", "[geometry][helper][SplitWalls]")
{
    SECTION("on one element")
    {
        Wall trackWall{{-10., -10.}, {10., -10.}};
        std::vector<Wall> trackWalls{trackWall};

        SECTION("One door on one wall element")
        {
            Point point1{-1., -10.};
            Point point2{1., -10.};

            Transition trainDoor;
            trainDoor.SetPoint1(point1);
            trainDoor.SetPoint2(point2);
            std::vector<Transition> trainDoors{trainDoor};

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall));

            REQUIRE(addedWalls.size() == 2);
            Wall wallSplit1{trackWall.GetPoint1(), point1};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit1));
            Wall wallSplit2{point2, trackWall.GetPoint2()};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit2));
        }

        SECTION("door on start of wall")
        {
            Point point1{-10., -10.};
            Point point2{-8., -10.};

            Transition trainDoor;
            trainDoor.SetPoint1(point1);
            trainDoor.SetPoint2(point2);
            std::vector<Transition> trainDoors{trainDoor};

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall));

            REQUIRE(addedWalls.size() == 1);
            Wall wallSplit{point2, trackWall.GetPoint2()};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit));
        }

        SECTION("door on end of wall")
        {
            Point point1{8., -10.};
            Point point2{10., -10.};

            Transition trainDoor;
            trainDoor.SetPoint1(point1);
            trainDoor.SetPoint2(point2);
            std::vector<Transition> trainDoors{trainDoor};

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall));

            REQUIRE(addedWalls.size() == 1);
            Wall wallSplit{trackWall.GetPoint1(), point1};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit));
        }

        SECTION("Multiple doors on one wall element")
        {
            Transition trainDoor1;
            trainDoor1.SetPoint1({-7., -10.});
            trainDoor1.SetPoint2({-5., -10.});

            Transition trainDoor2;
            trainDoor2.SetPoint1({-1., -10.});
            trainDoor2.SetPoint2({1., -10.});

            Transition trainDoor3;
            trainDoor3.SetPoint1({5., -10.});
            trainDoor3.SetPoint2({7., -10.});

            std::vector<Transition> trainDoors{trainDoor1, trainDoor2, trainDoor3};

            std::vector<Wall> splitWalls{
                {trackWall.GetPoint1(), trainDoor1.GetPoint1()},
                {trainDoor1.GetPoint2(), trainDoor2.GetPoint1()},
                {trainDoor2.GetPoint2(), trainDoor3.GetPoint1()},
                {trainDoor3.GetPoint2(), trackWall.GetPoint2()}};

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall));

            REQUIRE(addedWalls.size() == 4);
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(splitWalls));
        }
    }

    SECTION("straight wall, multiple elements")
    {
        Wall trackWall1{{-10., -10.}, {-8., -10.}};
        Wall trackWall2{{-8., -10.}, {-6., -10.}};
        Wall trackWall3{{-6., -10.}, {-4., -10.}};
        Wall trackWall4{{-4., -10.}, {-2., -10.}};
        Wall trackWall5{{-2., -10.}, {0., -10.}};

        std::vector<Wall> trackWalls{trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

        SECTION("One door on one wall element")
        {
            Point point1{-9.5, -10.};
            Point point2{-8.5, -10.};

            Transition trainDoor;
            trainDoor.SetPoint1(point1);
            trainDoor.SetPoint2(point2);
            std::vector<Transition> trainDoors{trainDoor};

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall1));

            REQUIRE(addedWalls.size() == 2);
            Wall wallSplit1{trackWall1.GetPoint1(), point1};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit1));
            Wall wallSplit2{point2, trackWall1.GetPoint2()};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit2));
        }

        SECTION("One door on neighboring wall element")
        {
            Point point1{-9.5, -10.};
            Point point2{-7.5, -10.};

            Transition trainDoor;
            trainDoor.SetPoint1(point1);
            trainDoor.SetPoint2(point2);
            std::vector<Transition> trainDoors{trainDoor};

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 2);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall1));
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall2));

            REQUIRE(addedWalls.size() == 2);
            Wall wallSplit1{trackWall1.GetPoint1(), point1};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit1));
            Wall wallSplit2{point2, trackWall2.GetPoint2()};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit2));
        }

        SECTION("One door ends on wall endpoint")
        {
            std::vector<Wall> wallSplit;
            std::vector<Transition> trainDoors;

            Transition door;
            door.SetPoint1(trackWall2.GetCentre());
            door.SetPoint2(trackWall2.GetPoint2());
            trainDoors.emplace_back(door);
            wallSplit.emplace_back(Wall{trackWall2.GetPoint1(), trackWall2.GetCentre()});

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall2));

            REQUIRE(addedWalls.size() == 1);
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(wallSplit));
        }

        SECTION("Multiple doors end on wall endpoint")
        {
            std::vector<Wall> wallSplit;
            std::vector<Transition> trainDoors;
            for(const auto & wall : trackWalls) {
                Transition door;
                door.SetPoint1(wall.GetCentre());
                door.SetPoint2(wall.GetPoint2());
                trainDoors.emplace_back(door);
                wallSplit.emplace_back(Wall{wall.GetPoint1(), wall.GetCentre()});
            }

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == trackWalls.size());
            REQUIRE_THAT(removedWalls, Catch::Matchers::UnorderedEquals(trackWalls));

            REQUIRE(addedWalls.size() == wallSplit.size());
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(wallSplit));
        }

        SECTION("One door starts on wall endpoint")
        {
            std::vector<Wall> wallSplit;
            std::vector<Transition> trainDoors;

            Transition door;
            door.SetPoint1(trackWall2.GetPoint1());
            door.SetPoint2(trackWall2.GetCentre());
            trainDoors.emplace_back(door);
            wallSplit.emplace_back(Wall{trackWall2.GetCentre(), trackWall2.GetPoint2()});

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall2));

            REQUIRE(addedWalls.size() == wallSplit.size());
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(wallSplit));
        }

        SECTION("One door on not neighboring wall element")
        {
            Point point1{-9.5, -10.};
            Point point2{-0.5, -10.};

            Transition trainDoor;
            trainDoor.SetPoint1(point1);
            trainDoor.SetPoint2(point2);
            std::vector<Transition> trainDoors{trainDoor};

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);
            REQUIRE(removedWalls.size() == 5);
            REQUIRE_THAT(removedWalls, Catch::Matchers::UnorderedEquals(trackWalls));

            REQUIRE(addedWalls.size() == 2);
            Wall wallSplit1{trackWall1.GetPoint1(), point1};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit1));
            Wall wallSplit2{point2, trackWall5.GetPoint2()};
            REQUIRE_THAT(addedWalls, Catch::VectorContains(wallSplit2));
        }

        SECTION("Multiple doors on one element")
        {
            Transition trainDoor1;
            trainDoor1.SetPoint1({-9.75, -10.});
            trainDoor1.SetPoint2({-9.25, -10.});

            Transition trainDoor2;
            trainDoor2.SetPoint1({-8.75, -10.});
            trainDoor2.SetPoint2({-8.25, -10.});

            std::vector<Transition> trainDoors{trainDoor1, trainDoor2};

            std::vector<Wall> shouldAdded;
            shouldAdded.emplace_back(Wall{trackWall1.GetPoint1(), trainDoor1.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor1.GetPoint2(), trainDoor2.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor2.GetPoint2(), trackWall1.GetPoint2()});

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);

            REQUIRE(removedWalls.size() == 1);
            REQUIRE_THAT(removedWalls, Catch::VectorContains(trackWall1));

            REQUIRE(addedWalls.size() == shouldAdded.size());
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(shouldAdded));
        }

        SECTION("Multiple doors on neighboring elements")
        {
            Transition trainDoor1;
            trainDoor1.SetPoint1(trackWall1.GetCentre());
            trainDoor1.SetPoint2(trackWall2.GetCentre());

            Transition trainDoor2;
            trainDoor2.SetPoint1(trackWall3.GetCentre());
            trainDoor2.SetPoint2(trackWall4.GetCentre());

            std::vector<Transition> trainDoors{trainDoor1, trainDoor2};

            std::vector<Wall> shouldRemoved{trackWall1, trackWall2, trackWall3, trackWall4};
            std::vector<Wall> shouldAdded;
            shouldAdded.emplace_back(Wall{trackWall1.GetPoint1(), trainDoor1.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor1.GetPoint2(), trackWall2.GetPoint2()});
            shouldAdded.emplace_back(Wall{trackWall3.GetPoint1(), trainDoor2.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor2.GetPoint2(), trackWall4.GetPoint2()});

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);

            REQUIRE(removedWalls.size() == shouldRemoved.size());
            REQUIRE_THAT(removedWalls, Catch::Matchers::UnorderedEquals(shouldRemoved));

            REQUIRE(addedWalls.size() == shouldAdded.size());
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(shouldAdded));
        }

        SECTION("Multiple doors on not neighboring elements")
        {
            Transition trainDoor1;
            trainDoor1.SetPoint1({-8.5, -10.});
            trainDoor1.SetPoint2({-5.5, -10.});

            Transition trainDoor2;
            trainDoor2.SetPoint1({-4.5, -10.});
            trainDoor2.SetPoint2({-0.5, -10.});

            std::vector<Transition> trainDoors{trainDoor1, trainDoor2};

            std::vector<Wall> shouldAdded;
            shouldAdded.emplace_back(Wall{trackWall1.GetPoint1(), trainDoor1.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor1.GetPoint2(), trainDoor2.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor2.GetPoint2(), trackWall5.GetPoint2()});

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);

            REQUIRE(removedWalls.size() == 5);
            REQUIRE_THAT(removedWalls, Catch::Matchers::UnorderedEquals(trackWalls));

            REQUIRE(addedWalls.size() == shouldAdded.size());
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(shouldAdded));
        }

        SECTION("mix all")
        {
            Transition trainDoor1;
            trainDoor1.SetPoint1({-9.5, -10.});
            trainDoor1.SetPoint2({-9., -10.});

            Transition trainDoor2;
            trainDoor2.SetPoint1({-8.5, -10.});
            trainDoor2.SetPoint2({-7.5, -10.});

            Transition trainDoor3;
            trainDoor3.SetPoint1({-6., -10.});
            trainDoor3.SetPoint2({-1.5, -10.});

            Transition trainDoor4;
            trainDoor4.SetPoint1({-1., -10.});
            trainDoor4.SetPoint2({-0., -10.});

            std::vector<Transition> trainDoors{trainDoor1, trainDoor2, trainDoor3, trainDoor4};

            std::vector<Wall> shouldAdded;
            shouldAdded.emplace_back(Wall{trackWall1.GetPoint1(), trainDoor1.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor1.GetPoint2(), trainDoor2.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor2.GetPoint2(), trainDoor3.GetPoint1()});
            shouldAdded.emplace_back(Wall{trainDoor3.GetPoint2(), trainDoor4.GetPoint1()});

            auto [addedWalls, removedWalls] = geometry::helper::SplitWalls(trackWalls, trainDoors);

            REQUIRE(removedWalls.size() == 5);
            REQUIRE_THAT(removedWalls, Catch::Matchers::UnorderedEquals(trackWalls));

            REQUIRE(addedWalls.size() == shouldAdded.size());
            REQUIRE_THAT(addedWalls, Catch::Matchers::UnorderedEquals(shouldAdded));
        }
    }

    SECTION("error handling")
    {
        Wall trackWall1{{-10., -10.}, {-8., -10.}};
        Wall trackWall2{{-8., -10.}, {-6., -10.}};
        Wall trackWall3{{-6., -10.}, {-4., -10.}};
        Wall trackWall4{{-4., -10.}, {-2., -10.}};
        Wall trackWall5{{-2., -10.}, {0., -10.}};

        std::vector<Wall> trackWalls{trackWall1, trackWall2, trackWall3, trackWall4, trackWall5};

        SECTION("door start not on walls")
        {
            Transition trainDoor;
            trainDoor.SetPoint1({-12, -10.});
            trainDoor.SetPoint2({-10., -10.});

            std::vector<Transition> trainDoors{trainDoor};
            REQUIRE_THROWS_WITH(
                geometry::helper::SplitWalls(trackWalls, trainDoors),
                "Point ( -12 : -10 ) does not belong to any track walls.");
        }

        SECTION("door end not on walls")
        {
            Transition trainDoor;
            trainDoor.SetPoint1({-1., -10.});
            trainDoor.SetPoint2({1., -10.});

            std::vector<Transition> trainDoors{trainDoor};
            REQUIRE_THROWS_WITH(
                geometry::helper::SplitWalls(trackWalls, trainDoors),
                "Point ( 1 : -10 ) does not belong to any track walls.");
        }
    }
}