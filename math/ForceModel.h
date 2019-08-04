/**
 * \file        ForceModel.h
 * \date        Apr 15, 2014
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
 * Implementation of classes for force-based models.
 * Actually we've got two different models:
 * 1. Generalized Centrifugal Force Model
 * 2. Gompertz Model
 *
 *
 **/
#pragma once

#include <string>

class Building;


class ForceModel {

public:
     // constructor/destructor
     ForceModel();
     virtual ~ForceModel();

     /**
      * Solve the differential equations and update the positions and velocities
      * @param t the actual time
      * @param tp the next timestep
      * @param building the geometry object
      */
     virtual void CalculateForce(double t, double tp, Building* building) const = 0;

     /**
      * Performs whatever initialization is needed/required
      * @param building the building object
      */
     virtual bool Init (Building* building) const = 0;

     /**
      * @return all model parameters in a nicely formatted string
      */
     virtual std::string writeParameter() const = 0;
};
