// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Simulation.hpp"

#include "GenericAgent.hpp"
#include "Geometry/Geometry2D.hpp"
#include "Journey.hpp"
#include "OperationalModel.hpp"
#include "Polygon.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/attr.h>
#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace py = pybind11;

void init_simulation(py::module_& m)
{
    py::class_<Simulation>(m, "Simulation")
        .def(
            // The model is moved out of the Python object into Simulation. After this constructor
            // returns, the Python model object passed here is disowned/invalid and must not be
            // reused.
            py::init(
                [](std::unique_ptr<OperationalModel> model, Geometry2D geometry, double dT) {
                    if(!model) {
                        throw std::invalid_argument("model must not be None");
                    }
                    return std::make_unique<Simulation>(
                        std::move(model), std::make_unique<Geometry2D>(geometry), dT);
                }),
            py::kw_only(),
            py::arg("model"),
            py::arg("geometry"),
            py::arg("dt"))
        .def(
            "add_waypoint_stage",
            [](Simulation& sim, std::tuple<double, double> position, double distance) {
                return sim.AddStage(WaypointDescription{intoPoint(position), distance}).getID();
            })
        .def(
            "add_queue_stage",
            [](Simulation& sim, const std::vector<std::tuple<double, double>>& positions) {
                return sim.AddStage(NotifiableQueueDescription{intoPoints(positions)}).getID();
            })
        .def(
            "add_waiting_set_stage",
            [](Simulation& sim, const std::vector<std::tuple<double, double>>& positions) {
                return sim.AddStage(NotifiableWaitingSetDescription{intoPoints(positions)}).getID();
            })
        .def(
            "add_exit_stage",
            [](Simulation& sim, const std::vector<std::tuple<double, double>>& polygon) {
                return sim.AddStage(ExitDescription{Polygon{intoPoints(polygon)}}).getID();
            })
        .def(
            "add_direct_steering_stage",
            [](Simulation& sim) { return sim.AddStage(DirectSteeringDescription{}).getID(); })
        .def(
            "add_journey",
            [](Simulation& sim, std::map<uint64_t, TransitionDescription>& journey) {
                auto native_journey = std::map<BaseStage::ID, TransitionDescription>{};
                for(const auto& [stage_id, desc] : journey) {
                    native_journey.emplace(stage_id, desc);
                }

                return sim.AddJourney(native_journey).getID();
            })
        .def(
            "add_agent",
            [](Simulation& sim,
               uint64_t journeyId,
               uint64_t stageId,
               GenericAgent::ModelState state) {
                return sim
                    .AddAgent(GenericAgent(
                        GenericAgent::ID::Invalid, journeyId, stageId, std::move(state)))
                    .getID();
            },
            py::kw_only(),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("state"))
        .def(
            "mark_agent_for_removal",
            [](Simulation& sim, uint64_t id) { sim.MarkAgentForRemoval(id); })
        .def(
            "removed_agents",
            [](const Simulation& sim) {
                auto removed_agent_ids = sim.RemovedAgents();
                auto agent_ids = std::vector<GenericAgent::ID::underlying_type>();
                agent_ids.reserve(removed_agent_ids.size());
                for(auto agent_id : removed_agent_ids) {
                    agent_ids.emplace_back(agent_id.getID());
                }
                return agent_ids;
            })
        .def("iterate", [](Simulation& sim) { sim.Iterate(); })
        .def(
            "switch_agent_journey",
            [](Simulation& sim, uint64_t agentId, uint64_t journeyId, uint64_t stageId) {
                sim.SwitchAgentJourney(agentId, journeyId, stageId);
            },
            py::kw_only(),
            py::arg("agent_id"),
            py::arg("journey_id"),
            py::arg("stage_id"))
        .def("agent_count", [](const Simulation& sim) { return sim.AgentCount(); })
        .def("elapsed_time", [](const Simulation& sim) { return sim.ElapsedTime(); })
        .def("delta_time", [](const Simulation& sim) { return sim.DT(); })
        .def("iteration_count", [](const Simulation& sim) { return sim.Iteration(); })
        .def(
            "agents",
            [](Simulation& sim) { return py::make_iterator(sim.Agents()); },
            py::keep_alive<0, 1>())
        .def(
            // TRANSIENT ONLY: the returned object wraps a raw reference into the
            // simulation's agent storage. It must not be stored across iterate();
            // callers (Python agent handles) resolve it freshly inside every
            // property access.
            "agent",
            [](Simulation& sim, uint64_t agentId) -> auto& { return sim.Agent(agentId); },
            py::arg("agent_id"),
            py::return_value_policy::reference)
        .def(
            "agents_in_range",
            [](Simulation& sim, std::tuple<double, double> pos, double distance) {
                auto agents_in_range = sim.AgentsInRange(intoPoint(pos), distance);
                auto agents = std::vector<uint64_t>();
                agents.reserve(agents_in_range.size());
                for(auto agent : agents_in_range) {
                    agents.emplace_back(agent.getID());
                }
                return agents;
            })
        .def(
            "agents_in_polygon",
            [](Simulation& sim, const std::vector<std::tuple<double, double>>& poly) {
                auto agents_in_range = sim.AgentsInPolygon(intoPoints(poly));
                auto agents = std::vector<uint64_t>();
                agents.reserve(agents_in_range.size());
                for(auto agent : agents_in_range) {
                    agents.emplace_back(agent.getID());
                }
                return agents;
            })
        .def("get_stage_proxy", [](Simulation& sim, uint64_t id) { return sim.Stage(id); })
        .def("set_tracing", [](Simulation& sim, bool status) { sim.SetTracing(status); })
        .def(
            "set_timer_log_level",
            [](Simulation& sim, size_t level) { sim.SetTimerLogLevel(level); })
        .def("get_geometry", [](Simulation& sim) { return sim.Geo(); })
        .def(
            "push_timer",
            [](Simulation& sim, const std::string& name, size_t probe_log_level) {
                sim.PushTimer(name, probe_log_level);
            })
        .def("pop_timer", [](Simulation& sim, const std::string& name) { sim.PopTimer(name); })
        .def(
            "get_duration",
            [](Simulation& sim, const std::string_view name) { return sim.GetTimerDuration(name); })
        .def("get_durations", [](Simulation& sim) { return sim.GetTimerDurations(); });
}
