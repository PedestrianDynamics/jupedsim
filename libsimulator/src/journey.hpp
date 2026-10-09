// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "point.hpp"
#include "routing_target.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "unique_id.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

class NonTransitionDescription
{
};

class FixedTransitionDescription
{
    BaseStage::ID _next{};

public:
    FixedTransitionDescription(BaseStage::ID next) : _next(next)
    {
        if(next == BaseStage::ID::invalid.get_id()) {
            throw SimulationError("Can not create fixed transition from invalid stage id.");
        }
    };

    BaseStage::ID next_id() const { return _next; }
};

class RoundRobinTransitionDescription
{
    std::vector<std::tuple<BaseStage::ID, uint64_t>> _weighted_stages{};

public:
    RoundRobinTransitionDescription(
        const std::vector<std::tuple<BaseStage::ID, uint64_t>>& weighted_stages)
        : _weighted_stages(weighted_stages)
    {
        for(const auto& [stage_id, _] : _weighted_stages) {
            if(stage_id == BaseStage::ID::invalid.get_id()) {
                throw SimulationError(
                    "Can not create round robin transition from invalid stage id.");
            }
        }
    };

    const std::vector<std::tuple<BaseStage::ID, uint64_t>>& weighted_stages() const
    {
        return _weighted_stages;
    }
};

class LeastTargetedTransitionDescription
{
private:
    std::vector<BaseStage::ID> _target_candidates;

public:
    LeastTargetedTransitionDescription(std::vector<BaseStage::ID> target_candidates)
        : _target_candidates(std::move(target_candidates))
    {
        for(const auto& stage_id : _target_candidates) {
            if(stage_id == BaseStage::ID::invalid.get_id()) {
                throw SimulationError(
                    "Can not create least targeted transition from invalid stage id.");
            }
        }
    }

    const std::vector<BaseStage::ID>& target_candidates() const { return _target_candidates; }
};

using TransitionDescription = std::variant<
    NonTransitionDescription,
    FixedTransitionDescription,
    RoundRobinTransitionDescription,
    LeastTargetedTransitionDescription>;

class Transition
{
public:
    virtual ~Transition() = default;
    virtual BaseStage* next_stage() = 0;
};

class FixedTransition : public Transition
{
private:
    BaseStage* _next;

public:
    FixedTransition(BaseStage* next) : _next(next) {};

    BaseStage* next_stage() override { return _next; }
};

class RoundRobinTransition : public Transition
{
private:
    std::vector<std::tuple<BaseStage*, uint64_t>> _weighted_stages{};
    uint64_t _next_called{};
    uint64_t _sum_weights{};

public:
    RoundRobinTransition(std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages)
        : _weighted_stages(std::move(weighted_stages))
    {
        for(auto const& [_, weight] : _weighted_stages) {
            if(weight == 0) {
                throw SimulationError("RoundRobinTransition no weight may be zero.");
            }
            _sum_weights += weight;
        }
    }

    BaseStage* next_stage() override
    {
        uint64_t sum_weights_so_far = 0;
        BaseStage* candidate{};
        for(const auto& [stage, weight] : _weighted_stages) {
            if(sum_weights_so_far <= _next_called) {
                candidate = stage;
            } else {
                break;
            }
            sum_weights_so_far += weight;
        }
        _next_called = (_next_called + 1) % _sum_weights;
        return candidate;
    }
};

class LeastTargetedTransition : public Transition
{
private:
    std::vector<BaseStage*> _target_candidates;

public:
    LeastTargetedTransition(std::vector<BaseStage*> target_candidates)
        : _target_candidates(std::move(target_candidates))
    {
    }

    BaseStage* next_stage() override
    {
        auto least_targeted = std::min_element(
            std::begin(_target_candidates),
            std::end(_target_candidates),
            [](auto const& a, auto const& b) {
                return a->count_targeting() < b->count_targeting();
            });
        return *least_targeted;
    }
};

struct JourneyNode {
    BaseStage* stage;
    std::unique_ptr<Transition> transition;
};

class Journey
{
public:
    using ID = jps::UniqueID<Journey>;

private:
    ID _id{};
    std::map<BaseStage::ID, JourneyNode> _stages{};

public:
    ~Journey() = default;

    Journey(std::map<BaseStage::ID, JourneyNode> stages) : _stages(std::move(stages)) {}

    ID id() const { return _id; }

    std::tuple<RoutingTarget, BaseStage::ID> target(const GenericAgent& agent) const
    {
        auto& node = _stages.at(agent.stage_id);
        auto stage = node.stage;
        const auto& transition = node.transition;

        if(stage->is_completed(agent)) {
            stage = transition->next_stage();
        }

        return std::make_tuple(stage->target(agent), stage->id());
    }

    size_t count_stages() const { return _stages.size(); }

    bool contains_stage(BaseStage::ID stage_id) const
    {
        const auto find_iter = _stages.find(stage_id);
        return find_iter != std::end(_stages);
    }

    const std::map<BaseStage::ID, JourneyNode>& stages() const { return _stages; };
};
