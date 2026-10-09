// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Simulation.hpp"

#include "GenericAgent.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Location.hpp"
#include "Journey.hpp"
#include "OperationalModel.hpp"
#include "Polygon.hpp"
#include "Stage.hpp"
#include "StageDescription.hpp"
#include "conversion.hpp"
#include "transition.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/attr.h>
#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep
#include <pybind11/typing.h>

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
            // reused. The same goes for the geometry.
            py::init([](std::unique_ptr<OperationalModel> model,
                        std::unique_ptr<Geometry> geometry,
                        double dt) {
                if(!model) {
                    throw std::invalid_argument("model must not be None");
                }
                if(!geometry) {
                    throw std::invalid_argument("geometry must not be None");
                }
                return std::make_unique<Simulation>(std::move(model), std::move(geometry), dt);
            }),
            py::kw_only(),
            py::arg("model"),
            py::arg("geometry"),
            py::arg("dt"))
        .def(
            "add_waypoint_stage",
            [](Simulation& sim,
               std::tuple<double, double> position,
               double distance,
               std::size_t region_id) {
                return sim.add_stage(WaypointDescription{into_point(position), distance, region_id})
                    .get_id();
            },
            py::arg("position"),
            py::arg("distance"),
            py::arg("region_id"))
        .def(
            "add_exit_stage",
            [](Simulation& sim,
               const std::vector<std::tuple<double, double>>& polygon,
               std::size_t region_id) {
                return sim.add_stage(ExitDescription{Polygon{into_points(polygon)}, region_id})
                    .get_id();
            },
            py::arg("polygon"),
            py::arg("region_id"))
        .def(
            "add_direct_steering_stage",
            [](Simulation& sim) { return sim.add_stage(DirectSteeringDescription{}).get_id(); })
        .def(
            "add_journey",
            [](Simulation& sim, const std::map<uint64_t, PyTransition>& journey) {
                auto native_journey = std::map<BaseStage::ID, TransitionDescription>{};
                for(const auto& [stage_id, transition] : journey) {
                    native_journey.emplace(stage_id, transition.description);
                }
                return sim.add_journey(native_journey).get_id();
            },
            py::arg("journey"))
        .def(
            "add_agent",
            [](Simulation& sim,
               uint64_t journey_id,
               uint64_t stage_id,
               std::tuple<double, double> position,
               OperationalModelState state,
               std::size_t region_id) {
                return sim
                    .add_agent(
                        journey_id, stage_id, into_point(position), std::move(state), region_id)
                    .get_id();
            },
            py::kw_only(),
            py::arg("journey_id"),
            py::arg("stage_id"),
            py::arg("position"),
            py::arg("state"),
            py::arg("region_id"))
        .def(
            "mark_agent_for_removal",
            [](Simulation& sim, uint64_t id) { sim.mark_agent_for_removal(id); })
        .def(
            "removed_agents",
            [](const Simulation& sim) {
                auto removed_agent_ids = sim.removed_agents();
                auto agent_ids = std::vector<GenericAgent::ID::UnderlyingType>();
                agent_ids.reserve(removed_agent_ids.size());
                for(auto agent_id : removed_agent_ids) {
                    agent_ids.emplace_back(agent_id.get_id());
                }
                return agent_ids;
            })
        .def("iterate", [](Simulation& sim) { sim.iterate(); })
        .def(
            "switch_agent_journey",
            [](Simulation& sim, uint64_t agent_id, uint64_t journey_id, uint64_t stage_id) {
                sim.switch_agent_journey(agent_id, journey_id, stage_id);
            },
            py::kw_only(),
            py::arg("agent_id"),
            py::arg("journey_id"),
            py::arg("stage_id"))
        .def("agent_count", [](const Simulation& sim) { return sim.agent_count(); })
        .def("elapsed_time", [](const Simulation& sim) { return sim.elapsed_time(); })
        .def("delta_time", [](const Simulation& sim) { return sim.dt(); })
        .def("iteration_count", [](const Simulation& sim) { return sim.iteration(); })
        .def(
            "agents",
            [](Simulation& sim) { return py::make_iterator(sim.agents()); },
            py::keep_alive<0, 1>())
        .def(
            // TRANSIENT ONLY: the returned object wraps a raw reference into the
            // simulation's agent storage. It must not be stored across iterate();
            // callers (Python agent handles) resolve it freshly inside every
            // property access.
            "agent",
            [](Simulation& sim, uint64_t agent_id) -> auto& { return sim.agent(agent_id); },
            py::arg("agent_id"),
            py::return_value_policy::reference,
            py::keep_alive<0, 1>())
        .def(
            "get_location",
            &Simulation::get_location,
            py::arg("x"),
            py::arg("y"),
            py::arg("region_id"),
            // The returned token points into geometry the simulation owns.
            py::keep_alive<0, 1>())
        .def(
            "set_agent_target",
            [](Simulation& sim, uint64_t agent_id, std::tuple<double, double> target) {
                sim.set_agent_target(agent_id, into_point(target));
            },
            py::arg("agent_id"),
            py::arg("target"))
        .def(
            "set_agent_target",
            [](Simulation& sim, uint64_t agent_id, const Location& target) {
                sim.set_agent_target(agent_id, target);
            },
            py::arg("agent_id"),
            py::arg("target"))
        .def(
            "agents_in_range",
            [](Simulation& sim, std::tuple<double, double> pos, double distance) {
                auto agents_in_range = sim.agents_in_range(into_point(pos), distance);
                auto agents = std::vector<uint64_t>();
                agents.reserve(agents_in_range.size());
                for(auto agent : agents_in_range) {
                    agents.emplace_back(agent.get_id());
                }
                return agents;
            })
        .def(
            "agents_in_polygon",
            [](Simulation& sim, const std::vector<std::tuple<double, double>>& poly) {
                auto agents_in_range = sim.agents_in_polygon(into_points(poly));
                auto agents = std::vector<uint64_t>();
                agents.reserve(agents_in_range.size());
                for(auto agent : agents_in_range) {
                    agents.emplace_back(agent.get_id());
                }
                return agents;
            })
        .def(
            "get_stage",
            [](py::object self,
               uint64_t id) -> py::typing::Union<WaypointProxy, ExitProxy, DirectSteeringProxy> {
                // A stage object refers to its simulation. The keep-alive is set by hand:
                // pybind11 3.1.0 runs py::keep_alive<0, 1>() also after a failed argument
                // conversion and then crashes instead of raising TypeError.
                auto stage = py::cast(self.cast<Simulation&>().stage(id));
                py::detail::keep_alive_impl(stage, self);
                return stage;
            },
            py::arg("stage_id"))
        .def("set_tracing", [](Simulation& sim, bool status) { sim.set_tracing(status); })
        .def(
            "set_timer_log_level",
            [](Simulation& sim, size_t level) { sim.set_timer_log_level(level); })
        .def(
            "get_geometry",
            [](const Simulation& sim) -> const Geometry& { return sim.geo(); },
            // Borrowed from the simulation, which keeps owning it.
            py::return_value_policy::reference_internal)
        .def(
            "push_timer",
            [](Simulation& sim, const std::string& name, size_t probe_log_level) {
                sim.push_timer(name, probe_log_level);
            })
        .def("pop_timer", [](Simulation& sim, const std::string& name) { sim.pop_timer(name); })
        .def(
            "get_duration",
            [](Simulation& sim, const std::string_view name) {
                return sim.get_timer_duration(name);
            })
        .def("get_durations", [](Simulation& sim) { return sim.get_timer_durations(); });
}
