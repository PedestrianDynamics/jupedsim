// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "generic_agent.hpp"
#include "routing_engine.hpp"
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

class NoRuleDescription
{
};

class FixedRuleDescription
{
    BaseStage::ID _next{};

public:
    FixedRuleDescription(BaseStage::ID next) : _next(next)
    {
        if(next == BaseStage::ID::invalid.get_id()) {
            throw SimulationError("Can not create fixed rule from invalid stage id.");
        }
    };

    BaseStage::ID next_id() const { return _next; }
};

class RoundRobinDescription
{
    std::vector<std::tuple<BaseStage::ID, uint64_t>> _weighted_stages{};

public:
    RoundRobinDescription(const std::vector<std::tuple<BaseStage::ID, uint64_t>>& weighted_stages)
        : _weighted_stages(weighted_stages)
    {
        for(const auto& [stage_id, _] : _weighted_stages) {
            if(stage_id == BaseStage::ID::invalid.get_id()) {
                throw SimulationError("Can not create round robin rule from invalid stage id.");
            }
        }
    };

    const std::vector<std::tuple<BaseStage::ID, uint64_t>>& weighted_stages() const
    {
        return _weighted_stages;
    }
};

class LeastTargetedDescription
{
private:
    std::vector<BaseStage::ID> _target_candidates;

public:
    LeastTargetedDescription(std::vector<BaseStage::ID> target_candidates)
        : _target_candidates(std::move(target_candidates))
    {
        for(const auto& stage_id : _target_candidates) {
            if(stage_id == BaseStage::ID::invalid.get_id()) {
                throw SimulationError("Can not create least targeted rule from invalid stage id.");
            }
        }
    }

    const std::vector<BaseStage::ID>& target_candidates() const { return _target_candidates; }
};

using RuleDescription = std::variant<
    NoRuleDescription,
    FixedRuleDescription,
    RoundRobinDescription,
    LeastTargetedDescription>;

/// What a rule may consult when it decides: the agent deciding and the routing engine for
/// travel-time queries.
struct RuleContext {
    const GenericAgent& agent;
    const RoutingEngine& routing;
};

class Rule
{
public:
    virtual ~Rule() = default;
    virtual BaseStage* next_stage(const RuleContext& ctx) = 0;
};

class FixedRule : public Rule
{
private:
    BaseStage* _next;

public:
    FixedRule(BaseStage* next) : _next(next) {};

    BaseStage* next_stage([[maybe_unused]] const RuleContext& ctx) override { return _next; }
};

class RoundRobin : public Rule
{
private:
    std::vector<std::tuple<BaseStage*, uint64_t>> _weighted_stages{};
    uint64_t _next_called{};
    uint64_t _sum_weights{};

public:
    RoundRobin(std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages)
        : _weighted_stages(std::move(weighted_stages))
    {
        for(auto const& [_, weight] : _weighted_stages) {
            if(weight == 0) {
                throw SimulationError("RoundRobin no weight may be zero.");
            }
            _sum_weights += weight;
        }
    }

    BaseStage* next_stage([[maybe_unused]] const RuleContext& ctx) override
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

class LeastTargeted : public Rule
{
private:
    std::vector<BaseStage*> _target_candidates;

public:
    LeastTargeted(std::vector<BaseStage*> target_candidates)
        : _target_candidates(std::move(target_candidates))
    {
    }

    BaseStage* next_stage([[maybe_unused]] const RuleContext& ctx) override
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

struct RouteNode {
    BaseStage* stage;
    std::unique_ptr<Rule> rule;
};

class Route
{
public:
    using ID = jps::UniqueID<Route>;

private:
    ID _id{};
    std::map<BaseStage::ID, RouteNode> _stages{};

public:
    ~Route() = default;

    Route(std::map<BaseStage::ID, RouteNode> stages) : _stages(std::move(stages)) {}

    ID id() const { return _id; }

    std::tuple<RoutingTarget, BaseStage::ID>
    target(const GenericAgent& agent, const RoutingEngine& routing) const
    {
        auto& node = _stages.at(agent.stage_id);
        auto stage = node.stage;
        const auto& rule = node.rule;

        if(stage->is_completed(agent)) {
            stage = rule->next_stage(RuleContext{agent, routing});
        }

        return std::make_tuple(stage->target(agent), stage->id());
    }

    size_t count_stages() const { return _stages.size(); }

    bool contains_stage(BaseStage::ID stage_id) const
    {
        const auto find_iter = _stages.find(stage_id);
        return find_iter != std::end(_stages);
    }

    const std::map<BaseStage::ID, RouteNode>& stages() const { return _stages; };
};
