// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_generalized_centrifugal_force_model(py::module_& m)
{
    py::class_<JPS_GeneralizedCentrifugalForceModelAgentParameters>(
        m, "GeneralizedCentrifugalForceModelAgentParameters")
        .def(
            py::init([](double speed,
                        std::tuple<double, double> e0,
                        std::tuple<double, double> position,
                        std::tuple<double, double> orientation,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        double mass,
                        double tau,
                        double v0,
                        double a_v,
                        double a_min,
                        double b_min,
                        double b_max) {
                return JPS_GeneralizedCentrifugalForceModelAgentParameters{
                    speed,
                    intoJPS_Point(e0),
                    intoJPS_Point(position),
                    intoJPS_Point(orientation),
                    journey_id,
                    stage_id,
                    mass,
                    tau,
                    v0,
                    a_v,
                    a_min,
                    b_min,
                    b_max};
            }),
            py::kw_only(),
            py::arg("speed"),
            py::arg("e0"),
            py::arg("position"),
            py::arg("orientation"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("mass"),
            py::arg("tau"),
            py::arg("v0"),
            py::arg("a_v"),
            py::arg("a_min"),
            py::arg("b_min"),
            py::arg("b_max"))
        .def("__repr__", [](const JPS_GeneralizedCentrifugalForceModelAgentParameters& p) {
            return fmt::format(
                "speed: {}, e0: {}, position: {}, orientation: {}, journey_id: {}, "
                "stage_id: {}, mass: {}, v0: {}, a_v: {}, a_min: {}, b_min: {}, b_max: {}",
                p.speed,
                intoTuple(p.e0),
                intoTuple(p.position),
                intoTuple(p.orientation),
                p.journeyId,
                p.stageId,
                p.mass,
                p.v0,
                p.a_v,
                p.a_min,
                p.b_min,
                p.b_max);
        });
    py::class_<JPS_GeneralizedCentrifugalForceModelBuilder_Wrapper>(
        m, "GeneralizedCentrifugalForceModelBuilder")
        .def(
            py::init([](double strengthNeighborRepulsion,
                        double strengthGeometryRepulsion,
                        double maxNeighborInteractionDistance,
                        double maxGeometryInteractionDistance,
                        double maxNeighborInterpolationDistance,
                        double maxGeometryInterpolationDistance,
                        double maxNeighborRepulsionForce,
                        double maxGeometryRepulsionForce) {
                return std::make_unique<JPS_GeneralizedCentrifugalForceModelBuilder_Wrapper>(
                    JPS_GeneralizedCentrifugalForceModelBuilder_Create(
                        strengthNeighborRepulsion,
                        strengthGeometryRepulsion,
                        maxNeighborInteractionDistance,
                        maxGeometryInteractionDistance,
                        maxNeighborInterpolationDistance,
                        maxGeometryInterpolationDistance,
                        maxNeighborRepulsionForce,
                        maxGeometryRepulsionForce));
            }),
            py::kw_only(),
            py::arg("strength_neighbor_repulsion"),
            py::arg("strength_geometry_repulsion"),
            py::arg("max_neighbor_interaction_distance"),
            py::arg("max_geometry_interaction_distance"),
            py::arg("max_neighbor_interpolation_distance"),
            py::arg("max_geometry_interpolation_distance"),
            py::arg("max_neighbor_repulsion_force"),
            py::arg("max_geometry_repulsion_force"))
        .def("build", [](JPS_GeneralizedCentrifugalForceModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_GeneralizedCentrifugalForceModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_GeneralizedCentrifugalForceModelState_Wrapper>(
        m, "GeneralizedCentrifugalForceModelState")
        .def_property(
            "speed",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetSpeed(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double speed) {
                JPS_GeneralizedCentrifugalForceModelState_SetSpeed(w.handle, speed);
            })
        .def_property(
            "e0",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return intoTuple(JPS_GeneralizedCentrifugalForceModelState_GetE0(w.handle));
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w,
               std::tuple<double, double> e0) {
                JPS_GeneralizedCentrifugalForceModelState_SetE0(w.handle, intoJPS_Point(e0));
            })
        .def_property(
            "mass",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetMass(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double mass) {
                JPS_GeneralizedCentrifugalForceModelState_SetMass(w.handle, mass);
            })
        .def_property(
            "tau",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetTau(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double tau) {
                JPS_GeneralizedCentrifugalForceModelState_SetTau(w.handle, tau);
            })
        .def_property(
            "v0",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetV0(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double v0) {
                JPS_GeneralizedCentrifugalForceModelState_SetV0(w.handle, v0);
            })
        .def_property(
            "a_v",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetAV(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double a_v) {
                JPS_GeneralizedCentrifugalForceModelState_SetAV(w.handle, a_v);
            })
        .def_property(
            "a_min",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetAMin(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double a_min) {
                JPS_GeneralizedCentrifugalForceModelState_SetAMin(w.handle, a_min);
            })
        .def_property(
            "b_min",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetBMin(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double b_min) {
                JPS_GeneralizedCentrifugalForceModelState_SetBMin(w.handle, b_min);
            })
        .def_property(
            "b_max",
            [](const JPS_GeneralizedCentrifugalForceModelState_Wrapper& w) {
                return JPS_GeneralizedCentrifugalForceModelState_GetBMax(w.handle);
            },
            [](JPS_GeneralizedCentrifugalForceModelState_Wrapper& w, double b_max) {
                JPS_GeneralizedCentrifugalForceModelState_SetBMax(w.handle, b_max);
            });
}
