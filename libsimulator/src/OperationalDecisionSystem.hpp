/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry.hpp"
#include "IteratorPair.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "SimulationError.hpp"

#include <boost/iterator/zip_iterator.hpp>

#include <iterator>
#include <memory>
#include <vector>

template <typename Model>
class OperationalDecisionSystem
{
    std::unique_ptr<Model> _model{};

public:
    OperationalDecisionSystem(std::unique_ptr<Model>&& model) : _model(std::move(model)) {}
    ~OperationalDecisionSystem() = default;
    OperationalDecisionSystem(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem& operator=(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem(OperationalDecisionSystem&& other) = delete;
    OperationalDecisionSystem& operator=(OperationalDecisionSystem&& other) = delete;

    void
    Run(double dT,
        double /*t_in_sec*/,
        const typename Model::NeighborhoodSearchType& neighborhoodSearch,
        const CollisionGeometry& geometry,
        std::vector<typename Model::Data>& agents) const
    {
        std::vector<std::optional<PedestrianUpdate>> updates{};
        updates.reserve(agents.size());

        std::transform(
            std::begin(agents),
            std::end(agents),
            std::back_inserter(updates),
            [this, &dT, &geometry, &neighborhoodSearch](const auto& agent) {
                return _model->ComputeNewPosition(dT, agent, geometry, neighborhoodSearch);
            });

        std::for_each(
            boost::make_zip_iterator(boost::make_tuple(std::begin(agents), std::begin(updates))),
            boost::make_zip_iterator(boost::make_tuple(std::end(agents), std::end(updates))),
            [this](auto tup) {
                auto& [agent, update] = tup;
                if(update) {
                    _model->ApplyUpdate(*update, agent);
                }
            });
    }

    void ValidateAgentParameterProfileId(OperationalModel::ParametersID id) const
    {
        if(!_model->ParameterProfileExists(id)) {
            throw SimulationError("Unknown parameters profile id \"{}\" supplied", id.getID());
        }
    }

    void ValidateAgent(
        const typename Model::Data& agent,
        const typename Model::NeighborhoodSearchType& neighborhoodSearch) const
    {
        _model->CheckDistanceConstraint(agent, neighborhoodSearch);
    }
};
