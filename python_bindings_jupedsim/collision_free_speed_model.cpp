// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_collision_free_speed_model(py::module_& m)
{
    py::class_<JPS_CollisionFreeSpeedModelAgentParameters>(
        m, "CollisionFreeSpeedModelAgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        double time_gap,
                        double desired_speed,
                        double radius,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id) {
                return JPS_CollisionFreeSpeedModelAgentParameters{
                    intoJPS_Point(position), journey_id, stage_id, time_gap, desired_speed, radius};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("time_gap"),
            py::arg("desired_speed"),
            py::arg("radius"),
            py::arg("journey_id"),
            py::arg("stage_id"))
        .def("__repr__", [](const JPS_CollisionFreeSpeedModelAgentParameters& p) {
            return fmt::format(
                "position: {}, journey_id: {}, stage_id: {}, "
                "time_gap: {}, desired_speed: {}, radius: {}",
                intoTuple(p.position),
                p.journeyId,
                p.stageId,
                p.time_gap,
                p.v0,
                p.radius);
        });
    py::class_<JPS_CollisionFreeSpeedModelBuilder_Wrapper>(m, "CollisionFreeSpeedModelBuilder")
        .def(
            py::init([](double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double rangeGeometryRepulsion) {
                return std::make_unique<JPS_CollisionFreeSpeedModelBuilder_Wrapper>(
                    JPS_CollisionFreeSpeedModelBuilder_Create(
                        strengthNeighborRepulsion,
                        rangeNeighborRepulsion,
                        strengthGeometryRepulsion,
                        rangeGeometryRepulsion));
            }),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion"),
            py::arg("range_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"))
        .def("build", [](JPS_CollisionFreeSpeedModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_CollisionFreeSpeedModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_CollisionFreeSpeedModelState_Wrapper>(m, "CollisionFreeSpeedModelState")
        .def_property(
            "time_gap",
            [](const JPS_CollisionFreeSpeedModelState_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelState_GetTimeGap(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelState_Wrapper& w, double time_gap) {
                JPS_CollisionFreeSpeedModelState_SetTimeGap(w.handle, time_gap);
            })
        .def_property(
            "desired_speed",
            [](const JPS_CollisionFreeSpeedModelState_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelState_GetV0(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelState_Wrapper& w, double desiredSpeed) {
                JPS_CollisionFreeSpeedModelState_SetV0(w.handle, desiredSpeed);
            })
        .def_property(
            "radius",
            [](const JPS_CollisionFreeSpeedModelState_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelState_GetRadius(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelState_Wrapper& w, double radius) {
                JPS_CollisionFreeSpeedModelState_SetRadius(w.handle, radius);
            });
}
