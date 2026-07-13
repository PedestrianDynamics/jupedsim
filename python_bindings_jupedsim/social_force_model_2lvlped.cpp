// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <jupedsim/jupedsim.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_social_force_model_2lvlped(py::module_& m)
{
    py::class_<JPS_SocialForceModel2LvlPedAgentParameters>(m, "SocialForceModel2LvlPedAgentParameters")
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
                        double legScale,
                        double forceDistance,
                        double obstacleForceDistance,
                        double legForceDistance,
                        double radius,
                        double bodyForce,
                        double friction) {
                return JPS_SocialForceModel2LvlPedAgentParameters{
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
                    legScale,
                    forceDistance,
                    obstacleForceDistance,
                    legForceDistance,
                    radius,
                    bodyForce,
                    friction};
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
            py::arg("leg_scale"),
            py::arg("force_distance"),
            py::arg("obstacle_force_distance"),
            py::arg("leg_force_distance"),
            py::arg("radius"),
            py::arg("body_force"),
            py::arg("friction"))
        .def("__repr__", [](const JPS_SocialForceModel2LvlPedAgentParameters& p) {
            return fmt::format(
                "position: {}, orientation: {}, journey_id: {}, stage_id: {}, "
                "velocity: {}, ground_support_position: {}, ground_support_velocity: {}, "
                "height: {}, desiredSpeed: {}, reactionTime: {}, "
                "lambdaU: {}, lambdaB: {}, balanceSpeed: {}, damping: {}, "
                "agentScale: {}, obstacleScale: {}, legScale: {}, forceDistance: {}, "
                "obstacleForceDistance: {}, legForceDistance: {}, radius: {}, "
                "bodyForce: {}, friction: {}",
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
                p.legScale,
                p.forceDistance,
                p.obstacleForceDistance,
                p.legForceDistance,
                p.radius,
                p.bodyForce,
                p.friction);
        });
    py::class_<JPS_SocialForceModel2LvlPedBuilder_Wrapper>(m, "SocialForceModel2LvlPedBuilder")
        .def(
            py::init([]() {
                return std::make_unique<JPS_SocialForceModel2LvlPedBuilder_Wrapper>(
                    JPS_SocialForceModel2LvlPedBuilder_Create());
            }))
        .def("build", [](JPS_SocialForceModel2LvlPedBuilder_Wrapper& w) {
            JPS_ErrorMessage errorMsg{};
            auto result = JPS_SocialForceModel2LvlPedBuilder_Build(w.handle, &errorMsg);
            if(result) {
                return std::make_unique<JPS_OperationalModel_Wrapper>(result);
            }
            auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
            JPS_ErrorMessage_Free(errorMsg);
            throw std::runtime_error{msg};
        });
    py::class_<JPS_SocialForceModel2LvlPedState_Wrapper>(m, "SocialForceModel2LvlPedState")
        .def_property(
            "velocity",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return intoTuple(JPS_SocialForceModel2LvlPedState_GetVelocity(w.handle));
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, std::tuple<double, double> velocity) {
                JPS_SocialForceModel2LvlPedState_SetVelocity(w.handle, intoJPS_Point(velocity));
            })
        .def_property(
            "ground_support_position",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return intoTuple(JPS_SocialForceModel2LvlPedState_GetGroundSupportPosition(w.handle));
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, std::tuple<double, double> ground_support_position) {
                JPS_SocialForceModel2LvlPedState_SetGroundSupportPosition(
                    w.handle, intoJPS_Point(ground_support_position));
            })
        .def_property(
            "ground_support_velocity",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return intoTuple(JPS_SocialForceModel2LvlPedState_GetGroundSupportVelocity(w.handle));
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, std::tuple<double, double> ground_support_velocity) {
                JPS_SocialForceModel2LvlPedState_SetGroundSupportVelocity(
                    w.handle, intoJPS_Point(ground_support_velocity));
            })
        .def_property(
            "height",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetHeight(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double height) {
                JPS_SocialForceModel2LvlPedState_SetHeight(w.handle, height);
            })
        .def_property(
            "desired_speed",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetDesiredSpeed(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double desiredSpeed) {
                JPS_SocialForceModel2LvlPedState_SetDesiredSpeed(w.handle, desiredSpeed);
            })
        .def_property(
            "reaction_time",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetReactionTime(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double reactionTime) {
                JPS_SocialForceModel2LvlPedState_SetReactionTime(w.handle, reactionTime);
            })
        .def_property(
            "lambda_u",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetLambdaU(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double lambdaU) {
                JPS_SocialForceModel2LvlPedState_SetLambdaU(w.handle, lambdaU);
            })
        .def_property(
            "lambda_b",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetLambdaB(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double lambdaB) {
                JPS_SocialForceModel2LvlPedState_SetLambdaB(w.handle, lambdaB);
            })
        .def_property(
            "balance_speed",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetBalanceSpeed(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double balanceSpeed) {
                JPS_SocialForceModel2LvlPedState_SetBalanceSpeed(w.handle, balanceSpeed);
            })
        .def_property(
            "damping",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetDamping(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double damping) {
                JPS_SocialForceModel2LvlPedState_SetDamping(w.handle, damping);
            })
        .def_property(
            "agent_scale",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetAgentScale(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double agentScale) {
                JPS_SocialForceModel2LvlPedState_SetAgentScale(w.handle, agentScale);
            })
        .def_property(
            "obstacle_scale",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetObstacleScale(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double obstacleScale) {
                JPS_SocialForceModel2LvlPedState_SetObstacleScale(w.handle, obstacleScale);
            })
        .def_property(
            "leg_scale",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetLegScale(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double legScale) {
                JPS_SocialForceModel2LvlPedState_SetLegScale(w.handle, legScale);
            })
        .def_property(
            "force_distance",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetForceDistance(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double forceDistance) {
                JPS_SocialForceModel2LvlPedState_SetForceDistance(w.handle, forceDistance);
            })
        .def_property(
            "obstacle_force_distance",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetObstacleForceDistance(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double obstacleForceDistance) {
                JPS_SocialForceModel2LvlPedState_SetObstacleForceDistance(w.handle, obstacleForceDistance);
            })
        .def_property(
            "leg_force_distance",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetLegForceDistance(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double legForceDistance) {
                JPS_SocialForceModel2LvlPedState_SetLegForceDistance(w.handle, legForceDistance);
            })
        .def_property(
            "radius",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetRadius(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double radius) {
                JPS_SocialForceModel2LvlPedState_SetRadius(w.handle, radius);
            })
        .def_property(
            "body_force",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetBodyForce(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double bodyForce) {
                JPS_SocialForceModel2LvlPedState_SetBodyForce(w.handle, bodyForce);
            })
        .def_property(
            "friction",
            [](const JPS_SocialForceModel2LvlPedState_Wrapper& w) {
                return JPS_SocialForceModel2LvlPedState_GetFriction(w.handle);
            },
            [](JPS_SocialForceModel2LvlPedState_Wrapper& w, double friction) {
                JPS_SocialForceModel2LvlPedState_SetFriction(w.handle, friction);
            });
}
