// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "simulation_error.hpp"
#include "stage.hpp"
#include "stage_manager.hpp"
#include "unique_id.hpp"

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

class FixedTransitionDescription
{
    BaseStage::ID _next{};

public:
    FixedTransitionDescription(BaseStage::ID next) : _next(next)
    {
        if(next == BaseStage::ID::invalid.get_id()) {
            throw SimulationError("Can not create fixed transition from invalid target id.");
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
        if(_weighted_stages.empty()) {
            throw SimulationError("A round robin transition needs at least one target.");
        }
        for(const auto& [stage_id, _] : _weighted_stages) {
            if(stage_id == BaseStage::ID::invalid.get_id()) {
                throw SimulationError(
                    "Can not create round robin transition from invalid target id.");
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
        if(_target_candidates.empty()) {
            throw SimulationError("A least targeted transition needs at least one target.");
        }
        for(const auto& stage_id : _target_candidates) {
            if(stage_id == BaseStage::ID::invalid.get_id()) {
                throw SimulationError(
                    "Can not create least targeted transition from invalid target id.");
            }
        }
    }

    const std::vector<BaseStage::ID>& target_candidates() const { return _target_candidates; }
};

using TransitionDescription = std::variant<
    FixedTransitionDescription,
    RoundRobinTransitionDescription,
    LeastTargetedTransitionDescription>;

/// One transition of a route: the stage it leaves, and how it chooses the next one.
struct RouteTransitionDescription {
    /// std::nullopt for the agent's location when it joins the route: this transition chooses
    /// the agent's first stage.
    std::optional<BaseStage::ID> source;
    TransitionDescription transition;
};

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

/// Smooth weighted round-robin (as in nginx): spreads each target's turns evenly over the cycle.
/// Each decision credits every target with its weight; the target with the most credit (ties: the
/// earlier one) is chosen and pays the sum of the weights. So within every sum-of-weights
/// decisions each target is chosen exactly weight times.
class RoundRobinTransition : public Transition
{
public:
    /// Keeps the current weights far from overflow.
    static constexpr uint64_t max_weight = 1'000'000'000;

private:
    struct Target {
        BaseStage* stage;
        int64_t weight;
        int64_t current;
    };
    std::vector<Target> _targets{};
    int64_t _sum_weights{};

public:
    RoundRobinTransition(const std::vector<std::tuple<BaseStage*, uint64_t>>& weighted_stages)
    {
        _targets.reserve(weighted_stages.size());
        for(auto const& [stage, weight] : weighted_stages) {
            if(weight == 0) {
                throw SimulationError("RoundRobinTransition no weight may be zero.");
            }
            if(weight > max_weight) {
                throw SimulationError(
                    "RoundRobinTransition weights must be at most {}.", max_weight);
            }
            _targets.push_back({stage, static_cast<int64_t>(weight), 0});
            _sum_weights += static_cast<int64_t>(weight);
        }
    }

    BaseStage* next_stage() override
    {
        Target* chosen{};
        for(auto& target : _targets) {
            target.current += target.weight;
            if(chosen == nullptr || target.current > chosen->current) {
                chosen = &target;
            }
        }
        chosen->current -= _sum_weights;
        return chosen->stage;
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

/// A directed graph of stages, given by its transitions. Every route has its own transitions,
/// so a round robin counts per route.
class Route
{
public:
    using ID = jps::UniqueID<Route>;

private:
    ID _id{};
    std::optional<std::string> _name;
    std::unique_ptr<Transition> _entry;
    /// Every stage of the route, with the transition leaving it; nullptr where the route ends.
    std::map<BaseStage::ID, std::unique_ptr<Transition>> _transitions;

public:
    /// Throws unless exactly one of @p transitions leaves the agent's location, at most one leaves
    /// each stage, none leaves an exit, none leads back to its own stage, and all of
    /// them form one graph when their directions are ignored.
    Route(
        const std::vector<RouteTransitionDescription>& transitions,
        std::optional<std::string> name,
        StageManager& stages);

    ID id() const { return _id; }
    const std::optional<std::string>& name() const { return _name; }

    /// The stage an agent joining the route goes to first.
    BaseStage* first_stage() { return _entry->next_stage(); }

    /// The stage after @p reached, or nullptr where the route ends.
    BaseStage* next_stage(BaseStage::ID reached);

    bool contains(BaseStage::ID stage_id) const { return _transitions.contains(stage_id); }

    /// Ids of all stages of the route, in ascending order.
    std::vector<BaseStage::ID> stage_ids() const;
};

template <>
struct fmt::formatter<Route> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Route& r, FormatContext& ctx) const
    {
        if(r.name()) {
            return fmt::format_to(
                ctx.out(),
                "Route(id={}, name={}, targets=[{}])",
                r.id(),
                *r.name(),
                fmt::join(r.stage_ids(), ", "));
        }
        return fmt::format_to(
            ctx.out(), "Route(id={}, targets=[{}])", r.id(), fmt::join(r.stage_ids(), ", "));
    }
};
