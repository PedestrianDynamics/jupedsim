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

#include "Geometry.hpp"
#include "OperationalModel.hpp"
#include "geometry/Building.hpp"
#include "neighborhood/NeighborhoodSearch.hpp"
#include "pedestrian/Pedestrian.hpp"

#include <vector>

using my_pair = std::pair<double, Pedestrian::UID>;
// sort with respect to first element (ascending).
// In case of equality sort with respect to second element (descending)
struct sort_pred {
    bool operator()(const my_pair& left, const my_pair& right)
    {
        return (left.first == right.first) ? (left.second > right.second) :
                                             (left.first < right.first);
    }
};

// forward declaration
class Pedestrian;
class DirectionStrategy;
class Simulation;

/*!
 * \class VelocityModel
 *
 * \brief @todo
 *
 *\ingroup OperationalModels
 *
 * \author Mohcine Chraibi
 */
class VelocityModel : public OperationalModel
{
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
    double OptimalSpeed(const Pedestrian* ped, double spacing) const;

    /**
     * The desired direction of pedestrian
     *
     * @param ped: Pointer to Pedestrians
     * @param room: Pointer to room
     *
     * @return Point
     */
    void e0(const Pedestrian* ped, Point target, PedestrianUpdate& update) const;
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
    my_pair GetSpacing(const Pedestrian* ped1, const Pedestrian* ped2, Point ei) const;
    /**
     * Repulsive force between two pedestrians ped1 and ped2 according to
     * the Velocity model (to be published in TGF15)
     *
     * @param ped1 Pointer to Pedestrian: First pedestrian
     * @param ped2 Pointer to Pedestrian: Second pedestrian
     *
     * @return Point
     */
    Point ForceRepPed(const Pedestrian* ped1, const Pedestrian* ped2) const;
    /**
     * Repulsive force acting on pedestrian <ped> from the walls in
     * <subroom>. The sum of all repulsive forces of the walls in <subroom> is calculated
     * @see ForceRepWall
     * @param ped Pointer to Pedestrian
     * @param subroom Pointer to SubRoom
     *
     * @return Point
     */
    Point ForceRepRoom(const Pedestrian* ped, const Geometry& geometry) const;
    /**
     * Repulsive force between pedestrian <ped> and wall <l>
     *
     * @param ped Pointer to Pedestrian
     * @param l reference to Wall
     *
     * @return Point
     */
    Point ForceRepWall(const Pedestrian* ped, const Line& l) const;

public:
    VelocityModel(double aped, double Dped, double awall, double Dwall);
    ~VelocityModel() override = default;

    PedestrianUpdate ComputeNewPosition(
        double dT,
        const Pedestrian& ped,
        const Geometry& geometry,
        const NeighborhoodSearch& neighborhoodSearch) const override;

    void ApplyUpdate(const PedestrianUpdate& update, Pedestrian& agent) const override;
};
