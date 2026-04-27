// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_collision_free_speed_model_v3(py::module_& m)
{
    py::class_<JPS_CollisionFreeSpeedModelV3AgentParameters>(
        m, "CollisionFreeSpeedModelV3AgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        double time_gap,
                        double desired_speed,
                        double radius,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        double strength_neighbor_repulsion,
                        double range_neighbor_repulsion,
                        double strength_geometry_repulsion,
                        double range_geometry_repulsion,
                        double range_x_scale,
                        double range_y_scale,
                        double theta_max_upper_bound,
                        double agent_buffer) {
                return JPS_CollisionFreeSpeedModelV3AgentParameters{
                    intoJPS_Point(position),
                    journey_id,
                    stage_id,
                    strength_neighbor_repulsion,
                    range_neighbor_repulsion,
                    strength_geometry_repulsion,
                    range_geometry_repulsion,
                    range_x_scale,
                    range_y_scale,
                    theta_max_upper_bound,
                    agent_buffer,
                    time_gap,
                    desired_speed,
                    radius};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("time_gap"),
            py::arg("desired_speed"),
            py::arg("radius"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"),
            py::arg("range_x_scale"),
            py::arg("range_y_scale"),
            py::arg("theta_max_upper_bound"),
            py::arg("agent_buffer"))
        .def("__repr__", [](const JPS_CollisionFreeSpeedModelV3AgentParameters& p) {
            return fmt::format(
                "position: {}, journey_id: {}, stage_id: {}, "
                "time_gap: {}, desired_speed: {}, radius: {}, "
                "strength_neighbor_repulsion: {}, range_neighbor_repulsion: {}, "
                "strength_geometry_repulsion: {}, range_geometry_repulsion: {}, "
                "range_x_scale: {}, range_y_scale: {}, theta_max_upper_bound: {}, "
                "agent_buffer: {}",
                intoTuple(p.position),
                p.journeyId,
                p.stageId,
                p.time_gap,
                p.v0,
                p.radius,
                p.strengthNeighborRepulsion,
                p.rangeNeighborRepulsion,
                p.strengthGeometryRepulsion,
                p.rangeGeometryRepulsion,
                p.rangeXScale,
                p.rangeYScale,
                p.thetaMaxUpperBound,
                p.agentBuffer);
        });
    py::class_<JPS_CollisionFreeSpeedModelV3Builder_Wrapper>(m, "CollisionFreeSpeedModelV3Builder")
        .def(py::init([]() {
            return std::make_unique<JPS_CollisionFreeSpeedModelV3Builder_Wrapper>(
                JPS_CollisionFreeSpeedModelV3Builder_Create());
        }))
        .def("build", [](JPS_CollisionFreeSpeedModelV3Builder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_CollisionFreeSpeedModelV3Builder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_CollisionFreeSpeedModelV3State_Wrapper>(m, "CollisionFreeSpeedModelV3State")
        .def_property(
            "time_gap",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetTimeGap(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double time_gap) {
                JPS_CollisionFreeSpeedModelV3State_SetTimeGap(w.handle, time_gap);
            })
        .def_property(
            "desired_speed",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetV0(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double desiredSpeed) {
                JPS_CollisionFreeSpeedModelV3State_SetV0(w.handle, desiredSpeed);
            })
        .def_property(
            "radius",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetRadius(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double radius) {
                JPS_CollisionFreeSpeedModelV3State_SetRadius(w.handle, radius);
            })
        .def_property(
            "strength_neighbor_repulsion",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetStrengthNeighborRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double strengthNeighborRepulsion) {
                JPS_CollisionFreeSpeedModelV3State_SetStrengthNeighborRepulsion(
                    w.handle, strengthNeighborRepulsion);
            })
        .def_property(
            "range_neighbor_repulsion",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetRangeNeighborRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double rangeNeighborRepulsion) {
                JPS_CollisionFreeSpeedModelV3State_SetRangeNeighborRepulsion(
                    w.handle, rangeNeighborRepulsion);
            })
        .def_property(
            "strength_geometry_repulsion",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetStrengthGeometryRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double strengthGeometryRepulsion) {
                JPS_CollisionFreeSpeedModelV3State_SetStrengthGeometryRepulsion(
                    w.handle, strengthGeometryRepulsion);
            })
        .def_property(
            "range_geometry_repulsion",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetRangeGeometryRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double rangeGeometryRepulsion) {
                JPS_CollisionFreeSpeedModelV3State_SetRangeGeometryRepulsion(
                    w.handle, rangeGeometryRepulsion);
            })
        .def_property(
            "range_x_scale",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetRangeXScale(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double range_x_scale) {
                JPS_CollisionFreeSpeedModelV3State_SetRangeXScale(
                    w.handle, range_x_scale);
            })
        .def_property(
            "range_y_scale",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetRangeYScale(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double range_y_scale) {
                JPS_CollisionFreeSpeedModelV3State_SetRangeYScale(
                    w.handle, range_y_scale);
            })
        .def_property(
            "theta_max_upper_bound",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetThetaMaxUpperBound(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double theta_max_upper_bound) {
                JPS_CollisionFreeSpeedModelV3State_SetThetaMaxUpperBound(
                    w.handle, theta_max_upper_bound);
            })
        .def_property(
            "agent_buffer",
            [](const JPS_CollisionFreeSpeedModelV3State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV3State_GetAgentBuffer(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV3State_Wrapper& w, double agent_buffer) {
                JPS_CollisionFreeSpeedModelV3State_SetAgentBuffer(
                    w.handle, agent_buffer);
            });
}
