// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry.hpp"
#include "IteratorPair.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModelType.hpp"
#include "OptimalStepsModel.hpp"
#include "SimulationError.hpp"

#include <boost/iterator/zip_iterator.hpp>

#include <iterator>
#include <memory>
#include <queue>
#include <vector>

class OperationalDecisionSystemInterface
{
public:
    virtual ~OperationalDecisionSystemInterface() = default;
    virtual void
    Run(double dT,
        double /*t_in_sec*/,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry,
        std::vector<GenericAgent>& agents) = 0;

    virtual void ValidateAgent(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const = 0;
};

class OperationalDecisionSystem : public OperationalDecisionSystemInterface
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
        std::vector<GenericAgent>& agents) override
    {
        std::vector<std::optional<OperationalModelUpdate>> updates{};
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

    void ValidateAgent(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override
    {
        _model->CheckModelConstraint(agent, neighborhoodSearch, geometry);
    }
};

class OperationalDecisionSystemOSM : public OperationalDecisionSystemInterface
{
    using Cmp = std::function<bool(const GenericAgent*, const GenericAgent*)>;

    std::unique_ptr<OperationalModel> _model{};
    std::priority_queue<GenericAgent*, std::vector<GenericAgent*>, Cmp> _agentActionEvents;

public:
    OperationalDecisionSystemOSM(std::unique_ptr<OperationalModel>&& model)
        : _model(std::move(model)), _agentActionEvents([](const auto lhs, const auto rhs) {
            const auto& lhsModel = std::get<OptimalStepsModelData>(lhs->model);
            const auto& rhsModel = std::get<OptimalStepsModelData>(rhs->model);

            if(lhsModel.nextTimeToAct == rhsModel.nextTimeToAct) {
                return lhs->id > rhs->id;
            }

            return lhsModel.nextTimeToAct > rhsModel.nextTimeToAct;
        })
    {
    }
    ~OperationalDecisionSystemOSM() = default;
    OperationalDecisionSystemOSM(const OperationalDecisionSystemOSM& other) = delete;
    OperationalDecisionSystemOSM& operator=(const OperationalDecisionSystemOSM& other) = delete;
    OperationalDecisionSystemOSM(OperationalDecisionSystemOSM&& other) = delete;
    OperationalDecisionSystemOSM& operator=(OperationalDecisionSystemOSM&& other) = delete;

    OperationalModelType ModelType() const { return _model->Type(); }

    void
    Run(double dT,
        double t_in_sec,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry,
        std::vector<GenericAgent>& agents) override
    {
        // clear prio queue
        // agents in queue
        _agentActionEvents = std::priority_queue<GenericAgent*, std::vector<GenericAgent*>, Cmp>();
        std::for_each(std::begin(agents), std::end(agents), [this](auto& agent) {
            _agentActionEvents.push(&agent);
        });

        // peek in queue as long as nextTimeToAct < t_in_sec + dT
        while(const auto& nextElement = _agentActionEvents.top();
              std::get<OptimalStepsModelData>(nextElement->model).nextTimeToAct < t_in_sec + dt) {
            // update pos
            // enqueue agent
        }
    }

    void ValidateAgent(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override
    {
        _model->CheckModelConstraint(agent, neighborhoodSearch, geometry);
    }
};

std::unique_ptr<OperationalDecisionSystemInterface>
MakeOperationalDecisionSystem(std::unique_ptr<OperationalModel>&& model);