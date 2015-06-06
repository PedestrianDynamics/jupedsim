
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

#define BOOST_TEST_MODULE SubRoomTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../../geometry/SubRoom.h" 


BOOST_AUTO_TEST_SUITE(SubRoomTest)

BOOST_AUTO_TEST_CASE(Line_test)
{
    BOOST_MESSAGE("starting small wall test");
    SubRoom sub1;
    sub1.SetSubRoomID(1);
    
    sub1.AddWall(Wall(Point(0, 0), Point(0, 10)));
    sub1.AddWall(Wall(Point(0, 10), Point(5, 10)));
    sub1.AddWall(Wall(Point(5, 10), Point(5.25, 10)));
    sub1.AddWall(Wall(Point(5.28, 10), Point(5.30, 10)));
    sub1.AddWall(Wall(Point(5.33, 10), Point(5.36, 10)));
    sub1.AddWall(Wall(Point(5.38, 10), Point(5.40, 10)));
    sub1.AddWall(Wall(Point(5.43, 10), Point(10, 10)));
    sub1.AddWall(Wall(Point(10, 10), Point(10, 5)));
    sub1.AddWall(Wall(Point(10, 0), Point(0, 0)));
    
    vector<Line> goal (Point(10, 5), Point(10, 0));
    bool temp = sub1.ConvertLineToPoly(&goal);
    vector<Point> poly (sub1.GetPolygon());
    for (auto it:poly)
      BOOST_WARN("x = " << it->GetX() << ", y = " << it->GetY());
    
    BOOST_MESSAGE("Leaving small wall test");
}

BOOST_AUTO_TEST_SUITE_END()