/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry.hpp"
#include "IteratorPair.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"

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
        std::vector<GenericAgent>& agents,
        std::vector<typename Model::Data>& modelData) const
    {
        std::vector<std::optional<PedestrianUpdate>> updates{};
        updates.reserve(agents.size());

        auto modelDataIter = std::begin(modelData);
        for(auto& genericAgent : agents) {
            updates.push_back(_model->ComputeNewPosition(
                dT, genericAgent, *modelDataIter, geometry, neighborhoodSearch));
            ++modelDataIter;
        }

        modelDataIter = std::begin(modelData);
        auto genericAgentIter = std::begin(agents);
        for(const auto& update : updates) {
            if(update) {
                _model->ApplyUpdate(*update, *genericAgentIter, *modelDataIter);
            }
            ++modelDataIter;
            ++genericAgentIter;
        }
    }

    void ValidateAgentParameterProfileId(OperationalModel::ParametersID id) const
    {
        if(!_model->ParameterProfileExists(id)) {
            throw new std::runtime_error(
                fmt::format("Unknown parameters profile id \"{}\" supplied", id.getID()));
        }
    }
};
