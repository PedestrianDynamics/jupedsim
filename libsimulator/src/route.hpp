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
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

class NoRuleDescription
{
};

/// The agent stays at the stage: completing it arrives there.
class StayDescription
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

class LeastTravelTimeDescription
{
private:
    std::vector<BaseStage::ID> _target_candidates;

public:
    LeastTravelTimeDescription(std::vector<BaseStage::ID> target_candidates)
        : _target_candidates(std::move(target_candidates))
    {
        for(const auto& stage_id : _target_candidates) {
            if(stage_id == BaseStage::ID::invalid.get_id()) {
                throw SimulationError(
                    "Can not create least travel time rule from invalid stage id.");
            }
        }
    }

    const std::vector<BaseStage::ID>& target_candidates() const { return _target_candidates; }
};

using RuleDescription = std::variant<
    NoRuleDescription,
    StayDescription,
    FixedRuleDescription,
    RoundRobinDescription,
    LeastTargetedDescription,
    LeastTravelTimeDescription>;

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
    /// The stage to go to next, or nullptr if every target is closed.
    virtual BaseStage* next_stage(const RuleContext& ctx) = 0;
    /// Whether @p stage is one of the rule's targets, whatever its weight.
    virtual bool targets(const BaseStage& stage) const = 0;
};

class FixedRule : public Rule
{
private:
    BaseStage* _next;

public:
    FixedRule(BaseStage* next) : _next(next) {};

    BaseStage* next_stage([[maybe_unused]] const RuleContext& ctx) override
    {
        return _next->closed() ? nullptr : _next;
    }

    bool targets(const BaseStage& stage) const override { return _next == &stage; }
};

/// Keeps the agent at its stage: the next stage is the stage itself, so the agent goes on
/// steering to it, and completing the stage arrives there (see Route::step).
class Stay : public Rule
{
private:
    BaseStage* _stage;

public:
    Stay(BaseStage* stage) : _stage(stage) {};

    BaseStage* next_stage([[maybe_unused]] const RuleContext& ctx) override { return _stage; }

    bool targets(const BaseStage& stage) const override { return _stage == &stage; }
};

/// Smooth weighted round-robin: each decision adds every target's weight to its current weight,
/// picks the target with the largest current weight (ties: the earlier one) and takes the sum of
/// the weights off it. Only the ratio of the weights matters. Closed targets and targets of weight
/// zero take no part: their current weights stay as they are, so the open targets keep their
/// ratio, and with no target left the rule is unchanged.
/// Weights are at most max_weight, so the current weights cannot overflow.
class RoundRobin : public Rule
{
public:
    static constexpr uint64_t max_weight = 1'000'000'000;

private:
    struct Target {
        BaseStage* stage;
        uint64_t weight;
        int64_t current;
    };
    std::vector<Target> _targets{};

public:
    RoundRobin(const std::vector<std::tuple<BaseStage*, uint64_t>>& weighted_stages)
    {
        _targets.reserve(weighted_stages.size());
        for(auto const& [stage, weight] : weighted_stages) {
            check_weight(weight);
            _targets.push_back({stage, weight, 0});
        }
        check_some_positive();
    }

    BaseStage* next_stage([[maybe_unused]] const RuleContext& ctx) override
    {
        Target* chosen{};
        int64_t sum_weights = 0;
        for(auto& target : _targets) {
            if(target.weight == 0 || target.stage->closed()) {
                continue;
            }
            const auto weight = static_cast<int64_t>(target.weight);
            target.current += weight;
            sum_weights += weight;
            if(chosen == nullptr || target.current > chosen->current) {
                chosen = &target;
            }
        }
        if(chosen == nullptr) {
            return nullptr;
        }
        chosen->current -= sum_weights;
        return chosen->stage;
    }

    bool targets(const BaseStage& stage) const override
    {
        return std::any_of(std::begin(_targets), std::end(_targets), [&stage](const auto& t) {
            return t.stage == &stage;
        });
    }

    /// Sets the weight of every target from @p weights and starts the sequence afresh. @p weights
    /// names exactly the targets, each at most max_weight, at least one > 0; otherwise throws and
    /// changes nothing.
    void set_weights(const std::map<BaseStage::ID, uint64_t>& weights)
    {
        check_keys(weights);
        uint64_t sum_weights = 0;
        for(const auto& [_, weight] : weights) {
            check_weight(weight);
            sum_weights += weight;
        }
        if(sum_weights == 0) {
            throw SimulationError("RoundRobin needs at least one weight > 0.");
        }
        for(auto& target : _targets) {
            target.weight = weights.at(target.stage->id());
            target.current = 0;
        }
    }

private:
    static void check_weight(uint64_t weight)
    {
        if(weight > max_weight) {
            throw SimulationError("RoundRobin weights must be at most {}.", max_weight);
        }
    }

    void check_some_positive() const
    {
        if(std::none_of(std::begin(_targets), std::end(_targets), [](const auto& t) {
               return t.weight > 0;
           })) {
            throw SimulationError("RoundRobin needs at least one weight > 0.");
        }
    }

