// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "wrapper.hpp"

#include <Unreachable.hpp>
#include <jupedsim/jupedsim.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_simulation(py::module_& m)
{
    py::class_<JPS_OperationalModel_Wrapper>(m, "OperationalModel");
    py::class_<JPS_Simulation_Wrapper>(m, "Simulation")
        .def(
            py::init(
                [](JPS_OperationalModel_Wrapper& model, JPS_Geometry_Wrapper& geometry, double dT) {
                    JPS_ErrorMessage errorMsg{};
                    auto result =
                        JPS_Simulation_Create(model.handle, geometry.handle, dT, &errorMsg);
                    if(result) {
                        return std::make_unique<JPS_Simulation_Wrapper>(result);
                    }
                    auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                    JPS_ErrorMessage_Free(errorMsg);
                    throw std::runtime_error{msg};
                }),
            py::kw_only(),
            py::arg("model"),
            py::arg("geometry"),
            py::arg("dt"))
        .def(
            "add_waypoint_stage",
            [](JPS_Simulation_Wrapper& w, std::tuple<double, double> position, double distance) {
                JPS_ErrorMessage errorMsg{};
                const auto result = JPS_Simulation_AddStageWaypoint(
                    w.handle, intoJPS_Point(position), distance, &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_queue_stage",
            [](JPS_Simulation_Wrapper& w,
               const std::vector<std::tuple<double, double>>& positions) {
                JPS_ErrorMessage errorMsg{};
                const auto jpsPointPositions = intoJPS_Point(positions);
                const auto result = JPS_Simulation_AddStageNotifiableQueue(
                    w.handle, jpsPointPositions.data(), jpsPointPositions.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_waiting_set_stage",
            [](JPS_Simulation_Wrapper& w,
               const std::vector<std::tuple<double, double>>& positions) {
                JPS_ErrorMessage errorMsg{};
                const auto jpsPointPositions = intoJPS_Point(positions);
                const auto result = JPS_Simulation_AddStageWaitingSet(
                    w.handle, jpsPointPositions.data(), jpsPointPositions.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_exit_stage",
            [](JPS_Simulation_Wrapper& w, const std::vector<std::tuple<double, double>>& polygon) {
                JPS_ErrorMessage errorMsg{};
                const auto jpsPointPoly = intoJPS_Point(polygon);
                const auto result = JPS_Simulation_AddStageExit(
                    w.handle, jpsPointPoly.data(), jpsPointPoly.size(), &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_direct_steering_stage",
            [](JPS_Simulation_Wrapper& w) {
                JPS_ErrorMessage errorMsg{};
                const auto result = JPS_Simulation_AddStageDirectSteering(w.handle, &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_journey",
            [](JPS_Simulation_Wrapper& simulation, JPS_JourneyDescription_Wrapper& journey) {
                JPS_ErrorMessage errorMsg{};
                const auto result =
                    JPS_Simulation_AddJourney(simulation.handle, journey.handle, &errorMsg);
                if(result != 0) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation,
               JPS_GeneralizedCentrifugalForceModelAgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_AddGeneralizedCentrifugalForceModelAgent(
                    simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation,
               JPS_CollisionFreeSpeedModelAgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_AddCollisionFreeSpeedModelAgent(
                    simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "add_agent",
            [](JPS_Simulation_Wrapper& simulation,
               JPS_CollisionFreeSpeedModelv2AgentParameters& parameters) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_AddCollisionFreeSpeedModelv2Agent(
                    simulation.handle, parameters, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "mark_agent_for_removal",
            [](JPS_Simulation_Wrapper& simulation, JPS_AgentId id) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_MarkAgentForRemoval(simulation.handle, id, &errorMsg);
                if(result) {
                    return result;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "removed_agents",
            [](const JPS_Simulation_Wrapper& simulation) {
                const JPS_AgentId* ids{};
                const auto count = JPS_Simulation_RemovedAgents(simulation.handle, &ids);
                return std::vector<JPS_AgentId>{ids, ids + count};
            })
        .def(
            "iterate",
            [](const JPS_Simulation_Wrapper& simulation) {
                JPS_ErrorMessage errorMsg{};
                bool iterate_ok = JPS_Simulation_Iterate(simulation.handle, &errorMsg);
                if(iterate_ok) {
                    return;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            })
        .def(
            "switch_agent_journey",
            [](const JPS_Simulation_Wrapper& w,
               JPS_AgentId agentId,
               JPS_JourneyId journeyId,
               JPS_StageId stageId) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_SwitchAgentJourney(
                    w.handle, agentId, journeyId, stageId, &errorMsg);
                if(result) {
                    return;
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            py::kw_only(),
            py::arg("agent_id"),
            py::arg("journey_id"),
            py::arg("stage_id"))
        .def(
            "agent_count",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_AgentCount(simulation.handle);
            })
        .def(
            "elapsed_time",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_ElapsedTime(simulation.handle);
            })
        .def(
            "delta_time",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_DeltaTime(simulation.handle);
            })
        .def(
            "iteration_count",
            [](JPS_Simulation_Wrapper& simulation) {
                return JPS_Simulation_IterationCount(simulation.handle);
            })
        .def(
            "agents",
            [](const JPS_Simulation_Wrapper& simulation) {
                return std::make_unique<JPS_AgentIterator_Wrapper>(
                    JPS_Simulation_AgentIterator(simulation.handle));
            })
        .def(
            "agent",
            [](const JPS_Simulation_Wrapper& simulation, JPS_AgentId agentId) {
                JPS_ErrorMessage errorMsg{};
                auto result = JPS_Simulation_GetAgent(simulation.handle, agentId, &errorMsg);
                if(result) {
                    return std::make_unique<JPS_Agent_Wrapper>(result);
                }
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            },
            py::arg("agent_id"))
        .def(
            "agents_in_range",
            [](JPS_Simulation_Wrapper& w, std::tuple<double, double> pos, double distance) {
                return std::make_unique<JPS_AgentIdIterator_Wrapper>(
                    JPS_Simulation_AgentsInRange(w.handle, intoJPS_Point(pos), distance));
            })
        .def(
            "agents_in_polygon",
            [](JPS_Simulation_Wrapper& w, const std::vector<std::tuple<double, double>>& poly) {
                const auto ppoly = intoJPS_Point(poly);
                return std::make_unique<JPS_AgentIdIterator_Wrapper>(
                    JPS_Simulation_AgentsInPolygon(w.handle, ppoly.data(), ppoly.size()));
            })
        .def(
            "get_stage_proxy",
            [](JPS_Simulation_Wrapper& w, JPS_StageId id)
                -> std::variant<
                    std::unique_ptr<JPS_WaypointProxy_Wrapper>,
                    std::unique_ptr<JPS_NotifiableQueueProxy_Wrapper>,
                    std::unique_ptr<JPS_WaitingSetProxy_Wrapper>,
                    std::unique_ptr<JPS_ExitProxy_Wrapper>,
                    std::unique_ptr<JPS_DirectSteeringProxy_Wrapper>> {
                const auto type = JPS_Simulation_GetStageType(w.handle, id);
                JPS_ErrorMessage errorMessage{};
                const auto raise = [](JPS_ErrorMessage err) {
                    const auto msg = std::string(JPS_ErrorMessage_GetMessage(err));
                    JPS_ErrorMessage_Free(err);
                    throw std::runtime_error{msg};
                };

                switch(type) {
                    case JPS_NotifiableQueueType: {
                        auto ptr = std::make_unique<JPS_NotifiableQueueProxy_Wrapper>(
                            JPS_Simulation_GetNotifiableQueueProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                    case JPS_WaitingSetType: {
                        auto ptr = std::make_unique<JPS_WaitingSetProxy_Wrapper>(
                            JPS_Simulation_GetWaitingSetProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                    case JPS_WaypointType: {
                        auto ptr = std::make_unique<JPS_WaypointProxy_Wrapper>(
                            JPS_Simulation_GetWaypointProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                    case JPS_ExitType: {
                        auto ptr = std::make_unique<JPS_ExitProxy_Wrapper>(
                            JPS_Simulation_GetExitProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                    case JPS_DirectSteeringType: {
                        auto ptr = std::make_unique<JPS_DirectSteeringProxy_Wrapper>(
                            JPS_Simulation_GetDirectSteeringProxy(w.handle, id, &errorMessage));
                        if(!ptr) {
                            raise(errorMessage);
                        }
                        return ptr;
                    }
                }
                UNREACHABLE();
            })
        .def(
            "set_tracing",
            [](JPS_Simulation_Wrapper& w, bool status) {
                JPS_Simulation_SetTracing(w.handle, status);
            })
        .def(
            "get_last_trace",
            [](JPS_Simulation_Wrapper& w) { return JPS_Simulation_GetTrace(w.handle); })
        .def(
            "get_geometry",
            [](const JPS_Simulation_Wrapper& w) {
                return std::make_unique<JPS_Geometry_Wrapper>(JPS_Simulation_GetGeometry(w.handle));
            })
        .def("switch_geometry", [](JPS_Simulation_Wrapper& w, JPS_Geometry_Wrapper& geometry) {
            JPS_ErrorMessage errorMsg{};

            auto success =
                JPS_Simulation_SwitchGeometry(w.handle, geometry.handle, nullptr, &errorMsg);

            if(!success) {
                auto msg = std::string(JPS_ErrorMessage_GetMessage(errorMsg));
                JPS_ErrorMessage_Free(errorMsg);
                throw std::runtime_error{msg};
            }
            return success;
        });
}
