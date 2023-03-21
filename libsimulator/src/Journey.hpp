/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Point.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "UniqueID.hpp"

#include <memory>
#include <tuple>
#include <vector>

class Journey
{
public:
    using ID = jps::UniqueID<Journey>;

private:
    ID id;
    std::vector<std::unique_ptr<Stage>> stages{};

public:
    ~Journey() = default;

    Journey(
        const std::vector<StageDescription>& stageDescriptions,
        std::vector<GenericAgent::ID>& toRemove);

    ID Id() const { return id; }

    std::tuple<Point, size_t> Target(const GenericAgent& agent) const;
};
