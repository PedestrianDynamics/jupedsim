// Copyright © 2012-2023 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <Unreachable.hpp>
#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_agent(py::module_& m)
{
    py::class_<JPS_AgentIterator_Wrapper>(m, "CollisionFreeSpeedModelAgentIterator")
        .def(
            "__iter__",
            [](JPS_AgentIterator_Wrapper& w) -> JPS_AgentIterator_Wrapper& { return w; })
        .def("__next__", [](JPS_AgentIterator_Wrapper& w) {
            const auto result = JPS_AgentIterator_Next(w.handle);
            if(result) {
                return std::make_unique<JPS_Agent_Wrapper>(result);
            }
            throw py::stop_iteration{};
        });
    py::class_<JPS_AgentIdIterator_Wrapper>(m, "AgentIdIterator")
        .def(
            "__iter__",
            [](JPS_AgentIdIterator_Wrapper& w) -> JPS_AgentIdIterator_Wrapper& { return w; })
        .def("__next__", [](JPS_AgentIdIterator_Wrapper& w) {
            const auto id = JPS_AgentIdIterator_Next(w.handle);
            if(id != 0) {
                return id;
            }
            throw py::stop_iteration{};
        });
    py::class_<JPS_Agent_Wrapper>(m, "Agent")
        .def_property_readonly(
            "id", [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetId(w.handle); })
        .def_property_readonly(
            "journey_id",
            [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetJourneyId(w.handle); })
        .def_property_readonly(
            "stage_id", [](const JPS_Agent_Wrapper& w) { return JPS_Agent_GetStageId(w.handle); })
        .def_property_readonly(
            "position",
            [](const JPS_Agent_Wrapper& w) { return intoTuple(JPS_Agent_GetPosition(w.handle)); })
        .def_property_readonly(
            "orientation",
            [](const JPS_Agent_Wrapper& w) {
                return intoTuple(JPS_Agent_GetOrientation(w.handle));
            })
        .def_property(
            "target",
            [](const JPS_Agent_Wrapper& w) { return intoTuple(JPS_Agent_GetTarget(w.handle)); },
            [](JPS_Agent_Wrapper& w, std::tuple<double, double> target) {
                JPS_ErrorMessage errorMsg{};
                auto success = JPS_Agent_SetTarget(w.handle, intoJPS_Point(target), &errorMsg);
                if(!success) {
                    auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                    JPS_ErrorMessage_Free(errorMsg);
                    throw std::runtime_error{msg};
                }
            })
        .def_property_readonly(
            "model",
            [](const JPS_Agent_Wrapper& w)
                -> std::variant<
                    std::unique_ptr<JPS_GeneralizedCentrifugalForceModelState_Wrapper>,
                    std::unique_ptr<JPS_CollisionFreeSpeedModelState_Wrapper>> {
                switch(JPS_Agent_GetModelType(w.handle)) {
                    case JPS_GeneralizedCentrifugalForceModel:
                        return std::make_unique<JPS_GeneralizedCentrifugalForceModelState_Wrapper>(
                            JPS_Agent_GetGeneralizedCentrifugalForceModelState(w.handle, nullptr));
                    case JPS_CollisionFreeSpeedModel:
                        return std::make_unique<JPS_CollisionFreeSpeedModelState_Wrapper>(
                            JPS_Agent_GetCollisionFreeSpeedModelState(w.handle, nullptr));
                }
                UNREACHABLE();
            });
}
