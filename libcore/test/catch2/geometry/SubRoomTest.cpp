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

#include "geometry/SubRoom.h"

#include "IO/OutputHandler.h"
#include "geometry/Crossing.h"
#include "geometry/Line.h"
#include "geometry/Point.h"
#include "geometry/Transition.h"
#include "geometry/Wall.h"

#include <catch2/catch.hpp>
#include <vector>

class NullOutputHandler : public OutputHandler
{
public:
    void Write(const std::string &) override{};
    void Write(const char *, ...) override{};
};

NullOutputHandler handler{};
OutputHandler * Log = &handler;


TEST_CASE("geometry/SubRoom", "[geometry][SubRoom]")
{
    SECTION("Small Gap between Walls")
    {
        NormalSubRoom sub1;
        sub1.SetSubRoomID(1);
        sub1.SetRoomID(1);

        Point P1(00.0, 00);
        Point P2(00.0, 10);
        Point P3(05.0, 10);
        Point P4(5.25, 10);
        Point P5(5.27, 10);
        Point P6(5.31, 10);
        Point P7(5.32, 10);
        Point P8(5.36, 10);
        Point P9(5.38, 10);
        Point P10(5.50, 10);
        Point P11(5.52, 10);
        Point P12(10.0, 10);
        Point P13(10.0, 05);
        Point P14(10.0, 00);

        // Walls with gap less than 0.03 were created
        sub1.AddWall(Wall(P1, P2));
        sub1.AddWall(Wall(P2, P3));
        sub1.AddWall(Wall(P3, P4));
        sub1.AddWall(Wall(P5, P6));
        sub1.AddWall(Wall(P7, P8));
        sub1.AddWall(Wall(P9, P10));
        sub1.AddWall(Wall(P11, P12));
        sub1.AddWall(Wall(P12, P13));
        sub1.AddWall(Wall(P14, P1));

        Line exit(P14, P13);

        std::vector<Line *> goal; // (Line(Point(10, 5), Point(10, 0)));
        goal.push_back(&exit);


        if(sub1.ConvertLineToPoly(goal) == true) {
            std::vector<Point> poly = sub1.GetPolygon();
            REQUIRE(poly.size() == 10);
            // TODO: Test the points of the Polygon
        } else {
            REQUIRE(false); // unreachable
        }
    }

    SECTION("Small Walls")
    {
        NormalSubRoom sub;
        sub.SetSubRoomID(1);
        sub.SetRoomID(1);

        Point P1(0, 0);
        Point P2(0, 10);
        Point P3(5.0, 10);
        Point P4(5.029, 10);
        Point P5(10, 10);
        Point P6(10, 5);
        Point P7(10, 0);


        REQUIRE(sub.AddWall(Wall(P1, P2)));
        REQUIRE(sub.AddWall(Wall(P2, P3)));
        REQUIRE_FALSE(sub.AddWall(Wall(P3, P4)));
        REQUIRE(sub.AddWall(Wall(P4, P5)));
        REQUIRE(sub.AddWall(Wall(P5, P6)));
        REQUIRE(sub.AddWall(Wall(P1, P7)));
    }

    SECTION("Overlapping Walls")
    {
        NormalSubRoom sub;
        sub.SetSubRoomID(1);
        sub.SetRoomID(1);

        Point P1(0, 0);
        Point P2(0, 10);
        Point P3(10, 10);
        Point P4(10, 8);
        Point P5(10, 0);
        Point P6(10, 5);

        sub.AddWall(Wall(P1, P2));
        sub.AddWall(Wall(P5, P1));
        sub.AddWall(Wall(P3, P6));
        sub.AddWall(Wall(P2, P3));
        sub.AddWall(Wall(P5, P6));

        Line exit(P6, P4);
        std::vector<Line *> door; // door overlaps with the wall
        door.push_back(&exit);

        REQUIRE_FALSE(sub.ConvertLineToPoly(door));

        NormalSubRoom sub2;
        sub2.SetSubRoomID(2);
        sub2.SetRoomID(2);

        sub2.AddWall(Wall(P1, P2));
        sub2.AddWall(Wall(P2, P3));
        sub2.AddWall(Wall(P3, P6)); // Overlapping Walls
        sub2.AddWall(Wall(P5, P1));
        sub2.AddWall(Wall(P4, P6)); // Overlapping Walls

        REQUIRE_FALSE(sub.ConvertLineToPoly(door));
    }

    SECTION("Concave Intersecting Walls")
    {
        Point P1;
        Point P2(2, 2);
        Point P3(2, 0);
        Point P4(0, 2);

        NormalSubRoom sub1;
        sub1.SetSubRoomID(1);
        sub1.SetRoomID(1);
        sub1.AddWall(Wall(P1, P2));
        sub1.AddWall(Wall(P2, P3));
        sub1.AddWall(Wall(P3, P4));

        Line exit(P1, P4);
        std::vector<Line *> door;
        door.push_back(&exit);

        REQUIRE_FALSE(sub1.ConvertLineToPoly(door));
    }
}

