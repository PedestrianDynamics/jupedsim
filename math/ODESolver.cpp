/**
 * \file        ODESolver.cpp
 * \date        Aug 17, 2010
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
#include "ODESolver.h"

#include "ForceModel.h"

#include "geometry/Building.h"
#include "pedestrian/Pedestrian.h"


ODESolver::ODESolver(ForceModel *tmodel) : model(tmodel)
{

}


EulerSolver::EulerSolver(ForceModel *tmodel) : ODESolver(tmodel)
{

}

void EulerSolver::solveODE(double ti, double tip1, Building* building) const
{
    model->CalculateForce(ti, tip1, building);
}
