/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include "GenericAgent.hpp"
#include "Journey.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

#include <unordered_map>
#include <vector>

struct GCFMModelAgentParameters {
    OperationalModel::ParametersID id{};
    double mass;
    double tau;
    double v0;
    double Av;
    double AMin;
    double BMin;
    double BMax;
};

class GCFMModel : public OperationalModelBase<GCFMModelAgentParameters>
{
public:
    struct Data : public GenericAgent {
        double speed{};
        Point e0{};
        int orientationDelay{0};
        Data(
            ID id_,
            Journey::ID journeyId_,
            OperationalModel::ParametersID parameterProfileId_,
            Point pos_,
            Point orientation_,
            double speed_,
            Point e0_)
            : GenericAgent(id_, journeyId_, parameterProfileId_, pos_, orientation_)
            , speed(speed_)
            , e0(e0_)
        {
        }
    };
    using NeighborhoodSearchType = NeighborhoodSearch<Data>;

private:
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
        const Data& agent,
        const CollisionGeometry& geometry,
        const NeighborhoodSearchType& neighborhoodSearch) const;
    void ApplyUpdate(const PedestrianUpdate& upate, Data& agent) const;
    void CheckDistanceConstraint(
        const Data& agent,
        const NeighborhoodSearchType& neighborhoodSearch) const;
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
    Point ForceDriv(
        const Data& ped,
        Point target,
        double mass,
        double tau,
        double deltaT,
        PedestrianUpdate& update) const;
    /**
     * Repulsive force between two pedestrians ped1 and ped2 according to
     * the Generalized Centrifugal Force Model (chraibi2010a)
     *
     * @param ped1 Pointer to Pedestrian: First pedestrian
     * @param ped2 Pointer to Pedestrian: Second pedestrian
     *
     * @return Point
     */
    Point ForceRepPed(const Data& ped1, const Data& ped2) const;
    /**
     * Repulsive force acting on pedestrian <ped> from the walls in
     * <subroom>. The sum of all repulsive forces of the walls in <subroom> is calculated
     * @see ForceRepWall
     * @param ped Pointer to Pedestrian
     * @param subroom Pointer to SubRoom
     *
     * @return
     */
    Point ForceRepRoom(const Data& ped, const CollisionGeometry& geometry) const;
    Point ForceRepWall(const Data& ped, const LineSegment& l) const;
    Point ForceRepStatPoint(const Data& ped, const Point& p, double l, double vn) const;
    Point ForceInterpolation(
        double v0,
        double K_ij,
        const Point& e,
        double v,
        double d,
        double r,
        double l) const;
    double AgentToAgentSpacing(const Data& agent, const Data& otherAgent) const;
};
