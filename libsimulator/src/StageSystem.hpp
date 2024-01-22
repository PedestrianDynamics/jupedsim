// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "Stage.hpp"
#include <unordered_map>

class StageSystem
{

public:
    StageSystem() {}
    ~StageSystem() = default;
    StageSystem(const StageSystem& other) = delete;
    StageSystem& operator=(const StageSystem& other) = delete;
    StageSystem(StageSystem&& other) = delete;
    StageSystem& operator=(StageSystem&& other) = delete;

    void
    Run(StageManager& stageManager,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry)
    {
        for(auto& [_, stage] : stageManager.Stages()) {
            if(auto* updatable_stage = dynamic_cast<NotifiableWaitingSet*>(stage.get());
               updatable_stage != nullptr) {
                updatable_stage->Update(neighborhoodSearch, geometry);
            } else if(auto* updatable_stage = dynamic_cast<NotifiableQueue*>(stage.get());
                      updatable_stage != nullptr) {
                updatable_stage->Update(neighborhoodSearch, geometry);
            }
        }
    }
};
