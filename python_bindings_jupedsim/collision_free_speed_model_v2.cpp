// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_collision_free_speed_model_v2(py::module_& m)
{
    py::class_<JPS_CollisionFreeSpeedModelV2AgentParameters>(
        m, "CollisionFreeSpeedModelV2AgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        double time_gap,
                        double v0,
                        double radius,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        double strengthNeighborRepulsion,
                        double rangeNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double rangeGeometryRepulsion) {
                return JPS_CollisionFreeSpeedModelV2AgentParameters{
                    intoJPS_Point(position),
                    journey_id,
                    stage_id,
                    time_gap,
                    v0,
                    radius,
                    strengthNeighborRepulsion,
                    rangeNeighborRepulsion,
                    strengthGeometryRepulsion,
                    rangeGeometryRepulsion};
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
            py::arg("strength_geometry_repulsion"),
            py::arg("range_geometry_repulsion"))
        .def("__repr__", [](const JPS_CollisionFreeSpeedModelV2AgentParameters& p) {
            return fmt::format(
                "position: {}, journey_id: {}, stage_id: {}, "
                "time_gap: {}, v0: {}, radius: {}",
                "strength_neighbor_repulsion: {}, range_neighbor_repulsion: {}"
                "strength_geometry_repulsion: {}, range_geometry_repulsion: {}",
                intoTuple(p.position),
                p.journeyId,
                p.stageId,
                p.time_gap,
                p.v0,
                p.radius,
                p.strengthNeighborRepulsion,
                p.rangeNeighborRepulsion,
                p.strengthGeometryRepulsion,
                p.rangeGeometryRepulsion);
        });
    py::class_<JPS_CollisionFreeSpeedModelV2Builder_Wrapper>(m, "CollisionFreeSpeedModelV2Builder")
        .def(py::init([]() {
            return std::make_unique<JPS_CollisionFreeSpeedModelV2Builder_Wrapper>(
                JPS_CollisionFreeSpeedModelV2Builder_Create());
        }))
        .def("build", [](JPS_CollisionFreeSpeedModelV2Builder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_CollisionFreeSpeedModelV2Builder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_CollisionFreeSpeedModelV2State_Wrapper>(m, "CollisionFreeSpeedModelV2State")
        .def_property(
            "time_gap",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetTimeGap(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double time_gap) {
                JPS_CollisionFreeSpeedModelV2State_SetTimeGap(w.handle, time_gap);
            })
        .def_property(
            "v0",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetV0(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double v0) {
                JPS_CollisionFreeSpeedModelV2State_SetV0(w.handle, v0);
            })
        .def_property(
            "radius",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetRadius(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double radius) {
                JPS_CollisionFreeSpeedModelV2State_SetRadius(w.handle, radius);
            })
        .def_property(
            "strength_neighbor_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetStrengthNeighborRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double strengthNeighborRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetStrengthNeighborRepulsion(
                    w.handle, strengthNeighborRepulsion);
            })
        .def_property(
            "range_neighbor_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetRangeNeighborRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double rangeNeighborRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetRangeNeighborRepulsion(
                    w.handle, rangeNeighborRepulsion);
            })
        .def_property(
            "strength_geometry_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetStrengthGeometryRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double strengthGeometryRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetStrengthGeometryRepulsion(
                    w.handle, strengthGeometryRepulsion);
            })
        .def_property(
            "range_geometry_repulsion",
            [](const JPS_CollisionFreeSpeedModelV2State_Wrapper& w) {
                return JPS_CollisionFreeSpeedModelV2State_GetRangeGeometryRepulsion(w.handle);
            },
            [](JPS_CollisionFreeSpeedModelV2State_Wrapper& w, double rangeGeometryRepulsion) {
                JPS_CollisionFreeSpeedModelV2State_SetRangeGeometryRepulsion(
                    w.handle, rangeGeometryRepulsion);
            });
}
