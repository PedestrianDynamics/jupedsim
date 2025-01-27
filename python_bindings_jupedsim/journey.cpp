// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_journey(py::module_& m)
{
    py::class_<JPS_JourneyDescription_Wrapper>(m, "JourneyDescription")
        .def(py::init([]() {
            return std::make_unique<JPS_JourneyDescription_Wrapper>(
                JPS_JourneyDescription_Create());
        }))
        .def(py::init([](const std::vector<JPS_StageId>& ids) {
            auto desc =
                std::make_unique<JPS_JourneyDescription_Wrapper>(JPS_JourneyDescription_Create());
            for(const auto id : ids) {
                JPS_JourneyDescription_AddStage(desc->handle, id);
            }
            return desc;
        }))
        .def(
            "add",
            [](JPS_JourneyDescription_Wrapper& w, JPS_StageId id) {
                JPS_JourneyDescription_AddStage(w.handle, id);
            })
        .def(
            "add",
            [](JPS_JourneyDescription_Wrapper& w, const std::vector<JPS_StageId>& ids) {
                for(const auto& id : ids) {
                    JPS_JourneyDescription_AddStage(w.handle, id);
                }
            })
        .def(
            "set_transition_for_stage",
            [](JPS_JourneyDescription_Wrapper& w,
               JPS_StageId stageId,
               JPS_Transition_Wrapper& transition) {
                JPS_ErrorMessage errorMsg{};
                auto success = JPS_JourneyDescription_SetTransitionForStage(
                    w.handle, stageId, transition.handle, &errorMsg);
                if(!success) {
                    auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                    JPS_ErrorMessage_Free(errorMsg);
                    throw std::runtime_error{msg};
                }
            });
}
