// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_social_force_model_IPP(py::module_& m)
{
    py::class_<JPS_SocialForceModelIPPAgentParameters>(m, "SocialForceModelIPPAgentParameters")
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
                return JPS_SocialForceModelIPPAgentParameters{
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
            py::arg("desired_speed"),
            py::arg("reaction_time"),
            py::arg("agent_scale"),
            py::arg("obstacle_scale"),
            py::arg("force_distance"),
            py::arg("radius"))
        .def("__repr__", [](const JPS_SocialForceModelIPPAgentParameters& p) {
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
    py::class_<JPS_SocialForceModelIPPBuilder_Wrapper>(m, "SocialForceModelIPPBuilder")
        .def(
            py::init([](double bodyForce, double friction) {
                return std::make_unique<JPS_SocialForceModelIPPBuilder_Wrapper>(
                    JPS_SocialForceModelIPPBuilder_Create(bodyForce, friction));
            }),
            py::kw_only(),
            py::arg("body_force"),
            py::arg("friction"))
        .def("build", [](JPS_SocialForceModelIPPBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_SocialForceModelIPPBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_SocialForceModelIPPState_Wrapper>(m, "SocialForceModelIPPState")
        .def_property(
            "velocity",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return intoTuple(JPS_SocialForceModelIPPState_GetVelocity(w.handle));
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, std::tuple<double, double> velocity) {
                JPS_SocialForceModelIPPState_SetVelocity(w.handle, intoJPS_Point(velocity));
            })
        .def_property(
            "mass",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetMass(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double mass) {
                JPS_SocialForceModelIPPState_SetMass(w.handle, mass);
            })
        .def_property(
            "desired_speed",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetDesiredSpeed(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double desiredSpeed) {
                JPS_SocialForceModelIPPState_SetDesiredSpeed(w.handle, desiredSpeed);
            })
        .def_property(
            "reaction_time",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetReactionTime(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double reactionTime) {
                JPS_SocialForceModelIPPState_SetReactionTime(w.handle, reactionTime);
            })
        .def_property(
            "agent_scale",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetAgentScale(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double agentScale) {
                JPS_SocialForceModelIPPState_SetAgentScale(w.handle, agentScale);
            })
        .def_property(
            "obstacle_scale",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetObstacleScale(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double obstacleScale) {
                JPS_SocialForceModelIPPState_SetObstacleScale(w.handle, obstacleScale);
            })
        .def_property(
            "force_distance",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetForceDistance(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double forceDistance) {
                JPS_SocialForceModelIPPState_SetForceDistance(w.handle, forceDistance);
            })
        .def_property(
            "radius",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetRadius(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double radius) {
                JPS_SocialForceModelIPPState_SetRadius(w.handle, radius);
            });
}