TEST_CASE(
    "geometry/SubRoom/IsPointOnPolygonBoundaries",
    "[geometry][SubRoom][IsPointOnPolygonBoundaries]")
{
    SECTION("Is NOT on boundaries")
    {
        NormalSubRoom subroom;
        subroom.AddWall(Wall(Point(0, 10), Point(0, 0)));

        SECTION("No on Wall")
        {
            REQUIRE_FALSE(subroom.IsPointOnPolygonBoundaries(Point(-1, 1), Wall()));
            REQUIRE_FALSE(subroom.IsPointOnPolygonBoundaries(Point(-1, -1), Wall()));
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(-1, 1), Wall(Point(0, 10), Point(0, 0))));
        }
        SECTION("On excluded Wall")
        {
            // Wall Excluded
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 10), Wall(Point(0, 10), Point(0, 0))));
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 9), Wall(Point(0, 10), Point(0, 0))));
        }

        SECTION("With Crossing")
        {
            Crossing crossing;
            crossing.SetPoint1(Point(10, 10));
            crossing.SetPoint2(Point(0, 10));
            subroom.AddCrossing(&crossing);

            REQUIRE_FALSE(subroom.IsPointOnPolygonBoundaries(Point(-1, 1), Wall()));
            REQUIRE_FALSE(subroom.IsPointOnPolygonBoundaries(Point(-1, -1), Wall()));
            // exclude wall
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(-1, 1), Wall(Point(0, 10), Point(0, 0))));
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 0), Wall(Point(0, 10), Point(0, 0))));
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 9), Wall(Point(0, 10), Point(0, 0))));
        }
        SECTION("With Transition")
        {
            Transition transition;
            transition.SetPoint1(Point(10, 10));
            transition.SetPoint2(Point(0, 10));
            subroom.AddTransition(&transition);

            REQUIRE_FALSE(subroom.IsPointOnPolygonBoundaries(Point(-1, 1), Wall()));
            REQUIRE_FALSE(subroom.IsPointOnPolygonBoundaries(Point(-1, -1), Wall()));
            // exclude wall
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(-1, 1), Wall(Point(0, 10), Point(0, 0))));
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 0), Wall(Point(0, 10), Point(0, 0))));
            REQUIRE_FALSE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 9), Wall(Point(0, 10), Point(0, 0))));
        }
    }
    SECTION("Is on boundaries")
    {
        NormalSubRoom subroom;
        subroom.AddWall(Wall(Point(0, 10), Point(0, 0)));

        SECTION("Without excluded wall")
        {
            REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(0, 0), Wall()));
            REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(0, 10), Wall()));
            REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(0, 3.4), Wall()));

            SECTION("Crossing")
            {
                Crossing crossing;
                crossing.SetPoint1(Point(10, 10));
                crossing.SetPoint2(Point(0, 10));
                subroom.AddCrossing(&crossing);

                REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(10, 10), Wall()));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(0, 10), Wall()));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(2.4, 10), Wall()));
            }

            SECTION("Transition")
            {
                Transition transition;
                transition.SetPoint1(Point(10, 10));
                transition.SetPoint2(Point(0, 10));
                subroom.AddTransition(&transition);

                REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(10, 10), Wall()));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(0, 10), Wall()));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(Point(2.4, 10), Wall()));
            }
        }

        SECTION("Wall excluded")
        {
            subroom.AddWall(Wall(Point(0, 10), Point(2, 10)));

            REQUIRE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 0), Wall(Point(0, 10), Point(2, 10))));
            // Wall is excluded, but Point(0,10) is also incident with the other wall
            REQUIRE(
                subroom.IsPointOnPolygonBoundaries(Point(0, 10), Wall(Point(0, 10), Point(2, 10))));
            REQUIRE(subroom.IsPointOnPolygonBoundaries(
                Point(0, 3.4), Wall(Point(0, 10), Point(2, 10))));

            SECTION("Crossing")
            {
                Crossing crossing;
                crossing.SetPoint1(Point(2, 10));
                crossing.SetPoint2(Point(2, 0));
                subroom.AddCrossing(&crossing);

                REQUIRE(subroom.IsPointOnPolygonBoundaries(
                    Point(2, 10), Wall(Point(0, 10), Point(2, 10))));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(
                    Point(2, 0), Wall(Point(0, 10), Point(2, 10))));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(
                    Point(2, 2.3), Wall(Point(0, 10), Point(2, 10))));
            }

            SECTION("Transition")
            {
                Transition transition;
                transition.SetPoint1(Point(2, 10));
                transition.SetPoint2(Point(2, 0));
                subroom.AddTransition(&transition);

                REQUIRE(subroom.IsPointOnPolygonBoundaries(
                    Point(2, 10), Wall(Point(0, 10), Point(2, 10))));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(
                    Point(2, 0), Wall(Point(0, 10), Point(2, 10))));
                REQUIRE(subroom.IsPointOnPolygonBoundaries(
                    Point(2, 2.3), Wall(Point(0, 10), Point(2, 10))));
            }
        }
    }
}
