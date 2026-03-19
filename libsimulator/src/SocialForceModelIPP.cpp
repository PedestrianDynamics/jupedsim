// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModelIPP.hpp"

#include "GenericAgent.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModelType.hpp"
#include "SimulationError.hpp"
#include "SocialForceModelIPPData.hpp"

#include <stdexcept>

SocialForceModelIPP::SocialForceModelIPP() {};

OperationalModelType SocialForceModelIPP::Type() const
{
    return OperationalModelType::SOCIAL_FORCE_IPP;
}

std::unique_ptr<OperationalModel> SocialForceModelIPP::Clone() const
{
    return std::make_unique<SocialForceModelIPP>(*this);
}

OperationalModelUpdate SocialForceModelIPP::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const auto& model = std::get<SocialForceModelIPPData>(ped.model);
    SocialForceModelIPPUpdate update{};

    const auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, this->_cutOffRadius);
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    // Unit vector from ground support toward upper body
    Point e_n = (ped.pos - model.ground_support_position).Normalized();

    // --- Upper body acceleration ---
    // Driving force: (v0 * e0 - v) / tau
    auto acc_ub = DrivingForce(ped);

    // Unbalancing: lambda_u * (-v_s * e_n - v_n)
    // Upper body "falls" away from legs (negative e_n direction)
    acc_ub += (e_n * (-model.balanceSpeed) - model.velocity) * model.lambdaU;

    // Damping: -lambda * v_n
    acc_ub = acc_ub - model.velocity * model.damping;

    // Upper body repulsion from other agents
    for(const auto& neighbor : neighborhood) {
        if(neighbor.id == ped.id) {
            continue;
        }
        acc_ub += ExponentialRepulsion(ped.pos, neighbor.pos, model.agentScale, model.forceDistance);
    }

    // Upper body repulsion from walls
    for(const auto& wall : walls) {
        const Point pt = wall.ShortestPoint(ped.pos);
        acc_ub += ExponentialRepulsion(ped.pos, pt, model.agentScale, model.forceDistance);
    }

    // --- Ground support (leg) acceleration ---
    // Balance recovery: lambda_b * (v_s * e_n - v_leg)
    // Legs "chase" the upper body (positive e_n direction)
    auto acc_gs = (e_n * model.balanceSpeed - model.ground_support_velocity) * model.lambdaB;

    // Leg repulsion from other agents' legs
    for(const auto& neighbor : neighborhood) {
        if(neighbor.id == ped.id) {
            continue;
        }
        const auto& nmodel = std::get<SocialForceModelIPPData>(neighbor.model);
        acc_gs += ExponentialRepulsion(
            model.ground_support_position,
            nmodel.ground_support_position,
            model.agentScale,
            model.legForceDistance);
    }

    // Leg repulsion from walls
    for(const auto& wall : walls) {
        const Point pt = wall.ShortestPoint(model.ground_support_position);
        acc_gs +=
            ExponentialRepulsion(model.ground_support_position, pt, model.agentScale, model.legForceDistance);
    }

    // --- Euler integration ---
    update.velocity = model.velocity + acc_ub * dT;
    update.position = ped.pos + update.velocity * dT;

    update.ground_support_velocity = model.ground_support_velocity + acc_gs * dT;
    update.ground_support_position = model.ground_support_position + update.ground_support_velocity * dT;

    return update;
}

void SocialForceModelIPP::ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent)
    const
{
    auto& model = std::get<SocialForceModelIPPData>(agent.model);
    const auto& upd = std::get<SocialForceModelIPPUpdate>(update);
    agent.pos = upd.position;
    agent.orientation = upd.velocity.Normalized();
    model.velocity = upd.velocity;
    model.ground_support_position = upd.ground_support_position;
    model.ground_support_velocity = upd.ground_support_velocity;
}

void SocialForceModelIPP::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearchType& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    auto throwIfNegative = [](double value, std::string name) {
        if(value < 0) {
            throw SimulationError(
                "Model constraint violation: {} {} not in allowed range, "
                "{} needs to be positive",
                name,
                value,
                name);
        }
    };

    const auto& model = std::get<SocialForceModelIPPData>(agent.model);

    throwIfNegative(model.desiredSpeed, "desired speed");
    throwIfNegative(model.reactionTime, "reaction time");
    throwIfNegative(model.radius, "radius");
    throwIfNegative(model.lambdaU, "unbalancing rate");
    throwIfNegative(model.lambdaB, "balancing rate");
    throwIfNegative(model.balanceSpeed, "balance speed");
    throwIfNegative(model.damping, "damping");
    throwIfNegative(model.agentScale, "agent scale");
    throwIfNegative(model.forceDistance, "force distance");
    throwIfNegative(model.legForceDistance, "leg force distance");

    const auto neighbors = neighborhoodSearch.GetNeighboringAgents(agent.pos, 2);
    for(const auto& neighbor : neighbors) {
        const auto distance = (agent.pos - neighbor.pos).Norm();
        if(model.radius >= distance) {
            throw SimulationError(
                "Model constraint violation: Agent {} too close to agent {}: distance {}, "
                "radius {}",
                agent.pos,
                neighbor.pos,
                distance,
                model.radius);
        }
    }
    const auto maxRadius = model.radius / 2;
    const auto lineSegments = geometry.LineSegmentsInDistanceTo(maxRadius, agent.pos);
    if(std::begin(lineSegments) != std::end(lineSegments)) {
        throw SimulationError(
            "Model constraint violation: Agent {} too close to geometry boundaries, distance <= "
            "{}/2",
            agent.pos,
            model.radius);
    }
}

Point SocialForceModelIPP::DrivingForce(const GenericAgent& agent)
{
    const auto& model = std::get<SocialForceModelIPPData>(agent.model);
    const Point e0 = (agent.destination - agent.pos).Normalized();
    return (e0 * model.desiredSpeed - model.velocity) / model.reactionTime;
}

Point SocialForceModelIPP::ExponentialRepulsion(
    const Point pt1,
    const Point pt2,
    const double A,
    const double B)
{
    const double dist = (pt1 - pt2).Norm();
    if(dist < 1e-10) {
        return Point(0, 0);
    }
    const Point n_ij = (pt1 - pt2).Normalized();
    return n_ij * A * exp(-dist / B);
}
