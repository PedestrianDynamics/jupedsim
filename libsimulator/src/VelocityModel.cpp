// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "VelocityModel.hpp"

#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "Logger.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"
#include "fmt/ostream.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>
#include <vector>

VelocityModel::VelocityModel(double aped, double Dped, double awall, double Dwall)
    : _aPed(aped), _DPed(Dped), _aWall(awall), _DWall(Dwall)
{
}

OperationalModelType VelocityModel::Type() const
{
    return OperationalModelType::VELOCITY;
}

OperationalModelUpdate VelocityModel::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, _cutOffRadius);
    const auto& boundary = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    // Remove any agent from the neighborhood that is obstructed by geometry and the current
    // agent
    neighborhood.erase(
        std::remove_if(
            std::begin(neighborhood),
            std::end(neighborhood),
            [&ped, &boundary](const auto& neighbor) {
                if(ped.id == neighbor.id) {
                    return true;
                }
                const auto agent_to_neighbor = LineSegment(ped.pos, neighbor.pos);
                if(std::find_if(
                       boundary.cbegin(),
                       boundary.cend(),
                       [&agent_to_neighbor](const auto& boundary_segment) {
                           return intersects(agent_to_neighbor, boundary_segment);
                       }) != boundary.end()) {
                    return true;
                }

                return false;
            }),
        std::end(neighborhood));

    const auto neighborRepulsion = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        Point{},
        [&ped, this](const auto& res, const auto& neighbor) {
            return res + NeighborRepulsion(ped, neighbor);
        });

    const auto boundaryRepulsion = std::accumulate(
        boundary.cbegin(),
        boundary.cend(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + BoundaryRepulsion(ped, element);
        });

    Point desired_direction = (ped.destination - ped.pos).Normalized();
    if(desired_direction == Point()) {
        desired_direction = ped.orientation;
    }
    const auto direction = (desired_direction + neighborRepulsion + boundaryRepulsion).Normalized();
    const auto spacing = std::accumulate(
        std::begin(neighborhood),
        std::end(neighborhood),
        std::numeric_limits<double>::max(),
        [&ped, &direction, this](const auto& res, const auto& neighbor) {
            return std::min(res, GetSpacing(ped, neighbor, direction));
        });

    const auto& model = std::get<VelocityModelData>(ped.model);
    const auto optimal_speed = OptimalSpeed(ped, spacing, model.timeGap);
    const auto velocity = direction * optimal_speed;
    if(ped.id == -18)
        fmt::print(
            "ped: {}, pos: {}\n direction: {}, desired_direction: {}\n Orientation: {}\n NR: {}, "
            "WR: {}\nspacing: "
            "{}, "
            "speed: "
            "{}\n--> velocity: {}\n",
            ped.id,
            ped.pos,
            direction,
            desired_direction,
            ped.orientation,
            neighborRepulsion,
            boundaryRepulsion,
            spacing,
            optimal_speed,
            velocity);

    return VelocityModelUpdate{ped.pos + velocity * dT, direction};
};

void VelocityModel::ApplyUpdate(const OperationalModelUpdate& upd, GenericAgent& agent) const
{
    const auto& update = std::get<VelocityModelUpdate>(upd);
    agent.pos = update.position;
    agent.orientation = update.orientation;
}

void VelocityModel::CheckDistanceConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    const auto& model = std::get<VelocityModelData>(agent.model);
    const auto r = model.radius;
    for(const auto& neighbor : neighbors) {
        const auto& neighbor_model = std::get<VelocityModelData>(neighbor.model);
        const auto contanctdDist = r + neighbor_model.radius;
        const auto distance = (agent.pos - neighbor.pos).Norm();
        if(contanctdDist >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}",
                agent.pos,
                neighbor.pos,
                distance);
        }
    }
}

std::unique_ptr<OperationalModel> VelocityModel::Clone() const
{
    return std::make_unique<VelocityModel>(*this);
}

