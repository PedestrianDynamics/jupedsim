
/**
 * \file        testClassLine.cpp
 * \date        April 27, 2015
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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include "../../geometry/SubRoom.h"

#include <vector>

BOOST_AUTO_TEST_SUITE(SubRoomTest)

BOOST_AUTO_TEST_CASE(JTol_WallGap_test)
{
    BOOST_TEST_MESSAGE("starting small gap between wall test");

    NormalSubRoom sub1;
    sub1.SetSubRoomID(1);
    sub1.SetRoomID(1);

    Point P1 (00.0, 00);
    Point P2 (00.0, 10);
    Point P3 (05.0, 10);
    Point P4 (5.25, 10);
    Point P5 (5.27, 10);
    Point P6 (5.31, 10);
    Point P7 (5.32, 10);
    Point P8 (5.36, 10);
    Point P9 (5.38, 10);
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

    std::vector<Line*> goal; // (Line(Point(10, 5), Point(10, 0)));
    goal.push_back(&exit);


    if (sub1.ConvertLineToPoly(goal) == true) {
        std::vector<Point> poly = sub1.GetPolygon();
        for (auto it:poly)
            BOOST_CHECK_MESSAGE(poly.size() == 10, "x = " << it._x << ", y = " << it._y);
    }
    else
        BOOST_CHECK(false);

    BOOST_TEST_MESSAGE("Leaving small wall test");
}

//BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(small_wall_test, 1);

BOOST_AUTO_TEST_CASE(small_wall_test)
{
    BOOST_TEST_MESSAGE("starting small wall test");

    NormalSubRoom sub;
    sub.SetSubRoomID(1);
    sub.SetRoomID(1);

    Point P1 (0, 0);
    Point P2 (0, 10);
    Point P3 (5.0, 10);
    Point P4 (5.029, 10);
    Point P5 (10, 10);
    Point P6 (10, 5);
    Point P7 (10, 0);


    BOOST_CHECK(sub.AddWall(Wall(P1, P2)) == true);
    BOOST_CHECK(sub.AddWall(Wall(P2, P3)) == true);
    BOOST_CHECK(sub.AddWall(Wall(P3, P4)) == false);
    BOOST_CHECK(sub.AddWall(Wall(P4, P5)) == true);
    BOOST_CHECK(sub.AddWall(Wall(P5, P6)) == true);
    BOOST_CHECK(sub.AddWall(Wall(P1, P7)) == true);

    BOOST_TEST_MESSAGE("Leaving small wall test");
}

//BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES(overlap_wall_test, 2);

BOOST_AUTO_TEST_CASE(overlap_wall_test)
{
    BOOST_TEST_MESSAGE("starting overlap wall test");
    NormalSubRoom sub;
    sub.SetSubRoomID(1);
    sub.SetRoomID(1);

    Point P1 (0, 0);
    Point P2 (0, 10);
    Point P3 (10, 10);
    Point P4 (10, 8);
    Point P5 (10, 0);
    Point P6 (10, 5);

    sub.AddWall(Wall(P1, P2));
    sub.AddWall(Wall(P5, P1));
    sub.AddWall(Wall(P3, P6));
    sub.AddWall(Wall(P2, P3));
    sub.AddWall(Wall(P5, P6));

    Line exit(P6, P4);
    std::vector<Line*> door; // door overlaps with the wall
    door.push_back(&exit);

    BOOST_CHECK(sub.ConvertLineToPoly(door) == false);

    NormalSubRoom sub2;
    sub2.SetSubRoomID(2);
    sub2.SetRoomID(2);

    sub2.AddWall(Wall(P1, P2));
    sub2.AddWall(Wall(P2, P3));
    sub2.AddWall(Wall(P3, P6)); // Overlapping Walls
    sub2.AddWall(Wall(P5, P1));
    sub2.AddWall(Wall(P4, P6)); // Overlapping Walls

    BOOST_CHECK(sub.ConvertLineToPoly(door) == false);

    BOOST_TEST_MESSAGE("Leaving overlap wall test");
}

BOOST_AUTO_TEST_CASE(concave_intersecting_wall_test)
{
	BOOST_TEST_MESSAGE("starting concave subroom intersecting wall test");
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
	std::vector<Line*> door;
	door.push_back(&exit);

	BOOST_CHECK(sub1.ConvertLineToPoly(door) == false);
}

BOOST_AUTO_TEST_SUITE_END()
