// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_warp_driver_model(py::module_& m)
{
    py::class_<JPS_WarpDriverModelAgentParameters>(m, "WarpDriverModelAgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        std::tuple<double, double> orientation,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        double radius,
                        double desired_speed,
                        double stuckTime,
                        double anchorX,
                        double anchorY,
                        double detourTime,
                        int detourSide) {
                return JPS_WarpDriverModelAgentParameters{
                    intoJPS_Point(position),
                    intoJPS_Point(orientation),
                    journey_id,
                    stage_id,
                    radius,
                    desired_speed,
                    stuckTime,
                    anchorX,
                    anchorY,
                    detourTime,
                    detourSide};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("orientation"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("radius"),
            py::arg("desired_speed"),
            py::arg("stuckTime") = 0.0,
            py::arg("anchorX") = 0.0,
            py::arg("anchorY") = 0.0,
            py::arg("detourTime") = 0.0,
            py::arg("detourSide") = 1)
        .def("__repr__", [](const JPS_WarpDriverModelAgentParameters& p) {
            return fmt::format(
                "position: {}, orientation: {}, journey_id: {}, stage_id: {}, "
                "radius: {}, v0: {}, stuckTime: {},"
                "anchorX: {}, anchor>: {}, detourTime: {}, detourSide: {}",
                intoTuple(p.position),
                intoTuple(p.orientation),
                p.journeyId,
                p.stageId,
                p.radius,
                p.v0,
                p.stuckTime,
                p.anchorX,
                p.anchorY,
                p.detourTime,
                p.detourSide);
        });
    py::class_<JPS_WarpDriverModelBuilder_Wrapper>(m, "WarpDriverModelBuilder")
        .def(
            py::init([](double timeHorizon,
                        double stepSize,
                        double sigma,
                        double timeUncertainty,
                        double velocityUncertaintyX,
                        double velocityUncertaintyY,
                        int numSamples,
                        uint64_t rngSeed) {
                return std::make_unique<JPS_WarpDriverModelBuilder_Wrapper>(
                    JPS_WarpDriverModelBuilder_Create(
                        timeHorizon,
                        stepSize,
                        sigma,
                        timeUncertainty,
                        velocityUncertaintyX,
                        velocityUncertaintyY,
                        numSamples,
                        rngSeed));
            }),
            py::kw_only(),
            py::arg("time_horizon") = 2.0,
            py::arg("step_size") = 0.5,
            py::arg("sigma") = 0.3,
            py::arg("time_uncertainty") = 0.5,
            py::arg("velocity_uncertainty_x") = 0.2,
            py::arg("velocity_uncertainty_y") = 0.2,
            py::arg("num_samples") = 20,
            py::arg("rng_seed") = 42)
        .def("build", [](JPS_WarpDriverModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_WarpDriverModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_WarpDriverModelState_Wrapper>(m, "WarpDriverModelState")
        .def_property(
            "radius",
            [](const JPS_WarpDriverModelState_Wrapper& w) {
                return JPS_WarpDriverModelState_GetRadius(w.handle);
            },
            [](JPS_WarpDriverModelState_Wrapper& w, double radius) {
                JPS_WarpDriverModelState_SetRadius(w.handle, radius);
            })
        .def_property(
            "desired_speed",
            [](const JPS_WarpDriverModelState_Wrapper& w) {
                return JPS_WarpDriverModelState_GetV0(w.handle);
            },
            [](JPS_WarpDriverModelState_Wrapper& w, double v0) {
                JPS_WarpDriverModelState_SetV0(w.handle, v0);
        //     })
        // .def_property(
        //     "stuck_time",
        //     [](const JPS_WarpDriverModelState_Wrapper& w) {
        //         return JPS_WarpDriverModelState_GetStuckTime(w.handle);
        //     },
        //     [](JPS_WarpDriverModelState_Wrapper& w, double stuckTime) {
        //         JPS_WarpDriverModelState_SetStuckTime(w.handle, stuckTime);
        //     })
        // .def_property(
        //     "anchor_x",
        //     [](const JPS_WarpDriverModelState_Wrapper& w) {
        //         return JPS_WarpDriverModelState_GetAnchorX(w.handle);
        //     },
        //     [](JPS_WarpDriverModelState_Wrapper& w, double anchorX) {
        //         JPS_WarpDriverModelState_SetAnchorX(w.handle, anchorX);
        //     })
        // .def_property(
        //     "anchor_y",
        //     [](const JPS_WarpDriverModelState_Wrapper& w) {
        //         return JPS_WarpDriverModelState_GetAnchorY(w.handle);
        //     },
        //     [](JPS_WarpDriverModelState_Wrapper& w, double anchorY) {
        //         JPS_WarpDriverModelState_SetAnchorY(w.handle, anchorY);
        //     })
        // .def_property(
        //     "detour_time",
        //     [](const JPS_WarpDriverModelState_Wrapper& w) {
        //         return JPS_WarpDriverModelState_GetDetourTime(w.handle);
        //     },
        //     [](JPS_WarpDriverModelState_Wrapper& w, double detourTime) {
        //         JPS_WarpDriverModelState_SetDetourTime(w.handle, detourTime);
        //     })
        // .def_property(
        //     "detour_side",
        //     [](const JPS_WarpDriverModelState_Wrapper& w) {
        //         return JPS_WarpDriverModelState_GetDetourSide(w.handle);
        //     },
        //     [](JPS_WarpDriverModelState_Wrapper& w, int detourSide) {
        //         JPS_WarpDriverModelState_SetDetourSide(w.handle, detourSide);
            });
}
