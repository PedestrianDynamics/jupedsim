/**
 * \file        Router.h
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
 **/
#pragma once
#include "geometry/Building.hpp"
#include "pedestrian/Pedestrian.hpp"

class Simulation;

/// Abstract base class for all routers.
/// For more information about routing see
/// http://www.jupedsim.org/jpscore/2016-11-03-routing.html
class Router
{
protected:
    double _currentTime{0.0};
    Simulation * _simulation{nullptr};

    Router() = default;

    Router(const Router &)             = default;
    Router & operator=(const Router &) = default;

    Router(Router &&)             = default;
    Router & operator=(Router &&) = default;

public:
    virtual ~Router() = default;

    void UpdateTime(double time) { _currentTime = time; }

    void SetSimulation(Simulation * simulation) { _simulation = simulation; }

    /// Find the next suitable target for Pedestrian p.
    /// @post (*p).exitline/.exitindex are set (important!)
    /// @param p the Pedestrian
    /// @return next suitable target, -1 in the case no destination could be found
    virtual int FindExit(Pedestrian * p) = 0;

    /// Update the router, when geometry changed due to external changes.
    virtual void Update() = 0;
};
