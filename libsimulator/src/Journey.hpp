// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "RoutingEngine.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "UniqueID.hpp"

#include <map>
#include <memory>
#include <tuple>
#include <vector>

class NonTransitionDescription
{
};

class FixedTransitionDescription
{
    BaseStage::ID next{};

public:
    FixedTransitionDescription(BaseStage::ID next_) : next(next_)
    {
        if(next_ == BaseStage::ID::Invalid.getID()) {
            throw SimulationError("Can not create fixed transition from invalid stage id.");
        }
    };

    BaseStage::ID NextId() const { return next; }
};

class RoundRobinTransitionDescription
{
    std::vector<std::tuple<BaseStage::ID, uint64_t>> weightedStages{};

public:
    RoundRobinTransitionDescription(
        const std::vector<std::tuple<BaseStage::ID, uint64_t>>& weightedStages_)
        : weightedStages(weightedStages_)
    {
        for(const auto& [stageId, _] : weightedStages) {
            if(stageId == BaseStage::ID::Invalid.getID()) {
                throw SimulationError(
                    "Can not create round robin transition from invalid stage id.");
            }
        }
    };

    const std::vector<std::tuple<BaseStage::ID, uint64_t>>& WeightedStages() const
    {
        return weightedStages;
    }
};

class LeastTargetedTransitionDescription
{
private:
    std::vector<BaseStage::ID> targetCandidates;

public:
    LeastTargetedTransitionDescription(std::vector<BaseStage::ID> targetCandidates_)
        : targetCandidates(std::move(targetCandidates_))
    {
        for(const auto& stageId : targetCandidates) {
            if(stageId == BaseStage::ID::Invalid.getID()) {
                throw SimulationError(
                    "Can not create least targeted transition from invalid stage id.");
            }
        }
    }

    const std::vector<BaseStage::ID>& TargetCandidates() const { return targetCandidates; }
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
    virtual BaseStage* NextStage() = 0;
};

class FixedTransition : public Transition
{
private:
    BaseStage* next;

public:
    FixedTransition(BaseStage* next_) : next(next_){};

    BaseStage* NextStage() override { return next; }
};

class RoundRobinTransition : public Transition
{
private:
    std::vector<std::tuple<BaseStage*, uint64_t>> weightedStages{};
    uint64_t nextCalled{};
    uint64_t sumWeights{};

public:
    RoundRobinTransition(std::vector<std::tuple<BaseStage*, uint64_t>> weightedStages_)
        : weightedStages(std::move(weightedStages_))
    {
        for(auto const& [_, weight] : weightedStages) {
            if(weight == 0) {
                throw SimulationError("RoundRobinTransition no weight may be zero.");
            }
            sumWeights += weight;
        }
    }

    BaseStage* NextStage() override
    {
        uint64_t sumWeightsSoFar = 0;
        BaseStage* candidate{};
        for(const auto& [stage, weight] : weightedStages) {
            if(sumWeightsSoFar <= nextCalled) {
                candidate = stage;
            } else {
                break;
            }
            sumWeightsSoFar += weight;
        }
        nextCalled = (nextCalled + 1) % sumWeights;
        return candidate;
    }
};

class LeastTargetedTransition : public Transition
{
private:
    std::vector<BaseStage*> targetCandidates;

public:
    LeastTargetedTransition(std::vector<BaseStage*> targetCandidates_)
        : targetCandidates(std::move(targetCandidates_))
    {
    }

    BaseStage* NextStage() override
    {
        auto leastTargeted = std::min_element(
            std::begin(targetCandidates),
            std::end(targetCandidates),
            [](auto const& a, auto const& b) { return a->CountTargeting() < b->CountTargeting(); });
        return *leastTargeted;
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
    ID id{};
    std::map<BaseStage::ID, JourneyNode> stages{};

public:
    ~Journey() = default;

    Journey(std::map<BaseStage::ID, JourneyNode> stages_) : stages(std::move(stages_)) {}

    ID Id() const { return id; }

    std::tuple<Point, BaseStage::ID> Target(const GenericAgent& agent) const
    {
        auto& node = stages.at(agent.stageId);
        auto stage = node.stage;
        const auto& transition = node.transition;

        if(stage->IsCompleted(agent)) {
            stage = transition->NextStage();
        }

        return std::make_tuple(stage->Target(agent), stage->Id());
    }

    size_t CountStages() const { return stages.size(); }

    bool ContainsStage(BaseStage::ID stageId) const
    {
        const auto find_iter = stages.find(stageId);
        return find_iter != std::end(stages);
    }

    const std::map<BaseStage::ID, JourneyNode>& Stages() const { return stages; };
};
