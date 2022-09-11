#pragma once

#include "Agent.hpp"
#include "Geometry.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"

#include <memory>
#include <vector>

class OperationalDecisionSystem
{
    std::unique_ptr<OperationalModel> _model{};

public:
    OperationalDecisionSystem(std::unique_ptr<OperationalModel>&& model);
    ~OperationalDecisionSystem() = default;
    OperationalDecisionSystem(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem& operator=(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem(OperationalDecisionSystem&& other) = delete;
    OperationalDecisionSystem& operator=(OperationalDecisionSystem&& other) = delete;

    void
    Run(double dT,
        double t_in_sec,
        const NeighborhoodSearch& neighborhoodSearch,
        const CollisionGeometry& geometry,
        std::vector<std::unique_ptr<Agent>>& agents) const;

    void ValidateAgentParameterProfileId(OperationalModel::ParametersID id) const;
};
