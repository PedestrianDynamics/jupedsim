// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_social_force_model(py::module_& m)
{
    py::class_<JPS_SocialForceModelAgentParameters>(m, "SocialForceModelAgentParameters")
        .def(
            py::init([](std::tuple<double, double> position,
                        std::tuple<double, double> orientation,
                        JPS_JourneyId journey_id,
                        JPS_StageId stage_id,
                        std::tuple<double, double> velocity,
                        double mass,
                        double desiredSpeed,
                        double reactionTime,
                        double agentScale,
                        double obstacleScale,
                        double forceDistance,
                        double radius) {
                return JPS_SocialForceModelAgentParameters{
                    intoJPS_Point(position),
                    intoJPS_Point(orientation),
                    journey_id,
                    stage_id,
                    intoJPS_Point(velocity),
                    mass,
                    desiredSpeed,
                    reactionTime,
                    agentScale,
                    obstacleScale,
                    forceDistance,
                    radius};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("orientation"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("velocity"),
            py::arg("mass"),
            py::arg("desiredSpeed"),
            py::arg("reactionTime"),
            py::arg("agentScale"),
            py::arg("obstacleScale"),
            py::arg("forceDistance"),
            py::arg("radius"))
        .def("__repr__", [](const JPS_SocialForceModelAgentParameters& p) {
            return fmt::format(
                "position: {}, orientation: {}, journey_id: {}, stage_id: {},"
                "velocity: {}, mass: {}, desiredSpeed: {},"
                "reactionTime: {}, agentScale: {}, obstacleScale: {}, forceDistance: {}, radius: "
                "{}",
                intoTuple(p.position),
                intoTuple(p.orientation),
                p.journeyId,
                p.stageId,
                intoTuple(p.velocity),
                p.mass,
                p.desiredSpeed,
                p.reactionTime,
                p.agentScale,
                p.obstacleScale,
                p.forceDistance,
                p.radius);
        });
    py::class_<JPS_SocialForceModelBuilder_Wrapper>(m, "SocialForceModelBuilder")
        .def(
            py::init([](double bodyForce, double friction) {
                return std::make_unique<JPS_SocialForceModelBuilder_Wrapper>(
                    JPS_SocialForceModelBuilder_Create(bodyForce, friction));
            }),
            py::kw_only(),
            py::arg("bodyForce"),
            py::arg("friction"))
        .def("build", [](JPS_SocialForceModelBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_SocialForceModelBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_SocialForceModelState_Wrapper>(m, "SocialForceModelState")
        .def_property(
            "velocity",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return intoTuple(JPS_SocialForceModelState_GetVelocity(w.handle));
            },
            [](JPS_SocialForceModelState_Wrapper& w, std::tuple<double, double> velocity) {
                JPS_SocialForceModelState_SetVelocity(w.handle, intoJPS_Point(velocity));
            })
        .def_property(
            "mass",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetMass(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double mass) {
                JPS_SocialForceModelState_SetMass(w.handle, mass);
            })
        .def_property(
            "desiredSpeed",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetDesiredSpeed(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double desiredSpeed) {
                JPS_SocialForceModelState_SetDesiredSpeed(w.handle, desiredSpeed);
            })
        .def_property(
            "reactionTime",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetReactionTime(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double reactionTime) {
                JPS_SocialForceModelState_SetReactionTime(w.handle, reactionTime);
            })
        .def_property(
            "agentScale",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetAgentScale(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double agentScale) {
                JPS_SocialForceModelState_SetAgentScale(w.handle, agentScale);
            })
        .def_property(
            "obstacleScale",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetObstacleScale(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double obstacleScale) {
                JPS_SocialForceModelState_SetObstacleScale(w.handle, obstacleScale);
            })
        .def_property(
            "ForceDistance",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetForceDistance(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double forceDistance) {
                JPS_SocialForceModelState_SetForceDistance(w.handle, forceDistance);
            })
        .def_property(
            "radius",
            [](const JPS_SocialForceModelState_Wrapper& w) {
                return JPS_SocialForceModelState_GetRadius(w.handle);
            },
            [](JPS_SocialForceModelState_Wrapper& w, double radius) {
                JPS_SocialForceModelState_SetRadius(w.handle, radius);
            });
}
