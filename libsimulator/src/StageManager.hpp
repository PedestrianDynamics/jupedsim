// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "SimulationError.hpp"
#include "Stage.hpp"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

class StageManager
{
private:
    std::unordered_map<BaseStage::ID, std::unique_ptr<BaseStage>> _stages;

public:
    StageManager() {}
    ~StageManager() = default;
    StageManager(const StageManager& other) = delete;
    StageManager& operator=(const StageManager& other) = delete;
    StageManager(StageManager&& other) = delete;
    StageManager& operator=(StageManager&& other) = delete;

    BaseStage::ID add_stage(std::unique_ptr<BaseStage> stage)
    {
        if(_stages.find(stage->id()) != _stages.end()) {
            throw SimulationError("Internal error, stage id already in use.");
        }
        const auto id = stage->id();
        _stages.emplace(id, std::move(stage));

        return id;
    }

    void migrate_agent(BaseStage::ID prev_target, BaseStage::ID new_target)
    {
        _stages.at(new_target)->increase_targeting();
        _stages.at(prev_target)->decrease_targeting();
    }

    void handle_new_agent(BaseStage::ID stage_id) { _stages.at(stage_id)->increase_targeting(); }
    void handle_remove_agent(BaseStage::ID stage_id) { _stages.at(stage_id)->decrease_targeting(); }

    BaseStage* stage(BaseStage::ID stage_id) const
    {
        const auto iter = _stages.find(stage_id);
        if(iter == std::end(_stages)) {
            throw SimulationError("Unknown stage id ({}) provided in journey.", stage_id.get_id());
        }
        return iter->second.get();
    }

    BaseStage* stage(BaseStage::ID stage_id)
    {
        auto iter = _stages.find(stage_id);
        if(iter == std::end(_stages)) {
            throw SimulationError("Unknown stage id ({}) provided in journey.", stage_id.get_id());
        }
        return iter->second.get();
    }

    std::unordered_map<BaseStage::ID, std::unique_ptr<BaseStage>>& stages() { return _stages; }

    const std::unordered_map<BaseStage::ID, std::unique_ptr<BaseStage>>& stages() const
    {
        return _stages;
    }
};
