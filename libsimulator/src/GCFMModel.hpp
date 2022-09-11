/**
 * \file        GCFMModel.h
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
 * Implementation of classes for Generalized Centrifugal Force Model
 *
 **/
#pragma once
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

#include <unordered_map>
#include <vector>

// forward declaration
class Agent;

struct GCFMModelAgentParameters {
    OperationalModel::ParametersID id{};
    double mass;
    double t;
    double tau;
    GCFMModelAgentParameters(OperationalModel::ParametersID id, double mass, double t, double tau)
        : id(id), mass(mass), t(t), tau(tau)
    {
    }
};

class GCFMModel : public OperationalModel
{
    std::unordered_map<OperationalModel::ParametersID, GCFMModelAgentParameters> _parameterProfiles;
    double _nuPed; /**< strength of the pedestrian repulsive force */
    double _nuWall; /**< strength of the wall repulsive force */
    double _intp_widthPed; /**< Interpolation cutoff radius (in cm) */
    double _intp_widthWall; /**< Interpolation cutoff radius (in cm) */
    double _maxfPed;
    double _maxfWall;
    double _distEffMaxPed; // maximal effective distance
    double _distEffMaxWall; // maximal effective distance
public:
    GCFMModel(
        double nuped,
        double nuwall,
        double dist_effPed,
        double dist_effWall,
        double intp_widthped,
        double intp_widthwall,
        double maxfped,
        double maxfwall,
        const std::vector<GCFMModelAgentParameters>& profiles);
    ~GCFMModel() override = default;

    PedestrianUpdate ComputeNewPosition(
        double dT,
        const Agent& ped,
        const CollisionGeometry& geometry,
        const NeighborhoodSearch& neighborhoodSearch) const override;
    void ApplyUpdate(const PedestrianUpdate& upate, Agent& agent) const override;
    bool ParameterProfileExists(ParametersID id) const override
    {
        return _parameterProfiles.count(id) > 0;
    };
    std::unique_ptr<OperationalModel> Clone() const override;
    OperationalModel::ParametersID AddParameterProfile(GCFMModelAgentParameters parameters);

private:
    /**
     * Driving force \f$ F_i =\frac{\mathbf{v_0}-\mathbf{v_i}}{\tau}\f$
     *
     * @param ped Pointer to Pedestrians
     * @param room Pointer to Room
     *
     * @return Point
     */
    Point
    ForceDriv(const Agent* ped, Point target, double mass, double tau, PedestrianUpdate& update)
        const;
    /**
     * Repulsive force between two pedestrians ped1 and ped2 according to
     * the Generalized Centrifugal Force Model (chraibi2010a)
     *
     * @param ped1 Pointer to Pedestrian: First pedestrian
     * @param ped2 Pointer to Pedestrian: Second pedestrian
     *
     * @return Point
     */
    Point ForceRepPed(const Agent* ped1, const Agent* ped2) const;
    /**
     * Repulsive force acting on pedestrian <ped> from the walls in
     * <subroom>. The sum of all repulsive forces of the walls in <subroom> is calculated
     * @see ForceRepWall
     * @param ped Pointer to Pedestrian
     * @param subroom Pointer to SubRoom
     *
     * @return
     */
    Point ForceRepRoom(const Agent* ped, const CollisionGeometry& geometry) const;
    Point ForceRepWall(const Agent* ped, const Line& l) const;
    Point ForceRepStatPoint(const Agent* ped, const Point& p, double l, double vn) const;
    Point ForceInterpolation(
        double v0,
        double K_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
};