    /// Throws unless @p weights has a key for every target and no other key.
    void check_keys(const std::map<BaseStage::ID, uint64_t>& weights) const
    {
        const auto named = [&weights](const auto& t) { return weights.count(t.stage->id()) == 1; };
        const auto is_target = [this](const auto& entry) {
            return std::any_of(std::begin(_targets), std::end(_targets), [&entry](const auto& t) {
                return t.stage->id() == entry.first;
            });
        };
        if(!std::all_of(std::begin(_targets), std::end(_targets), named) ||
           !std::all_of(std::begin(weights), std::end(weights), is_target)) {
            throw SimulationError("RoundRobin weights must name exactly the rule's targets.");
        }
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

    /// Ties go to the first candidate.
    BaseStage* next_stage([[maybe_unused]] const RuleContext& ctx) override
    {
        BaseStage* least_targeted{};
        for(auto* candidate : _target_candidates) {
            if(candidate->closed()) {
                continue;
            }
            if(least_targeted == nullptr ||
               candidate->count_targeting() < least_targeted->count_targeting()) {
                least_targeted = candidate;
            }
        }
        return least_targeted;
    }

    bool targets(const BaseStage& stage) const override
    {
        return std::find(std::begin(_target_candidates), std::end(_target_candidates), &stage) !=
               std::end(_target_candidates);
    }
};

/// Picks the open candidate with the least travel time from the agent's position.
class LeastTravelTime : public Rule
{
private:
    std::vector<BaseStage*> _target_candidates;

public:
    LeastTravelTime(std::vector<BaseStage*> target_candidates)
        : _target_candidates(std::move(target_candidates))
    {
    }

    /// Ties go to the first candidate.
    BaseStage* next_stage(const RuleContext& ctx) override
    {
        BaseStage* nearest{};
        double least_time = std::numeric_limits<double>::infinity();
        for(auto* candidate : _target_candidates) {
            if(candidate->closed()) {
                continue;
            }
            const auto time = travel_time(ctx, *candidate);
            if(nearest == nullptr || time < least_time) {
                nearest = candidate;
                least_time = time;
            }
        }
        return nearest;
    }

    bool targets(const BaseStage& stage) const override
    {
        return std::find(std::begin(_target_candidates), std::end(_target_candidates), &stage) !=
               std::end(_target_candidates);
    }

private:
    static double travel_time(const RuleContext& ctx, const BaseStage& stage)
    {
        const auto* destination = stage.destination();
        if(destination == nullptr) {
            return std::numeric_limits<double>::infinity();
        }
        return ctx.routing.travel_time(ctx.agent.location, *destination);
    }
};

/// One decision step of an agent: the stage to go to, nullptr if the agent is stranded, and the
/// stage whose rule decided. With @p held the agent completed a held stage and stops there; with
/// @p arrived it completed a stage whose rule is Stay.
struct RouteStep {
    BaseStage* stage;
    std::optional<BaseStage::ID> decision_stage;
    bool held{false};
    bool arrived{false};
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
    std::set<BaseStage::ID> _holds{};

public:
    ~Route() = default;

    Route(std::map<BaseStage::ID, RouteNode> stages) : _stages(std::move(stages)) {}

    ID id() const { return _id; }

    /// The agent keeps its stage until it completes it; then the stage's rule decides. An agent
    /// reaching a closed exit is not removed: the rule that sent it there decides again. An agent
    /// completing a held stage stops there. An agent completing a stage whose rule is Stay has
    /// arrived; it keeps the stage and the decision stage.
    RouteStep step(const GenericAgent& agent, const RoutingEngine& routing) const
    {
        const auto& node = _stages.at(agent.stage_id);
        auto* stage = node.stage;
        if(!stage->is_completed(agent)) {
            return {stage, agent.decision_stage};
        }
        const bool stays = dynamic_cast<const Stay*>(node.rule.get()) != nullptr;
        // A hold placed after an agent has arrived does not stop it.
        if(holds(stage->id()) && !(stays && agent.arrived)) {
            return {stage, stage->id(), true};
        }
        if(stays) {
            return {stage, agent.decision_stage, false, true};
        }
        const auto at = stage->closed() ? agent.decision_stage.value_or(stage->id()) : stage->id();
        return {decide(agent, at, routing), at};
    }

    /// The stage the rule of stage @p at picks for @p agent, nullptr if all its targets are closed.
    BaseStage*
    decide(const GenericAgent& agent, BaseStage::ID at, const RoutingEngine& routing) const
    {
        return _stages.at(at).rule->next_stage(RuleContext{agent, routing});
    }

    /// The rule at stage @p at.
    Rule& rule(BaseStage::ID at) const { return *_stages.at(at).rule; }

    /// Agents completing stage @p at stop there until release().
    void hold(BaseStage::ID at) { _holds.insert(at); }

    void release(BaseStage::ID at) { _holds.erase(at); }

    bool holds(BaseStage::ID at) const { return _holds.contains(at); }

    size_t count_stages() const { return _stages.size(); }

    bool contains_stage(BaseStage::ID stage_id) const
    {
        const auto find_iter = _stages.find(stage_id);
        return find_iter != std::end(_stages);
    }

    const std::map<BaseStage::ID, RouteNode>& stages() const { return _stages; };
};
