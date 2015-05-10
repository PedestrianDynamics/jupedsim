 
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

#define BOOST_TEST_MODULE ObstacleTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../../geometry/Obstacle.h"
#include "../../geometry/Point.h"
#include "../../geometry/Wall.h"

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>

BOOST_AUTO_TEST_SUITE(ObstacleTest)

BOOST_AUTO_TEST_CASE(Obstacle_Constr_setGet_Test)
{
     BOOST_MESSAGE("starting obstacle_Constr_setGet_Test");
     std::srand(std::time(0));
     const double PI = 3.14159265358979323846;
     Obstacle obs1;
     std::vector<Wall> walls_test;
     for (int i = 1; i < 10; ++i)
     {
          Point P1 (cos(PI/i), sin(PI/i));  
          Point P2 (i, i*i);
          Wall W1(P1, P2, "interior");
          obs1.AddWall(W1);
          walls_test.emplace_back(W1);
          obs1.SetCaption("obstacle" + std::to_string(i));
          int flag = std::rand() % 2;
          obs1.SetClosed(flag);
          obs1.SetHeight(-i);  // logically incorrect
          obs1.SetId(i-1);
          
          BOOST_CHECK(obs1.GetCaption() == "obstacle" + std::to_string(i));
          BOOST_CHECK(obs1.GetClosed() == flag);
          BOOST_CHECK(obs1.GetHeight() == -i); // logically incorrect
          BOOST_CHECK(obs1.GetId() == (i-1));
          BOOST_CHECK(obs1.GetAllWalls() == walls_test);
     }
     BOOST_MESSAGE("Leaving obstacle_Constr_setGet_Test");
}
/*
BOOST_FIXTURE_TEST_CASE(Obstacle_whichQuad_test, Obstacle)
{
     BOOST_MESSAGE("starting obstacle which_quad test");
     Point vertex;
     Point hitPos;
     Obstacle obs;
          
     BOOST_CHECK(WhichQuad(vertex, hitPos) == 3);
     for (int i = 1, j = 1; i < 10; ++i, ++j)
     {
          vertex = Point(i, j);
          hitPos = Point(-i, -j);  
          BOOST_CHECK(obs.WhichQuad(vertex, hitPos) == 1);
          
          vertex = Point(-i, j);
          hitPos = Point(i, -j);	  
          BOOST_CHECK(obs.WhichQuad(vertex, hitPos) == 2);
                    
          vertex = Point(-i, -j);
          hitPos = Point(i, j);	  
          BOOST_CHECK(obs.WhichQuad(vertex, hitPos) == 3);
                    
          vertex = Point(i, -j);
          hitPos = Point(-i, j);	  
          BOOST_CHECK(obs.WhichQuad(vertex, hitPos) == 4);
          
     }
     BOOST_MESSAGE("Leaving obstacle which_quad test");
} */

BOOST_AUTO_TEST_CASE(Obstacle_Contains_test)
{
     BOOST_MESSAGE("starting obstacle contains test");
     // test
     BOOST_MESSAGE("Leaving obstacle contains test");
}

BOOST_AUTO_TEST_CASE(Obstacle_ConvertLineToPoly_Test)
{
     BOOST_MESSAGE("starting obstacle ConvertLineToPoly test");
     const double PI = 3.14159265358979323846;
     for (int i = 1; i < 10; ++i)
     {
          Point P1 (cos(PI/i), sin(PI/i));  
          Point P2 (i, i*i);
          Point P3 (2*i, i);
          Point P4 (-i, -i*i);
          Point P5 (100, 100);
          
          Obstacle obs1;
          
          Wall w1(P1, P2);
          Wall w2(P2, P3);
          Wall w3(P3, P4);
          Wall w4(P4, P1);
          Wall w5(P2, P5);
          
          obs1.SetId(i-1);
          obs1.AddWall(w1);
          obs1.AddWall(w2);
          obs1.AddWall(w3);
          
          obs1.SetClosed(1);
          //BOOST_CHECK_MESSAGE(obs1.ConvertLineToPoly() == false, obs1.ConvertLineToPoly());
          
          obs1.AddWall(w4);
          BOOST_CHECK_MESSAGE(obs1.ConvertLineToPoly() == true, obs1.ConvertLineToPoly());
          
          obs1.AddWall(w5);
          BOOST_CHECK_MESSAGE(obs1.ConvertLineToPoly() == true, obs1.ConvertLineToPoly());
     }
     BOOST_MESSAGE("Leaving obstacle ConvertLineToPoly test");
}

BOOST_AUTO_TEST_CASE(Obstacle_GetCentroid_Test)
{
     BOOST_MESSAGE("starting obstacle GetCentroid & intersectWithLine test");
     for (int i = 0; i < 10; ++i) {
          Point P1(i, i);
          Point P2(i+10, i);
          Point P3(i+5, i+5);
          Point P4(i+5, i-5);
                    
          Wall w1(P1, P2);
          Wall w2(P2, P3);
          Wall w3(P3, P1);
          
          Obstacle obs1;
          obs1.SetId(i-1);
          
          obs1.AddWall(w1);
          obs1.AddWall(w2);
          obs1.AddWall(w3);
          
          obs1.SetClosed(1);
          obs1.ConvertLineToPoly();
          BOOST_CHECK(obs1.GetCentroid() == (P1 + P2 + P3) / 3);  
          
          Line L1(P3, P4);
          BOOST_CHECK(obs1.IntersectWithLine(L1) == true);
     }
     BOOST_MESSAGE("starting obstacle GetCentroid test");
}
BOOST_AUTO_TEST_SUITE_END()