double VelocityModel::OptimalSpeed(const GenericAgent& ped, double spacing, double time_gap) const
{
    const auto& model = std::get<VelocityModelData>(ped.model);
    return std::min(std::max(spacing / time_gap, 0.0), model.v0);
}

double VelocityModel::GetSpacing(
    const GenericAgent& ped1,
    const GenericAgent& ped2,
    const Point& direction) const
{
    const auto& model1 = std::get<VelocityModelData>(ped1.model);
    const auto& model2 = std::get<VelocityModelData>(ped2.model);
    const auto distp12 = ped2.pos - ped1.pos;
    const auto inFront = direction.ScalarProduct(distp12) >= 0;
    if(!inFront) {
        return std::numeric_limits<double>::max();
    }

    const auto left = direction.Rotate90Deg();
    const auto l = model1.radius + model2.radius;
    bool inCorridor = std::abs(left.ScalarProduct(distp12)) <= l;
    if(!inCorridor) {
        return std::numeric_limits<double>::max();
    }

    if(ped1.id == -18)
        fmt::print(
            "Spacing: \n ped1: {}, ped2: {}. Dist: {}\ninFront: {} = {}, inCorridor: {} = {}\n direction: "
            "{}, distp12: {}",
            ped1.id,
            ped2.id,
            distp12.Norm() - l,
            inFront,
            direction.ScalarProduct(distp12) ,
            inCorridor,
            left.ScalarProduct(distp12),
            direction,
            distp12);
    
    return distp12.Norm() - l;
}
Point VelocityModel::NeighborRepulsion(const GenericAgent& ped1, const GenericAgent& ped2) const
{
    const auto distp12 = ped2.pos - ped1.pos;
    const auto [distance, direction] = distp12.NormAndNormalized();
    const auto& model1 = std::get<VelocityModelData>(ped1.model);
    const auto& model2 = std::get<VelocityModelData>(ped2.model);
    const auto l = model1.radius + model2.radius;
    Point desired_direction = (ped1.destination - ped1.pos).Normalized();
    if(desired_direction == Point()) {
        desired_direction = ped1.orientation;
    }
    bool newmodel = false1;
    // if((ped1.id == 3 && ped2.id == 35) || (ped1.id == 35 && ped2.id == 3))
    //     fmt::print(
    //         "------\n in neighbor function --\n ped1: {}, {}, ped2: {}, {}\n",
    //         ped1.id,
    //         ped1.pos,
    //         ped2.id,
    //         ped2.pos);
    if(newmodel) {
        const auto inFront = ped1.orientation.ScalarProduct(distp12) >= 0;

        if(!inFront) {
            // if((ped1.id == 3 && ped2.id == 35) || (ped1.id == 35 && ped2.id == 3))
            //     fmt::print("inFront {}\n\n", inFront);
            return Point();
        }
        auto rotated_desired_direction = desired_direction.Rotate90Deg();
        float dotProduct = rotated_desired_direction.ScalarProduct(direction);
        float sign = (dotProduct > 0) ? 1 : -1;
        Point result = rotated_desired_direction * -sign * (_aPed * exp((l - distance) / _DPed));
        // if((ped1.id == 3 && ped2.id == 35) || (ped1.id == 35 && ped2.id == 3))
        //     fmt::print("result: {}\n-------\n", result);
        return result;
    } else
        return direction * -(_aPed * exp((l - distance) / _DPed));
}

Point VelocityModel::BoundaryRepulsion(const GenericAgent& ped, const LineSegment& boundary_segment)
    const
{
    const auto pt = boundary_segment.ShortestPoint(ped.pos);
    const auto dist_vec = pt - ped.pos;
    const auto [dist, e_iw] = dist_vec.NormAndNormalized();
    const auto& model = std::get<VelocityModelData>(ped.model);
    const auto l = model.radius;
    const auto R_iw = -_aWall * exp((l - dist) / _DWall);
    return e_iw * R_iw;
}
