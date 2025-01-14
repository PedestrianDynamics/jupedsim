// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "jupedsim/anticipation_velocity_model.h"
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_anticipation_velocity_model(py::module_& m)
{
    py::class_<JPS_AnticipationVelocityModelAgentParameters>(
        m, "AnticipationVelocityModelAgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        double time_gap,
                        double v0,
                        double radius,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double wallBufferDistance,
                        double anticipationTime,
                        double reactionTime) {
                return JPS_AnticipationVelocityModelAgentParameters{
                    intoJPS_Point(position),
                    journey_id,
                    stage_id,
                    time_gap,
                    v0,
                    radius,
                    strengthNeighborRepulsion,
                    rangeNeighborRepulsion,
                    wallBufferDistance,
                    anticipationTime,
                    reactionTime};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("time_gap"),
            py::arg("v0"),
            py::arg("radius"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("wall_buffer_distance"),
            py::arg("anticipation_time"),
            py::arg("reaction_time"))
        .def("__repr__", [](const JPS_AnticipationVelocityModelAgentParameters& p) {
            return fmt::format(
                "position: {}, journey_id: {}, stage_id: {}, "
                "time_gap: {}, v0: {}, radius: {}",
                "strength_neighbor_repulsion: {}, range_neighbor_repulsion: {}"
                "wall_buffer_distance: {}"
                "anticipation_time: {}, reaction_time: {}",
                intoTuple(p.position),
                p.journeyId,
                p.stageId,
                p.time_gap,
                p.v0,
                p.radius,
                p.strengthNeighborRepulsion,
                p.rangeNeighborRepulsion,
                p.wallBufferDistance,
                p.anticipationTime,
                p.reactionTime);
        });
    py::class_<JPS_AnticipationVelocityModelBuilder_Wrapper>(m, "AnticipationVelocityModelBuilder")
        .def(py::init([]() {
            return std::make_unique<JPS_AnticipationVelocityModelBuilder_Wrapper>(
                JPS_AnticipationVelocityModelBuilder_Create());
        }))
        .def("build", [](JPS_AnticipationVelocityModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_AnticipationVelocityModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_AnticipationVelocityModelState_Wrapper>(m, "AnticipationVelocityModelState")
        .def_property(
            "time_gap",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetTimeGap(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double time_gap) {
                JPS_AnticipationVelocityModelState_SetTimeGap(w.handle, time_gap);
            })
        .def_property(
            "v0",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetV0(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double v0) {
                JPS_AnticipationVelocityModelState_SetV0(w.handle, v0);
            })
        .def_property(
            "radius",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetRadius(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double radius) {
                JPS_AnticipationVelocityModelState_SetRadius(w.handle, radius);
            })
        .def_property(
            "strength_neighbor_repulsion",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetStrengthNeighborRepulsion(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double strengthNeighborRepulsion) {
                JPS_AnticipationVelocityModelState_SetStrengthNeighborRepulsion(
                    w.handle, strengthNeighborRepulsion);
            })
        .def_property(
            "range_neighbor_repulsion",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetRangeNeighborRepulsion(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double rangeNeighborRepulsion) {
                JPS_AnticipationVelocityModelState_SetRangeNeighborRepulsion(
                    w.handle, rangeNeighborRepulsion);
            })
        .def_property(
            "wall_buffer_distance",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetWallBufferDistance(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double wallBufferDistance) {
                JPS_AnticipationVelocityModelState_SetWallBufferDistance(
                    w.handle, wallBufferDistance);
            })
        .def_property(
            "anticipation_time",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetAnticipationTime(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double anticipationTime) {
                JPS_AnticipationVelocityModelState_SetAnticipationTime(w.handle, anticipationTime);
            })
        .def_property(
            "reaction_time",
            [](const JPS_AnticipationVelocityModelState_Wrapper& w) {
                return JPS_AnticipationVelocityModelState_GetReactionTime(w.handle);
            },
            [](JPS_AnticipationVelocityModelState_Wrapper& w, double reactionTime) {
                JPS_AnticipationVelocityModelState_SetReactionTime(w.handle, reactionTime);
            });
}
