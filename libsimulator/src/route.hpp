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

    /// The stage after @p completed, or nullptr where the route ends.
    BaseStage* next_stage(BaseStage::ID completed);

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
