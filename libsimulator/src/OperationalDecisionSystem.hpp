// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry.hpp"
#include "IteratorPair.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "SimulationError.hpp"

#include <boost/iterator/zip_iterator.hpp>

#include <iterator>
#include <memory>
#include <optional>
#include <vector>

class OperationalDecisionSystem
{
    std::unique_ptr<OperationalModel> _model{};

public:
    OperationalDecisionSystem(std::unique_ptr<OperationalModel>&& model) : _model(std::move(model))
    {
    }
    ~OperationalDecisionSystem() = default;
    OperationalDecisionSystem(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem& operator=(const OperationalDecisionSystem& other) = delete;
    OperationalDecisionSystem(OperationalDecisionSystem&& other) = delete;
    OperationalDecisionSystem& operator=(OperationalDecisionSystem&& other) = delete;

    OperationalModelType ModelType() const { return _model->Type(); }

    void
    Run(double dT,
        double /*t_in_sec*/,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry,
        std::vector<GenericAgent>& agents) const
    {
        std::vector<std::optional<OperationalModelUpdate>> updates{};
        updates.reserve(agents.size());

        using OptVec = const std::vector<GenericAgent>*;
        auto getCell = [](const auto& n, Grid2DIndex idx) -> OptVec {
            const auto iter = n._grid.find(idx);
            if(iter == std::end(n._grid)) {
                return nullptr;
            } else {
                return &iter->second;
            }
        };
        for(const auto& [key, value] : neighborhoodSearch._grid) {
            std::vector<GenericAgent> neighbors{};
            for(int x = -1; x < 2; ++x) {
                for(int y = -1; y < 2; ++y) {
                    auto c = getCell(neighborhoodSearch, {key.idx + x, key.idy + y});
                    if(c != nullptr) {
                        neighbors.insert(neighbors.end(), c->begin(), c->end());
                    }
                }
            }

            for(const auto& agent : value) {
                _model->ComputeNewPosition(dT, agent, geometry, neighbors);
            }
        }

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

    void ValidateAgent(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const
    {
        _model->CheckModelConstraint(agent, neighborhoodSearch, geometry);
    }
};
