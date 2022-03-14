/**
 * \file        OperationalModel.cpp
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
#include "OperationalModel.hpp"

#include "OperationalModelType.hpp"
#include "direction/DirectionManager.hpp"
#include "general/Configuration.hpp"
#include "math/GCFMModel.hpp"
#include "math/VelocityModel.hpp"

std::unique_ptr<OperationalModel> OperationalModel::CreateFromType(
    OperationalModelType type,
    const Configuration & config,
    DirectionManager * directionManager)
{
    switch(type) {
        case OperationalModelType::GCFM:
            return std::make_unique<GCFMModel>(
                directionManager,
                config.nuPed,
                config.nuWall,
                config.distEffMaxPed,
                config.distEffMaxWall,
                config.intPWidthPed,
                config.intPWidthWall,
                config.maxFPed,
                config.maxFWall);
        case OperationalModelType::VELOCITY:
            return std::make_unique<VelocityModel>(
                directionManager, config.aPed, config.dPed, config.aWall, config.dWall);
    }
}

OperationalModel::OperationalModel(DirectionManager * directionManager) :
    _direction(directionManager)
{
}

void OperationalModel::Init(Simulation * simulation)
{
    _simulation = simulation;
}
