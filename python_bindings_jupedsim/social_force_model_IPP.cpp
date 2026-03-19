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
                        std::tuple<double, double> ground_support_position,
                        std::tuple<double, double> ground_support_velocity,
                        double height,
                        double desiredSpeed,
                        double reactionTime,
                        double lambdaU,
                        double lambdaB,
                        double balanceSpeed,
                        double damping,
                        double agentScale,
                        double obstacleScale,
                        double forceDistance,
                        double obstacleForceDistance,
                        double legForceDistance,
                        double radius) {
                return JPS_SocialForceModelIPPAgentParameters{
                    intoJPS_Point(position),
                    intoJPS_Point(orientation),
                    journey_id,
                    stage_id,
                    intoJPS_Point(velocity),
                    intoJPS_Point(ground_support_position),
                    intoJPS_Point(ground_support_velocity),
                    height,
                    desiredSpeed,
                    reactionTime,
                    lambdaU,
                    lambdaB,
                    balanceSpeed,
                    damping,
                    agentScale,
                    obstacleScale,
                    forceDistance,
                    obstacleForceDistance,
                    legForceDistance,
                    radius};
            }),
            py::kw_only(),
            py::arg("position"),
            py::arg("orientation"),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("velocity"),
            py::arg("ground_support_position"),
            py::arg("ground_support_velocity"),
            py::arg("height"),
            py::arg("desired_speed"),
            py::arg("reaction_time"),
            py::arg("lambda_u"),
            py::arg("lambda_b"),
            py::arg("balance_speed"),
            py::arg("damping"),
            py::arg("agent_scale"),
            py::arg("obstacle_scale"),
            py::arg("force_distance"),
            py::arg("obstacle_force_distance"),
            py::arg("leg_force_distance"),
            py::arg("radius"))
        .def("__repr__", [](const JPS_SocialForceModelIPPAgentParameters& p) {
            return fmt::format(
                "position: {}, orientation: {}, journey_id: {}, stage_id: {}, "
                "velocity: {}, ground_support_position: {}, ground_support_velocity: {}, "
                "height: {}, desiredSpeed: {}, reactionTime: {}, "
                "lambdaU: {}, lambdaB: {}, balanceSpeed: {}, damping: {}, "
                "agentScale: {}, obstacleScale: {}, forceDistance: {}, "
                "obstacleForceDistance: {}, legForceDistance: {}, radius: {}",
                intoTuple(p.position),
                intoTuple(p.orientation),
                p.journeyId,
                p.stageId,
                intoTuple(p.velocity),
                intoTuple(p.ground_support_position),
                intoTuple(p.ground_support_velocity),
                p.height,
                p.desiredSpeed,
                p.reactionTime,
                p.lambdaU,
                p.lambdaB,
                p.balanceSpeed,
                p.damping,
                p.agentScale,
                p.obstacleScale,
                p.forceDistance,
                p.obstacleForceDistance,
                p.legForceDistance,
                p.radius);
        });
    py::class_<JPS_SocialForceModelIPPBuilder_Wrapper>(m, "SocialForceModelIPPBuilder")
        .def(
            py::init([]() {
                return std::make_unique<JPS_SocialForceModelIPPBuilder_Wrapper>(
                    JPS_SocialForceModelIPPBuilder_Create());
            }))
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
            "ground_support_position",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return intoTuple(JPS_SocialForceModelIPPState_GetGroundSupportPosition(w.handle));
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, std::tuple<double, double> ground_support_position) {
                JPS_SocialForceModelIPPState_SetGroundSupportPosition(
                    w.handle, intoJPS_Point(ground_support_position));
            })
        .def_property(
            "ground_support_velocity",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return intoTuple(JPS_SocialForceModelIPPState_GetGroundSupportVelocity(w.handle));
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, std::tuple<double, double> ground_support_velocity) {
                JPS_SocialForceModelIPPState_SetGroundSupportVelocity(
                    w.handle, intoJPS_Point(ground_support_velocity));
            })
        .def_property(
            "height",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetHeight(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double height) {
                JPS_SocialForceModelIPPState_SetHeight(w.handle, height);
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
            "lambda_u",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetLambdaU(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double lambdaU) {
                JPS_SocialForceModelIPPState_SetLambdaU(w.handle, lambdaU);
            })
        .def_property(
            "lambda_b",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetLambdaB(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double lambdaB) {
                JPS_SocialForceModelIPPState_SetLambdaB(w.handle, lambdaB);
            })
        .def_property(
            "balance_speed",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetBalanceSpeed(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double balanceSpeed) {
                JPS_SocialForceModelIPPState_SetBalanceSpeed(w.handle, balanceSpeed);
            })
        .def_property(
            "damping",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetDamping(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double damping) {
                JPS_SocialForceModelIPPState_SetDamping(w.handle, damping);
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
            "obstacle_force_distance",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetObstacleForceDistance(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double obstacleForceDistance) {
                JPS_SocialForceModelIPPState_SetObstacleForceDistance(w.handle, obstacleForceDistance);
            })
        .def_property(
            "leg_force_distance",
            [](const JPS_SocialForceModelIPPState_Wrapper& w) {
                return JPS_SocialForceModelIPPState_GetLegForceDistance(w.handle);
            },
            [](JPS_SocialForceModelIPPState_Wrapper& w, double legForceDistance) {
                JPS_SocialForceModelIPPState_SetLegForceDistance(w.handle, legForceDistance);
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
