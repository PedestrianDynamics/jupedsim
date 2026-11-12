// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "geometry/location.hpp"
#include "journey.hpp"
#include "stage.hpp"
#include "stage_manager.hpp"
#include "thread_pool.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

class StrategicalDecisionSystem
{
public:
    StrategicalDecisionSystem() = default;
    ~StrategicalDecisionSystem() = default;
    StrategicalDecisionSystem(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem& operator=(const StrategicalDecisionSystem& other) = delete;
    StrategicalDecisionSystem(StrategicalDecisionSystem&& other) = delete;
    StrategicalDecisionSystem& operator=(StrategicalDecisionSystem&& other) = delete;

    void
    run(const std::unordered_map<Journey::ID, std::unique_ptr<Journey>>& journeys,
        auto&& agents,
        StageManager& stage_manager) const
    {
        for(auto& agent : agents) {
            const auto [target, id] = journeys.at(agent.journey_id)->target(agent);
            agent.final_target = target;
            stage_manager.migrate_agent(agent.stage_id, id);
            agent.stage_id = id;
        }
    }

    /// Same result as run(), in three passes:
    ///  1. serial: solve the destination of every stage some agent targets;
    ///  2. parallel: per agent, test whether it reached its stage (read-only) and, if not,
    ///     refresh its target -- its stage stays, and migrating to the same stage is a no-op;
    ///  3. serial, in container order: for agents that reached their stage apply what run()
    ///     does -- exit bookkeeping, the transition (round robin and least targeted keep state
    ///     and read the targeting counts, so order matters) and the targeting counters.
    /// Returns whether any agent ends up heading for a plain location, which only the serial
    /// routing engine can route; floor-field destinations it leaves prepared.
    bool run_parallel(
        const std::unordered_map<Journey::ID, std::unique_ptr<Journey>>& journeys,
        auto&& agents,
        StageManager& stage_manager,
        ThreadPool& pool)
    {
        const std::size_t n = std::size(agents);
        stage_manager.for_each_stage([](BaseStage& stage) {
            if(stage.count_targeting() > 0) {
                if(const auto* d = stage.destination()) {
                    d->prepare();
                }
            }
        });
        // Written only for the rare agents that reached their stage, so threads do not share
        // cache lines through it.
        _reached.assign(n, 0);
        std::atomic<bool> any_location{false};
        pool.parallel_for(n, std::max<std::size_t>(1, n / (pool.size() * 16)), [&](std::size_t i) {
            auto& agent = agents[i];
            BaseStage* stage = journeys.at(agent.journey_id)->stages().at(agent.stage_id).stage;
            if(stage->reached_prepared(agent)) {
                _reached[i] = 1;
                return;
            }
            agent.final_target = stage->target(agent);
            if(std::holds_alternative<Location>(agent.final_target)) {
                any_location.store(true, std::memory_order_relaxed);
            }
        });
        bool location_target = any_location.load();
        for(std::size_t i = 0; i < n; ++i) {
            if(_reached[i] == 0) {
                continue;
            }
            auto& agent = agents[i];
            const auto& node = journeys.at(agent.journey_id)->stages().at(agent.stage_id);
            node.stage->on_completed(agent);
            BaseStage* next = node.transition->next_stage();
            if(const auto* d = next->destination()) {
                d->prepare();
            }
            agent.final_target = next->target(agent);
            location_target =
                location_target || std::holds_alternative<Location>(agent.final_target);
            stage_manager.migrate_agent(agent.stage_id, next->id());
            agent.stage_id = next->id();
        }
        return location_target;
    }

private:
    std::vector<char> _reached{};
};
