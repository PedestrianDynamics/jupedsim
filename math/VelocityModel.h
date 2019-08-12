/**
 * \file        VelocityModel.h
 * \date        Apr 15, 2014
 * \version     v0.8
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
 * Implementation of first-order model
 * Velocity Model: Tordeux2015 (3)
 *
 *
 **/
#pragma once

#include "OperationalModel.h"

#include "geometry/Building.h"

#include <vector>

typedef std::pair<double, double> my_pair;
// sort with respect to first element (ascending).
// In case of equality sort with respect to second element (descending)
struct sort_pred
{
      bool operator () (const my_pair& left, const my_pair& right)
            {
                  return (left.first == right.first) ?
                        (left.second > right.second) :
                        (left.first < right.first);
            }
};


//forward declaration
class Pedestrian;
class DirectionManager;

/*!
 * \class VelocityModel
 *
 * \brief @todo
 *
 *\ingroup OperationalModels
 *
 * \author Mohcine Chraibi
 */
class VelocityModel : public OperationalModel {
private:

     /// Modellparameter
     double _aPed;
     double _DPed;

     double _aWall;
     double _DWall;

     /**
      * Optimal velocity function \f$ V(spacing) =\min{v_0, \max{0, (s-l)/T}}  \f$
      *
      * @param ped: Pointer to Pedestrians
      * @param spacing: minimum spacing to the neighbors
      * @param winkel: angle between <ped> and the nearest neighbor. Is not yet used!
      *
      * @return double
      */
     double OptimalSpeed(Pedestrian* ped, double spacing) const;

     /**
      * The desired direction of pedestrian
      *
      * @param ped: Pointer to Pedestrians
      * @param room: Pointer to room
      *
      * @return Point
      */
     Point e0(Pedestrian *ped, Room* room) const;
     /**
      * Get the spacing between ped1 and ped2
      *
      * @param ped1 Pointer to Pedestrian: First pedestrian
      * @param ped2 Pointer to Pedestrian: Second pedestrian
      * @param ei the direction of pedestrian.
      * This direction is: \f$ e_0 + \sum_j{R(spacing_{ij})*e_{ij}}\f$
      * and should be calculated *before* calling OptimalSpeed
      * @return Point
      */
     my_pair GetSpacing(Pedestrian* ped1, Pedestrian* ped2, Point ei, int periodic) const;
     /**
      * Repulsive force between two pedestrians ped1 and ped2 according to
      * the Velocity model (to be published in TGF15)
      *
      * @param ped1 Pointer to Pedestrian: First pedestrian
      * @param ped2 Pointer to Pedestrian: Second pedestrian
      *
      * @return Point
      */
     Point ForceRepPed(Pedestrian* ped1, Pedestrian* ped2, int periodic) const;
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

     VelocityModel(std::shared_ptr<DirectionManager> dir, double aped, double Dped,
                   double awall, double Dwall);
     virtual ~VelocityModel(void);


//     std::shared_ptr<DirectionStrategy> GetDirection() const;

     /**
      * @todo What is this parameter doing?
      *
      * @return double
      *
      */
     double GetaPed() const;

     /**
      * @todo What is this parameter doing?
      *
      * @return double
      */
     double GetDPed() const;

     /**
      * @todo What is this parameter doing?
      *
      * @return double
      */
     double GetaWall() const;

     /**
      * @todo What is this parameter doing?
      *
      * @return double
      */
     double GetDWall() const;

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
      * @param periodic: used in some utests for periodic scenarios (very specific)
      */
     virtual void ComputeNextTimeStep(double current, double deltaT, Building* building, int periodic);
};
