/**
 * \file        GompertzModel.h
 * \date        Apr 15, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
 * \ingroup OperationalModels
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
 * Implementation of classes for Gompertz Model
 *
 **/
#pragma once

#include "OperationalModel.h"

#include "geometry/Building.h"

#include <vector>

//forward declaration
class Pedestrian;
class DirectionManager;

/*!
 * \class GompertzModel
 *
 * \brief @todo
 *
 *\ingroup OperationalModels
 *
 * \author Mohcine Chraibi
 */
class GompertzModel : public OperationalModel {
private:

     /// Modellparameter
     double _nuPed;
     double _aPed;
     double _bPed;
     double _cPed;

     double _nuWall;
     double _aWall;
     double _bWall;
     double _cWall;

     /**
      * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
      * This is a duplicate of @see GCFMModel::ForceDriv
      * @param ped Pointer to Pedestrians
      * @param room Pointer to Room
      *
      *
      * @return Point
      */
     Point ForceDriv(Pedestrian* ped, Room* room) const;
     /**
      * Repulsive force between two pedestrians ped1 and ped2 according to
      * the Gompertz model (unpublished)
      *
      * @param ped1 Pointer to Pedestrian: First pedestrian
      * @param ped2 Pointer to Pedestrian: Second pedestrian
      *
      * @return Point
      */
     Point ForceRepPed(Pedestrian* ped1, Pedestrian* ped2) const;
     /**
      * Repulsive force acting on pedestrian <ped> from the walls in
      * <subroom>. The sum of all repulsive forces of the walls in <subroom> is calculated
      * @see ForceRepWall
      * @param ped Pointer to Pedestrian
      * @param subroom Pointer to SubRoom
      *
      * @return Point
      */
     Point ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const;
     /**
      * Repulsive force between pedestrian <ped> and wall <l>
      *
      * @param ped Pointer to Pedestrian
      * @param l reference to Wall
      *
      * @return Point
      */
     Point ForceRepWall(Pedestrian* ped, const Line& l, const Point& centroid, bool inside) const;

public:

     GompertzModel(std::shared_ptr<DirectionManager> dir, double nuped, double aped, double bped, double cped,
                   double nuwall, double awall, double bwall, double cwall);
     virtual ~GompertzModel(void);

//    std::shared_ptr<DirectionStrategy> GetDirection() const;

     /**
      * Get the parameter for the strength of the ped-PED repulsive force
      *
      * @return double
      */
     double GetNuPed() const;

     /**
      * ToDO: What is this parameter doing?
      *
      * @return double
      */
     double GetaPed() const;

     /**
      * ToDO: What is this parameter doing?
      *
      * @return double
      */
     double GetbPed() const;

     /**
      * ToDO: What is this parameter doing?
      *
      * @return double
      */
     double GetcPed() const;

     /**
      * Get the parameter for the strength of the ped-WALL repulsive force
      *
      * @return
      */
     double GetNuWall() const;

     /**
      * ToDO: What is this parameter doing?
      *
      * @return double
      */
     double GetaWall() const;

     /**
      * ToDO: What is this parameter doing?
      *
      * @return double
      */
     double GetbWall() const;

     /**
      * ToDO: What is this parameter doing?
      *
      * @return double
      */
     double GetcWall() const;

     /**
      * @return all model parameters in a nicely formatted string
      */
     virtual std::string GetDescription();

     /**
      * initialize the phi angle
      * @param building
      */
     virtual bool Init (Building* building);

     /**
      * Compute the next simulation step
      * Solve the differential equations and update the positions and velocities
      * @param current the actual time
      * @param deltaT the next timestep
      * @param building the geometry object
      */
     virtual void ComputeNextTimeStep(double current, double deltaT, Building* building, int periodic);
};
