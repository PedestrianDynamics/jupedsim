// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SocialForceModelIPP.hpp"

#include "Ellipse.hpp"
#include "GenericAgent.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Simulation.hpp"
#include "SocialForceModelIPPData.hpp"

#include <Logger.hpp>
#include <iostream>
#include <stdexcept>

SocialForceModelIPP::SocialForceModelIPP(double bodyForce_, double friction_)
    : bodyForce(bodyForce_), friction(friction_) {};

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
    auto forces = DrivingForce(ped);

    const auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, this->_cutOffRadius);
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        if(neighbor.id == ped.id) {
            continue;
        }
        F_rep += AgentForce(ped, neighbor);
    }
    forces += F_rep / model.mass;
    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    const auto obstacle_f = std::accumulate(
        walls.cbegin(),
        walls.cend(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + ObstacleForce(ped, element);
        });
    forces += obstacle_f / model.mass;



    // IPP model parameteres
    // Model Antoine
    // Point vb = 1; // balancing speed
    // double lambda_u = 0.5; // unbalancing rate
    // double lambda_b = 1; // Balancing rate

    // update.velocity = model.velocity + ( ( - vb - model.velocity) * lambda_u - model.velocity + forces) * dT;
    
    // update.ground_support_velocity =    model.ground_support_velocity + 
    //                                     (   
    //                                         (vb - model.velocity) * lambda_b 
    //                                     )* dT;
                    
    // update.position = ped.pos + update.velocity * dT;
    // update.ground_support_position = model.ground_support_position + update.ground_support_velocity * dT;





    // // model perso 
    double lambda_friction = 0.5; 
    Point vb = 1; // balancing speed
    double lambda_u = 0.5; // unbalancing rate
    double lambda_b = 10; // Balancing rate

    update.velocity = model.velocity + ( ( - vb - model.velocity) * lambda_u - model.velocity + forces) * dT;
    update.position = ped.pos + update.velocity * dT;
    update.ground_support_velocity =    model.ground_support_velocity + 
                                        (   
                                            (ped.pos - model.ground_support_position) * lambda_b 
                                            - model.ground_support_velocity * lambda_friction
                                        )* dT;
    update.ground_support_position = model.ground_support_position + update.ground_support_velocity * dT;

    // printf(
    //     "pos=(%.2f, %.2f), vel=(%.2f, %.2f), gs_pos=(%.2f, %.2f), gs_vel=(%.2f, %.2f)\n",
    //     update.position.x,
    //     update.position.y,
    //     update.velocity.x,
    //     update.velocity.y,
    //     update.ground_support_position.x,
    //     update.ground_support_position.y,
    //     update.ground_support_velocity.x,
    //     update.ground_support_velocity.y);
    // std::cin.get(); //pause

    return update;
}

void SocialForceModelIPP::ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const
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
    // none of these constraint are given by the paper but are useful to create a simulation that
    // does not break immediately
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

    const auto mass = model.mass;
    throwIfNegative(mass, "mass");

    const auto desiredSpeed = model.desiredSpeed;
    throwIfNegative(desiredSpeed, "desired speed");

    const auto reactionTime = model.reactionTime;
    throwIfNegative(reactionTime, "reaction time");

    const auto radius = model.radius;
    throwIfNegative(radius, "radius");

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
};
double SocialForceModelIPP::PushingForceLength(double A, double B, double r, double distance)
{
    return A * exp((r - distance) / B);
}

Point SocialForceModelIPP::AgentForce(const GenericAgent& ped1, const GenericAgent& ped2) const
{
    const auto& model1 = std::get<SocialForceModelIPPData>(ped1.model);
    const auto& model2 = std::get<SocialForceModelIPPData>(ped2.model);

    const double total_radius = model1.radius + model2.radius;

    return ForceBetweenPoints(
        ped1.pos,
        ped2.pos,
        model1.agentScale,
        model1.forceDistance,
        total_radius,
        model2.velocity - model1.velocity);
};

Point SocialForceModelIPP::ObstacleForce(const GenericAgent& agent, const LineSegment& segment) const
{
    const auto& model = std::get<SocialForceModelIPPData>(agent.model);
    const Point pt = segment.ShortestPoint(agent.pos);
    return ForceBetweenPoints(
        agent.pos, pt, model.obstacleScale, model.forceDistance, model.radius, model.velocity);
}

Point SocialForceModelIPP::ForceBetweenPoints(
    const Point pt1,
    const Point pt2,
    const double A,
    const double B,
    const double radius,
    const Point velocity) const
{
    // todo reduce range of force to 180 degrees
    const double dist = (pt1 - pt2).Norm();
    double pushing_force_length = PushingForceLength(A, B, radius, dist);
    double friction_force_length = 0;
    const Point n_ij = (pt1 - pt2).Normalized();
    const Point tangent = n_ij.Rotate90Deg();
    if(dist < radius) {
        pushing_force_length += this->bodyForce * (radius - dist);
        friction_force_length =
            this->friction * (radius - dist) * (velocity.ScalarProduct(tangent));
    }
    return n_ij * pushing_force_length + tangent * friction_force_length;
}
