/**
 * File:   ODESolver.cpp
 *
 * Created on 17. August 2010, 15:31
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */


#include "ODESolver.h"
#include "ForceModel.h"
#include "../geometry/Building.h"
#include "../pedestrian/Pedestrian.h"

using namespace std;

ODESolver::ODESolver(ForceModel *model) : model(model)
{

}


EulerSolver::EulerSolver(ForceModel *model) : ODESolver(model)
{

}

void EulerSolver::solveODE(double ti, double tip1, Building* building) const
{
    model->CalculateForce(ti, tip1, building);
}
