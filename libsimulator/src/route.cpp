// SPDX-License-Identifier: LGPL-3.0-or-later
#include "route.hpp"

#include "id_and_name.hpp"
#include "simulation_error.hpp"
#include "stage.hpp"
#include "stage_manager.hpp"
#include "visitor.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace
{
std::vector<BaseStage::ID> next_ids(const TransitionDescription& transition)
{
    return std::visit(
        Overloaded{
            [](const FixedTransitionDescription& d) {
                return std::vector<BaseStage::ID>{d.next_id()};
            },
            [](const RoundRobinTransitionDescription& d) {
                std::vector<BaseStage::ID> ids{};
                for(const auto& [id, _] : d.weighted_stages()) {
                    ids.push_back(id);
                }
                return ids;
            },
            [](const LeastTargetedTransitionDescription& d) { return d.target_candidates(); }},
        transition);
}

std::unique_ptr<Transition>
make_transition(const TransitionDescription& transition, StageManager& stages)
{
    return std::visit(
        Overloaded{
            [&stages](const FixedTransitionDescription& d) -> std::unique_ptr<Transition> {
                return std::make_unique<FixedTransition>(stages.stage(d.next_id()));
            },
            [&stages](const RoundRobinTransitionDescription& d) -> std::unique_ptr<Transition> {
                std::vector<std::tuple<BaseStage*, uint64_t>> weighted_stages{};
                for(const auto& [id, weight] : d.weighted_stages()) {
                    weighted_stages.emplace_back(stages.stage(id), weight);
                }
                return std::make_unique<RoundRobinTransition>(std::move(weighted_stages));
            },
            [&stages](const LeastTargetedTransitionDescription& d) -> std::unique_ptr<Transition> {
                std::vector<BaseStage*> candidates{};
                for(const auto& id : d.target_candidates()) {
                    candidates.push_back(stages.stage(id));
                }
                return std::make_unique<LeastTargetedTransition>(std::move(candidates));
            }},
        transition);
}

std::string list_of(const std::set<BaseStage::ID>& ids, const StageManager& stages)
{
    std::vector<std::string> listed{};
    for(const auto& id : ids) {
        listed.push_back(id_and_name(*stages.stage(id)));
    }
    return fmt::format("{}", fmt::join(listed, ", "));
}

/// Throws unless every stage in @p transitions is reached from the agent's location when the
/// directions of the transitions are ignored.
void check_connected(
    const std::vector<RouteTransitionDescription>& transitions,
    const StageManager& stages)
{
    std::map<BaseStage::ID, std::vector<BaseStage::ID>> neighbors{};
    std::vector<BaseStage::ID> unvisited{};
    for(const auto& [source, transition] : transitions) {
        for(const auto& next : next_ids(transition)) {
            neighbors[next];
            if(source) {
                neighbors[*source].push_back(next);
                neighbors[next].push_back(*source);
            } else {
                unvisited.push_back(next);
            }
        }
    }
    std::set<BaseStage::ID> connected{};
    while(!unvisited.empty()) {
        const auto id = unvisited.back();
        unvisited.pop_back();
        if(connected.insert(id).second) {
            unvisited.insert(unvisited.end(), neighbors[id].begin(), neighbors[id].end());
        }
    }
    std::set<BaseStage::ID> apart{};
    for(const auto& [id, _] : neighbors) {
        if(!connected.contains(id)) {
            apart.insert(id);
        }
    }
    if(!apart.empty()) {
        throw InvalidValueError(
            "{} {} {} not connected to the rest of the route; define {} as a route of {} own.",
            apart.size() == 1 ? "Target" : "Targets",
            list_of(apart, stages),
            apart.size() == 1 ? "is" : "are",
            apart.size() == 1 ? "it" : "them",
            apart.size() == 1 ? "its" : "their");
    }
}
} // namespace

Route::Route(
    const std::vector<RouteTransitionDescription>& transitions,
    std::optional<std::string> name,
    StageManager& stages)
    : _name(std::move(name))
{
    if(transitions.empty()) {
        throw InvalidValueError("The route is empty.");
    }
    for(const auto& [source, transition] : transitions) {
        if(source) {
            stages.stage(*source);
        }
        for(const auto& next : next_ids(transition)) {
            stages.stage(next);
        }
    }
    for(const auto& [source, transition] : transitions) {
        if(!source) {
            continue;
        }
        const auto& stage = *stages.stage(*source);
        if(dynamic_cast<const Exit*>(&stage) != nullptr) {
            throw InvalidValueError(
                "Target {} is an exit: agents leave the simulation there, so no transition can "
                "leave it.",
                id_and_name(stage));
        }
        const auto next = next_ids(transition);
        if(std::find(next.begin(), next.end(), *source) == next.end()) {
            continue;
        }
        if(std::holds_alternative<FixedTransitionDescription>(transition)) {
            throw InvalidValueError(
                "Target {} cannot follow itself; remove the self-transition to keep agents there.",
                id_and_name(stage));
        }
        throw InvalidValueError(
            "Target {} cannot follow itself; remove it from the targets of its own transition.",
            id_and_name(stage));
    }
    std::map<std::optional<BaseStage::ID>, std::size_t> outgoing{};
    for(const auto& [source, _] : transitions) {
        ++outgoing[source];
    }
    for(const auto& [source, count] : outgoing) {
        if(source && count > 1) {
            throw InvalidValueError(
                "Target {} has {} outgoing transitions; combine them into one, e.g. a "
                "jps.RoundRobinTransition.",
                id_and_name(*stages.stage(*source)),
                count);
        }
    }
    if(!outgoing.contains(std::nullopt)) {
        throw InvalidValueError(
            "No transition leaves AGENT_LOCATION; add one, e.g. "
            "jps.FixedTransition(jps.AGENT_LOCATION, ...).");
    }
    if(outgoing[std::nullopt] > 1) {
        throw InvalidValueError(
            "{} transitions leave AGENT_LOCATION; combine them into one, e.g. a "
            "jps.RoundRobinTransition.",
            outgoing[std::nullopt]);
    }
    check_connected(transitions, stages);

    for(const auto& [source, transition] : transitions) {
        if(source) {
            _transitions[*source] = make_transition(transition, stages);
        } else {
            _entry = make_transition(transition, stages);
        }
        for(const auto& next : next_ids(transition)) {
            _transitions.try_emplace(next);
        }
    }
}

BaseStage* Route::next_stage(BaseStage::ID reached)
{
    const auto& transition = _transitions.at(reached);
    return transition ? transition->next_stage() : nullptr;
}

std::vector<BaseStage::ID> Route::stage_ids() const
{
    std::vector<BaseStage::ID> ids{};
    for(const auto& [id, _] : _transitions) {
        ids.push_back(id);
    }
    return ids;
}
