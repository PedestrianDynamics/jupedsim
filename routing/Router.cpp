/**
 * \file        Router.cpp
 * \date        Nov 11, 2010
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
#include "Router.h"

#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"

Router::Router()
{
     _finalDestinations = std::vector<int>();
     _id=-1;
     _strategy=ROUTING_UNDEFINED;
}

Router::Router(int id, RoutingStrategy s)
{
    _id = id;
    _strategy = s;
}

Router::~Router()
{
}

void Router::AddFinalDestinationID(int id)
{
     _finalDestinations.push_back(id);
}

const std::vector<int> Router::GetFinalDestinations() const
{
     return _finalDestinations;
}

int Router::GetID() const
{
     return _id;
}

void Router::SetStrategy(const RoutingStrategy& strategy)
{
     _strategy=strategy;
}

RoutingStrategy Router::GetStrategy() const
{
     return _strategy;
}

void Router::Update(){

}