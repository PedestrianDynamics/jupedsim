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
 * This class defines the interface for operational models, which aim is to compute the next positions of
 * the pedestrians.
 *
 **/


/** @defgroup OperationalModels
 * Collection of different operational models
 * [documentation](http://www.jupedsim.org/jpscore/2016-11-01-operativ)
 *  @{
 */
/** @} */ // end of group
#pragma once

#include "Geometry.hpp"
#include "OperationalModelType.hpp"
#include "direction/DirectionManager.hpp"

#include <memory>
#include <string>


class Building;
class Simulation;
struct Configuration;

struct PedestrianUpdate {
    std::optional<Point> position{};
    std::optional<Point> velocity{};
    std::optional<Point> lastE0{};
    std::optional<Point> waitingPos{};
    Point v0{};
    bool resetTurning{false};
    bool resetPhi{false};
};

class OperationalModel
{
protected:
    // define the strategy for crossing a door (used for calculating the driving force)
    DirectionManager * _direction{};
    double _currentTime{0.0};
    Simulation * _simulation{};

public:
    static std::unique_ptr<OperationalModel> CreateFromType(
        OperationalModelType type,
        const Configuration & config,
        DirectionManager * directionManager);
    explicit OperationalModel(DirectionManager * directionManager);
    virtual ~OperationalModel() = default;

    virtual PedestrianUpdate ComputeNewPosition(
        double dT,
        const Pedestrian & ped,
        const Building & building,
        const Geometry & geometry) const = 0;

    virtual void ApplyUpdate(const PedestrianUpdate & update, Pedestrian & agent) const = 0;

    /**
      * Performs whatever initialization is needed/required.
      * This function is called at the beginning the simulation once.
      * @param building, the building object
      */
    void Init(Simulation * simulation);

    void Update(double time) { _currentTime = time; }
};
