
/**
 * \file        testClassLine.cpp
 * \date        May 10, 2015
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
#include "../../geometry/Room.h"
#include "../../general/Macros.h"
#include <string>

BOOST_AUTO_TEST_SUITE(RoomTest)

BOOST_AUTO_TEST_CASE(ROOM_CONST_SET_GET_TEST)
{
     BOOST_TEST_MESSAGE("starting const_set_get test");
     Room R1;
     for (int i = 1; i < 10; ++i)
     {
          R1.SetID(i);
          R1.SetCaption("Room" + std::to_string(i));
          R1.SetZPos(10.0/i);
          R1.SetEgressTime(i * 10.0 / 3);
          R1.SetState(RoomState(i%2));
          Room R2(R1);
          BOOST_CHECK(R2.GetID() == i);
          BOOST_CHECK(R2.GetCaption() == "Room" + std::to_string(i));
          BOOST_CHECK(R2.GetZPos() == 10.0 / i);
          BOOST_CHECK(R2.GetEgressTime() == i * 10.0 / 3);
          BOOST_CHECK(R2.GetState() == i % 2);
     }
     BOOST_TEST_MESSAGE("Leaving const_set_get test");
}

BOOST_AUTO_TEST_SUITE_END()