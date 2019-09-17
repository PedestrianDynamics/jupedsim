/**
 * \file        DirectionStrategy.cpp
 * \date        Dec 13, 2010
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
#include "DirectionStrategy.h"

#include "geometry/Building.h"
#include "geometry/Line.h"
#include "geometry/NavLine.h"
#include "geometry/SubRoom.h"
#include "pedestrian/Pedestrian.h"
#include <chrono>


double DirectionStrategy::GetDistance2Wall(Pedestrian*) const
{
     return -1.;
}

double DirectionStrategy::GetDistance2Target(Pedestrian*, int)
{
     return -1.;
}