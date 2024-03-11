#include "SocialForceModel.hpp"

#include "Ellipse.hpp"
#include "GenericAgent.hpp"
#include "Macros.hpp"
#include "Mathematics.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "Simulation.hpp"
#include "SocialForceModelData.hpp"

#include <Logger.hpp>
#include <iostream>
#include <stdexcept>

SocialForceModel::SocialForceModel(double bodyForce_, double friction_)
    : bodyForce(bodyForce_), friction(friction_){};

OperationalModelType SocialForceModel::Type() const
{
    return OperationalModelType::SOCIAL_FORCE;
}

std::unique_ptr<OperationalModel> SocialForceModel::Clone() const
{
    return std::make_unique<SocialForceModel>(*this);
}

OperationalModelUpdate SocialForceModel::ComputeNewPosition(
    double dT,
    const GenericAgent& ped,
    const CollisionGeometry& geometry,
    const NeighborhoodSearchType& neighborhoodSearch) const
{
    const auto& model = std::get<SocialForceModelData>(ped.model);
    SocialForceModelUpdate update{};
    auto forces = DrivingForce(ped);

    const auto neighborhood = neighborhoodSearch.GetNeighboringAgents(ped.pos, this->_cutOffRadius);
    Point F_rep;
    for(const auto& neighbor : neighborhood) {
        if(neighbor.id == ped.id) {
            continue;
        }
        forces += AgentForce(ped, neighbor) / model.mass;
    }

    const auto& walls = geometry.LineSegmentsInApproxDistanceTo(ped.pos);

    auto obstacle_f = std::accumulate(
        walls.cbegin(),
        walls.cend(),
        Point(0, 0),
        [this, &ped](const auto& acc, const auto& element) {
            return acc + ObstacleForce(ped, element);
        });
    forces += obstacle_f / model.mass;

    update.velocity = model.velocity + forces * dT;
    update.position = ped.pos + update.velocity * dT;

    return update;
}

void SocialForceModel::ApplyUpdate(const OperationalModelUpdate& update, GenericAgent& agent) const
{
    auto& model = std::get<SocialForceModelData>(agent.model);
    const auto& upd = std::get<SocialForceModelUpdate>(update);
    agent.pos = upd.position;
    model.velocity = upd.velocity;
    agent.orientation = upd.velocity.Normalized();
}

void SocialForceModel::CheckModelConstraint(
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

    const auto& model = std::get<SocialForceModelData>(agent.model);

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

Point SocialForceModel::DrivingForce(const GenericAgent& agent) const
{
    auto& model = std::get<SocialForceModelData>(agent.model);
    Point e0 = (agent.destination - agent.pos).Normalized();
    return (e0 * model.desiredSpeed - model.velocity) / model.reactionTime;
};
double SocialForceModel::Pushing_Force(double A, double B, double r, double distance) const
{
    return A * exp((r - distance) / B);
}

Point SocialForceModel::AgentForce(const GenericAgent& ped1, const GenericAgent& ped2) const
{
    double dist = (ped1.pos - ped2.pos).Norm();
    auto& model1 = std::get<SocialForceModelData>(ped1.model);
    auto& model2 = std::get<SocialForceModelData>(ped2.model);

    double total_radius = model1.radius + model2.radius;
    // todo reduce range of force to 180 degrees
    double pushing_force_length =
        Pushing_Force(model1.agentScale, model1.forceDistance, total_radius, dist);
    double friction_force_length = 0;
    Point n_ij = (ped1.pos - ped2.pos).Normalized();
    Point tangent = n_ij.Rotate90Deg();

    if(dist < total_radius) {
        pushing_force_length += this->bodyForce * (total_radius - dist);
        friction_force_length += this->friction * (total_radius - dist) *
                                 ((model2.velocity - model1.velocity).ScalarProduct(tangent));
    }

    return n_ij * pushing_force_length + tangent * friction_force_length;
};

Point SocialForceModel::ObstacleForce(const GenericAgent& agent, const LineSegment& segment) const
{
    auto& model = std::get<SocialForceModelData>(agent.model);
    Point pt = segment.ShortestPoint(agent.pos);
    double dist = (agent.pos - pt).Norm();
    double pushing_force_length =
        Pushing_Force(model.obstacleScale, model.forceDistance, model.radius, dist);
    double friction_force_length = 0;
    Point n_ij = (agent.pos - pt).Normalized();
    Point tangent = n_ij.Rotate90Deg();

    if(dist < model.radius) {
        pushing_force_length += this->bodyForce * (model.radius - dist);
        friction_force_length +=
            this->friction * (model.radius - dist) * (model.velocity.ScalarProduct(tangent));
    }
    return n_ij * pushing_force_length + tangent * friction_force_length;
}