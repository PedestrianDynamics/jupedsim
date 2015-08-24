/**
 * \file        VelocityModel.h
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
 * Implementation of first-order model
 * 2. Velocity Model: Tordeux2015
 *
 *
 **/


#ifndef VELOCITYMODEL_H_
#define VELOCITYMODEL_H_

#include <vector>

#include "../geometry/Building.h"
#include "OperationalModel.h"

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
class DirectionStrategy;


class VelocityModel : public OperationalModel {
private:
     /// define the strategy for crossing a door (used for calculating the driving force)
     DirectionStrategy* _direction;

     /// Modellparameter
     double _aPed;
     double _DPed;

     double _aWall;
     double _DWall;

     /**
      * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
      * This is a duplicate of @see GCFMModel::ForceDriv
      * @param ped Pointer to Pedestrians
      * @param room Pointer to Room
      *
      *
      * @return Point
      */
     double OptimalSpeed(Pedestrian* ped, double spacing, double winkel) const;

     Point e0(Pedestrian *ped, Room* room) const;

     my_pair GetSpacing(Pedestrian* ped1, Pedestrian* ped2) const;
           
     /**
      * Repulsive force between two pedestrians ped1 and ped2 according to
      * the Velocity model (unpublished)
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

     VelocityModel(DirectionStrategy* dir, double aped, double Dped,
                   double awall, double Dwall);
     virtual ~VelocityModel(void);

     DirectionStrategy* GetDirection() const;

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
     double GetDPed() const;

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
     double GetDWall() const;

     /**
      * @return all model parameters in a nicely formatted string
      */
     virtual std::string GetDescription() const;

     /**
      * initialize the phi angle
      * @param building
      */
     virtual bool Init (Building* building) const;

     /**
      * Compute the next simulation step
      * Solve the differential equations and update the positions and velocities
      * @param current the actual time
      * @param deltaT the next timestep
      * @param building the geometry object
      */
     virtual void ComputeNextTimeStep(double current, double deltaT, Building* building) const;
};


#endif /* GOMPERTZMODEL_H_ */
