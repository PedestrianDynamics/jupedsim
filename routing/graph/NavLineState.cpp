/**
 * \file        NAvLineState.cpp
 * \date        Sep 18, 2012
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#include "NavLineState.h"



NavLineState::NavLineState()
{
     open = true;
     timeOfInformation = 0;
     timeFirstSeen = 0;
}

NavLineState::~NavLineState()
{

}

void NavLineState::close(double time)
{
     open = false;
     timeOfInformation = time;
     timeFirstSeen = time;
}

bool NavLineState::closed()
{
     return !open;

}

bool NavLineState::isShareable(double time)
{
     if(!timeOfInformation) return true;
     if(timeOfInformation+INFO_OFFSET < time) {
          timeOfInformation = 0;
          return true;
     }
     return false;


}

bool NavLineState::mergeDoor(NavLineState & orig, double time)
{
     if(timeFirstSeen == 0 || orig.timeFirstSeen > timeFirstSeen) {
          open = orig.open;
          timeFirstSeen = orig.timeFirstSeen;
          timeOfInformation = time;
          return true;
     }
     return false;
}

void NavLineState::print()
{
     std::cout << open << " - "<< timeFirstSeen << " - " << timeOfInformation << std::endl;
}
