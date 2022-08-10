/**
 * \file        OperationalModel.h
 * \date        Nov. 11, 2014
 * \version     v0.7
 * \author      Ulrich Kemloh
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
 * This class defines the interface for operational models, which aim is to compute the next
 *positions of the pedestrians.
 *
 **/

#pragma once

#include "CollisionGeometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "Pedestrian.hpp"
#include "Point.hpp"

#include <optional>

struct PedestrianUpdate {
    std::optional<Point> position{};
    std::optional<Point> velocity{};
    std::optional<Point> waitingPos{};
    Point v0{};
    bool resetTurning{false};
    bool resetPhi{false};
};

class OperationalModel
{
public:
    OperationalModel() = default;
    virtual ~OperationalModel() = default;

    virtual PedestrianUpdate ComputeNewPosition(
        double dT,
        const Pedestrian& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch& neighborhoodSearch) const = 0;

    virtual void ApplyUpdate(const PedestrianUpdate& update, Pedestrian& agent) const = 0;

    virtual std::unique_ptr<OperationalModel> Clone() const = 0;
};
