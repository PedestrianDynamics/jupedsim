// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/journey.h"

#include "jupedsim/error.h"

#include "ErrorMessage.hpp"
#include "JourneyDescription.hpp"

#include <Journey.hpp>
#include <StageDescription.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// JourneyDescription
////////////////////////////////////////////////////////////////////////////////////////////////////
using JourneyDesc = std::map<BaseStage::ID, TransitionDescription>;

JPS_JourneyDescription JPS_JourneyDescription_Create()
{
    return reinterpret_cast<JPS_JourneyDescription>(new JourneyDesc{});
}

void JPS_JourneyDescription_AddStage(JPS_JourneyDescription handle, JPS_StageId id)
{
    assert(handle);
    auto journeyDesc = reinterpret_cast<JourneyDesc*>(handle);
    (*journeyDesc)[BaseStage::ID{id}] = NonTransitionDescription{};
}

bool JPS_JourneyDescription_SetTransitionForStage(
    JPS_JourneyDescription handle,
    JPS_StageId id,
    JPS_Transition transition,
    JPS_ErrorMessage* errorMessage)
{
    assert(handle);
    assert(transition);
    auto journeyDesc = reinterpret_cast<JourneyDesc*>(handle);

    auto iter = journeyDesc->find(BaseStage::ID{id});
    if(iter != std::end(*journeyDesc)) {
        iter->second = *reinterpret_cast<TransitionDescription*>(transition);
        return true;
    }

    if(errorMessage) {
        *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{
            fmt::format("Could not set transition for given stage id {}. Stage not found.", id)});
    }
    return false;
}

void JPS_JourneyDescription_Free(JPS_JourneyDescription handle)
{
    delete reinterpret_cast<JourneyDesc*>(handle);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/// JourneyTransition
////////////////////////////////////////////////////////////////////////////////////////////////////
JPS_Transition
JPS_Transition_CreateFixedTransition(JPS_StageId stageId, JPS_ErrorMessage* errorMessage)
{
    JPS_Transition result{};
    try {
        result = reinterpret_cast<JPS_Transition>(
            new TransitionDescription(FixedTransitionDescription{stageId}));

    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

JPS_Transition JPS_Transition_CreateRoundRobinTransition(
    JPS_StageId* stages,
    uint64_t* weights,
    size_t len,
    JPS_ErrorMessage* errorMessage)
{
    JPS_Transition result{};
    std::vector<std::tuple<BaseStage::ID, uint64_t>> stageWeights;
    stageWeights.reserve(len);
    for(size_t i = 0; i < len; ++i) {
        stageWeights.emplace_back(std::make_tuple(stages[i], weights[i]));
    }

    try {
        result = reinterpret_cast<JPS_Transition>(
            new TransitionDescription(RoundRobinTransitionDescription{stageWeights}));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

JPS_Transition JPS_Transition_CreateLeastTargetedTransition(
    JPS_StageId* stages,
    size_t len,
    JPS_ErrorMessage* errorMessage)
{
    JPS_Transition result{};
    std::vector<BaseStage::ID> stageIds(stages, stages + len);
    try {
        result = reinterpret_cast<JPS_Transition>(
            new TransitionDescription(LeastTargetedTransitionDescription(std::move(stageIds))));
    } catch(const std::exception& ex) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(new JPS_ErrorMessage_t{ex.what()});
        }
    } catch(...) {
        if(errorMessage) {
            *errorMessage = reinterpret_cast<JPS_ErrorMessage>(
                new JPS_ErrorMessage_t{"Unknown internal error."});
        }
    }
    return result;
}

void JPS_Transition_Free(JPS_Transition handle)
{
    delete reinterpret_cast<JPS_Transition*>(handle);
}
