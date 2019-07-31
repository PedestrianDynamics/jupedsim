
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
#include "../../geometry/Wall.h"

BOOST_AUTO_TEST_SUITE(WallTest)

BOOST_AUTO_TEST_CASE(Wall_Constr_Test)
{
     BOOST_TEST_MESSAGE("starting wall constructor test");
     Wall W1;
     Point P1;
     BOOST_CHECK(W1.GetPoint1() == P1 && W1.GetPoint2() == P1);
     std::string type[] = {"internal", "external"};
     for (int i = 0; i < 10; ++i)
     {
          Point P2(0, i*i);
          Point P3(static_cast<float>(i)/1000000, i*i);

          Line L1(P2, P3);
          Wall W2(P2, P3, type[i%2]);
          BOOST_CHECK_MESSAGE(W2 == L1, "");
          // GetType test
          BOOST_CHECK(W2.GetType() == type[i%2]);
     }
     BOOST_TEST_MESSAGE("Leaving wall constructor test");

}

BOOST_AUTO_TEST_SUITE_END()