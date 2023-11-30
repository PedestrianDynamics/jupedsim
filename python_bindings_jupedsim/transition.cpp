// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_transition(py::module_& m)
{
    py::class_<JPS_Transition_Wrapper>(m, "Transition")
        .def_static(
            "create_fixed_transition",
            [](JPS_StageId stageId) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Transition_CreateFixedTransition(stageId, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Transition_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def_static(
            "create_round_robin_transition",
            [](const std::vector<std::tuple<JPS_StageId, uint64_t>>& stageWeights) {
                JPS_ErrorMessage errorMsg{};
                std::vector<JPS_StageId> stageIds;
                stageIds.reserve(stageWeights.size());
                std::vector<uint64_t> weights;
                weights.reserve(stageWeights.size());
                for(auto const& [stageId, weight] : stageWeights) {
                    stageIds.emplace_back(stageId);
                    weights.emplace_back(weight);
                }
                auto result = JPS_Transition_CreateRoundRobinTransition(
                    stageIds.data(), weights.data(), stageIds.size(), &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Transition_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def_static("create_least_targeted_transition", [](const std::vector<JPS_StageId>& stages) {
            JPS_ErrorMessage errorMsg{};
            std::vector<JPS_StageId> stageIds;
            stageIds.reserve(stages.size());
            std::copy(std::begin(stages), std::end(stages), std::back_inserter(stageIds));

            auto result = JPS_Transition_CreateLeastTargetedTransition(
                stageIds.data(), stageIds.size(), &errorMsg);
            if(result) {
                return std::make_unique<JPS_Transition_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
}
