/**
 * @file ForceModel.h
 *
 * @brief Implementation of classes for some force-based models
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
 * Implementation of classes for force-based models.
 * Actually we've got two different models:
 * 1. Generalized Centrifugal Force Model
 * 2. Gompertz Model
 *
 * @date Tue Apr 15 19:19:04 2014
 */

//
#ifndef _FORCEMODEL_H
#define _FORCEMODEL_H

#include <vector>

#include "../geometry/Building.h"



class Pedestrian;
class DirectionStrategy;


/**
 * @date   Fri Apr 18 16:40:39 2014
 *
 * @brief The operative model. Definition of several force-based models
 *         for ped pedestrians dynamics
 *
 *
 */
class ForceModel {

public:
     // Konstruktoren
     ForceModel();
     virtual ~ForceModel();

     //FIXME: remove
     virtual void CalculateForce(double time, std::vector< Point >& result_acc, Building* building, int roomID, int SubRoomID) const = 0;

     /**
      * Solve the differential equations and update the positions and velocities
      * @param t the actual time
      * @param tp the next timestep
      * @param building the geometry object
      */
     virtual void CalculateForceLC(double t, double tp, Building* building) const = 0;

     /**
      * @return all model parameters in a nicely formatted string
      */
     virtual std::string writeParameter() const = 0;
};

/************************************************************
 GCFM ForceModel
************************************************************/
/**
 * @date   Fri Apr 18 16:39:13 2014
 *
 * @brief  The Generalized Centrifugal Force Model
 *
 *
 */
class GCFMModel : public ForceModel {
private:
     /// define the strategy for crossing a door (used for calculating the driving force)
     DirectionStrategy* _direction;
     // Modellparameter
     double _nuPed;                /**< strength of the pedestrian repulsive force */
     double _nuWall;               /**< strength of the wall repulsive force */
     double _intp_widthPed; /**< Interpolation cutoff radius (in cm) */
     double _intp_widthWall; /**< Interpolation cutoff radius (in cm) */
     double _maxfPed;
     double _maxfWall;
     double _distEffMaxPed; // maximal effective distance
     double _distEffMaxWall; // maximal effective distance

     // Private Funktionen
     /**
      * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
      *
      * @param ped Pointer to Pedestrians
      * @param room Pointer to Room
      *
      * @return Point
      */
     Point ForceDriv(Pedestrian* ped, Room* room) const;
     /**
      * Repulsive force between two pedestrians ped1 and ped2 according to
      * the Generalized Centrifugal Force Model (chraibi2010a)
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
      * @return
      */
     Point ForceRepRoom(Pedestrian* ped, SubRoom* subroom) const;
     Point ForceRepWall(Pedestrian* ped, const Wall& l) const;
     Point ForceRepStatPoint(Pedestrian* ped, const Point& p, double l, double vn) const;
     Point ForceInterpolation(double v0, double K_ij, const Point& e, double v, double d, double r, double l) const;
public:

     GCFMModel(DirectionStrategy* dir, double nuped, double nuwall, double dist_effPed, double dist_effWall,
               double intp_widthped, double intp_widthwall, double maxfped, double maxfwall);
     virtual ~GCFMModel(void);

     // Getter
     DirectionStrategy* GetDirection() const;
     double GetNuPed() const;
     double GetNuWall() const;
     double GetDistEffMax() const;
     double GetIntpWidthPed() const;
     double GetIntpWidthWall() const;
     double GetMaxFPed() const;
     double GetMaxFWall() const;
     double GetDistEffMaxPed() const;
     double GetDistEffMaxWall() const;


     //void UpdateCellularModel(Building* building) const;

     // virtuelle Funktionen
     virtual void CalculateForce(double time, std::vector< Point >& result_acc, Building* building,
                                 int roomID, int SubRoomID) const;
     virtual void CalculateForceLC(double t, double tp, Building* building) const;
     virtual std::string writeParameter() const;
};

/************************************************************
 GOMPERTZ ForceModel
************************************************************/
/**
 * Class defining the Gompertz model
 *
 *
 * @brief The Gompertz model. Not yet published.
 */
class GompertzModel : public ForceModel {
private:
     /// define the strategy for crossing a door (used for calculating the driving force)
     DirectionStrategy* _direction;

     /// Modellparameter
     double _nuPed;
     double _nuWall;

     /**
      * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\$
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
     Point ForceRepWall(Pedestrian* ped, const Wall& l) const;

public:

     GompertzModel(DirectionStrategy* dir, double nuped, double nuwall);
     virtual ~GompertzModel(void);

     DirectionStrategy* GetDirection() const;
     /**
      * Get the parameter for the strength of the ped-PED repulsive force
      *
      *
      * @return double
      */
     double GetNuPed() const;
     /**
      * Get the parameter for the strength of the ped-WALL repulsive force
      *
      *
      * @return
      */
     double GetNuWall() const;
     virtual void CalculateForce(double time, std::vector< Point >& result_acc, Building* building,
                                 int roomID, int SubRoomID) const;
     /**
      * Solve the differential equations and update the positions and velocities
      * @param t the actual time
      * @param tp the next timestep
      * @param building the geometry object
      */
     virtual void CalculateForceLC(double t, double tp, Building* building) const;
     /**
      * @return all model parameters in a nicely formatted string
      */
     virtual std::string writeParameter() const;
};

#endif  /* _FORCEMODEL_H */

