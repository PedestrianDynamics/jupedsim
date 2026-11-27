// SPDX-License-Identifier: LGPL-3.0-or-later
#include "simulation.hpp"

#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry/geometry_builder.hpp"
#include "geometry/location.hpp"
#include "id_and_name.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "polygon.hpp"
#include "records.hpp"
#include "route.hpp"
#include "stage.hpp"
#include "stage_description.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <fmt/format.h>
#include <pybind11/attr.h>
#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace py = pybind11;

namespace
{
using Area = std::variant<GeometryBuilder::Polygon, std::vector<Point>>;

std::vector<Point> boundary_without_holes(Area area)
{
    if(auto* polygon = std::get_if<GeometryBuilder::Polygon>(&area)) {
        if(!polygon->holes.empty()) {
            throw py::value_error("Polygon must not have holes");
        }
        return std::move(polygon->boundary);
    }
    return std::get<std::vector<Point>>(std::move(area));
}

using Areas = std::vector<std::pair<Area, std::size_t>>;

std::vector<StageArea> stage_areas(Areas areas)
{
    std::vector<StageArea> result{};
    result.reserve(areas.size());
    for(auto& [area, region_id] : areas) {
        result.push_back({Polygon{boundary_without_holes(std::move(area))}, region_id});
    }
    return result;
}

using SpeedArray = py::array_t<double, py::array::c_style | py::array::forcecast>;

/// @p values as the flat, row-major speed field of region @p region_id. Throws unless its shape
/// is the region grid's (height, width): the floor field only checks the number of values, and
/// a transposed array has the same number and would be taken row for column.
std::vector<double>
flat_speed_field(const Simulation& sim, std::size_t region_id, const SpeedArray& values)
{
    const auto grid = sim.grid_info(region_id);
    if(values.ndim() != 2 || values.shape(0) != grid.height || values.shape(1) != grid.width) {
        throw py::value_error(
            fmt::format(
                "The speed field of region {} needs shape ({}, {}).",
                id_and_name(region_id, sim.geo()->region_name(region_id)),
                grid.height,
                grid.width));
    }
    return {values.data(), values.data() + values.size()};
}

/// @p location as a Python object that keeps @p simulation alive: it points into the geometry
/// the simulation owns.
py::object kept_alive_by(const Location& location, py::handle simulation)
{
    auto object = py::cast(location);
    py::detail::keep_alive_impl(object, simulation);
    return object;
}

/// One of the _...TransitionDescription objects bound in transition.cpp. The variant has no
/// type caster: its alternatives cannot be default constructed.
TransitionDescription transition_description(const py::handle& transition)
{
    if(py::isinstance<FixedTransitionDescription>(transition)) {
        return transition.cast<FixedTransitionDescription>();
    }
    if(py::isinstance<RoundRobinTransitionDescription>(transition)) {
        return transition.cast<RoundRobinTransitionDescription>();
    }
    return transition.cast<LeastTargetedTransitionDescription>();
}

std::optional<uint64_t> optional_id(const std::optional<BaseStage::ID>& id)
{
    return id ? std::optional{id->get_id()} : std::nullopt;
}
} // namespace

void init_simulation(py::module_& m)
{
    py::class_<Simulation>(m, "_Simulation")
        .def(
            // The model is moved out of the Python object into Simulation. After this constructor
            // returns, the Python model object passed here is disowned/invalid and must not be
            // reused. The geometry is shared instead: the Python object stays valid and can be
            // used for further simulations or routing engines.
            py::init([](std::unique_ptr<OperationalModel> model,
                        std::shared_ptr<Geometry> geometry,
                        double dt,
                        FloorfieldOptions floorfield_options) {
                if(!model) {
                    throw std::invalid_argument("model must not be None");
                }
                if(!geometry) {
                    throw std::invalid_argument("geometry must not be None");
                }
                return std::make_unique<Simulation>(
                    std::move(model), std::move(geometry), dt, floorfield_options);
            }),
            py::kw_only(),
            py::arg("model"),
            py::arg("geometry"),
            py::arg("dt"),
            py::arg("floorfield_options"))
        .def(
            "add_waypoint",
            [](Simulation& sim, Areas areas, std::optional<std::string> name) {
                return sim
                    .add_stage(WaypointDescription{stage_areas(std::move(areas))}, std::move(name))
                    .get_id();
            },
            py::arg("areas"),
            py::arg("name"))
        .def(
            "add_exit",
            [](Simulation& sim, Areas areas, std::optional<std::string> name) {
                return sim
                    .add_stage(ExitDescription{stage_areas(std::move(areas))}, std::move(name))
                    .get_id();
            },
            py::arg("areas"),
            py::arg("name"))
        .def(
            "target_name",
            [](const Simulation& sim, uint64_t target_id) { return sim.stage(target_id)->name(); },
            py::arg("target_id"))
        .def(
            "add_route",
            [](Simulation& sim,
               const std::vector<std::tuple<std::optional<uint64_t>, py::object>>& transitions,
               std::optional<std::string> name) {
                std::vector<RouteTransitionDescription> descriptions{};
                for(const auto& [source, transition] : transitions) {
                    descriptions.push_back(
                        {source ? std::optional<BaseStage::ID>{*source} : std::nullopt,
                         transition_description(transition)});
                }
                return sim.add_route(descriptions, std::move(name)).get_id();
            },
            py::arg("transitions"),
            py::arg("name"))
        .def(
            "route_ids",
            [](const Simulation& sim) {
                std::vector<uint64_t> ids{};
                for(const auto id : sim.route_ids()) {
                    ids.push_back(id.get_id());
                }
                return ids;
            })
        .def(
            "route_name",
            [](const Simulation& sim, uint64_t route_id) { return sim.route(route_id).name(); },
            py::arg("route_id"))
        .def(
            "add_agent",
            [](Simulation& sim,
               const Location& location,
               uint64_t route_id,
               std::optional<uint64_t> first_target_id,
               OperationalModelState state,
               std::optional<std::string> name) {
                return sim
                    .add_agent(
                        location,
                        route_id,
                        first_target_id ? std::optional<BaseStage::ID>{*first_target_id} :
                                          std::nullopt,
                        std::move(state),
                        std::move(name))
                    .get_id();
            },
            py::kw_only(),
            py::arg("location"),
            py::arg("route_id"),
            py::arg("first_target_id"),
            py::arg("state"),
            py::arg("name"))
        .def(
            "add_steered_agent",
            [](Simulation& sim,
               const Location& location,
               const Location& steer_to,
               OperationalModelState state,
               std::optional<std::string> name) {
                return sim.add_agent(location, steer_to, std::move(state), std::move(name))
                    .get_id();
            },
            py::kw_only(),
            py::arg("location"),
            py::arg("steer_to"),
            py::arg("state"),
            py::arg("name"))
        .def(
            "agent_name",
            [](const Simulation& sim, uint64_t agent_id) { return sim.agent_name(agent_id); },
            py::arg("agent_id"))
        .def(
            "agent_named",
            [](const Simulation& sim, const std::string& name) -> std::optional<uint64_t> {
                const auto id = sim.agent_named(name);
                return id ? std::optional{id->get_id()} : std::nullopt;
            },
            py::arg("name"))
        .def(
            "reroute",
            [](Simulation& sim,
               uint64_t agent_id,
               uint64_t route_id,
               std::optional<uint64_t> first_target_id) {
                sim.reroute(
                    agent_id,
                    route_id,
                    first_target_id ? std::optional<BaseStage::ID>{*first_target_id} :
                                      std::nullopt);
            },
            py::arg("agent_id"),
            py::arg("route_id"),
            py::arg("first_target_id"))
        .def(
            "reroute_within_route",
            [](Simulation& sim, uint64_t agent_id, uint64_t first_target_id) {
                sim.reroute(agent_id, BaseStage::ID{first_target_id});
            },
            py::arg("agent_id"),
            py::arg("first_target_id"))
        .def(
            "steer",
            [](Simulation& sim, uint64_t agent_id, const Location& steer_to) {
                sim.reroute(agent_id, steer_to);
            },
            py::arg("agent_id"),
            py::arg("steer_to"))
        .def(
            "mark_agent_for_removal",
            [](Simulation& sim, uint64_t id) { sim.mark_agent_for_removal(id); })
        .def(
            // (agent id, target id or location, time, location) per reached target, from index
            // @p since on.
            "reached_targets",
            [](py::object self, std::size_t since) {
                const auto& sim = self.cast<const Simulation&>();
                const auto& records = sim.reached_targets();
                py::list reached{};
                for(std::size_t i = since; i < records.size(); ++i) {
                    const auto& r = records[i];
                    const auto target = std::holds_alternative<BaseStage::ID>(r.target) ?
                                            py::cast(std::get<BaseStage::ID>(r.target).get_id()) :
                                            kept_alive_by(std::get<Location>(r.target), self);
                    reached.append(
                        py::make_tuple(
                            r.agent_id.get_id(), target, r.time, kept_alive_by(r.location, self)));
                }
                return reached;
            },
            py::arg("since") = 0)
        .def(
            // (agent id, exit id or None, time) per removal.
            "removed_agents",
            [](const Simulation& sim) {
                std::vector<std::tuple<uint64_t, std::optional<uint64_t>, double>> removals{};
                for(const auto& r : sim.removed_agents()) {
                    removals.emplace_back(r.agent_id.get_id(), optional_id(r.exit), r.time);
                }
                return removals;
            })
        .def("iterate", [](Simulation& sim) { sim.iterate(); })
        .def("close", [](Simulation& sim) { sim.close(); })
        .def("agent_count", [](const Simulation& sim) { return sim.agent_count(); })
        .def("elapsed_time", [](const Simulation& sim) { return sim.elapsed_time(); })
        .def("delta_time", [](const Simulation& sim) { return sim.dt(); })
        .def("iteration_count", [](const Simulation& sim) { return sim.iteration(); })
        .def(
            "agents",
            [](Simulation& sim) { return py::make_iterator(sim.agents()); },
            py::keep_alive<0, 1>())
        .def(
            "agent_ids",
            [](Simulation& sim) {
                const auto& agents = sim.agents();
                std::vector<uint64_t> ids;
                ids.reserve(agents.size());
                for(const auto& agent : agents) {
                    ids.push_back(agent.id.get_id());
                }
                return ids;
            })
        .def(
            // (ids, xyz, region ids) of all agents in id order, as copied arrays of shape (n,),
            // (n, 3) and (n,).
            "agent_locations",
            [](Simulation& sim) {
                const auto& agents = sim.agents();
                const auto count = static_cast<py::ssize_t>(agents.size());
                py::array_t<uint64_t> ids(count);
                py::array_t<double> xyz({count, py::ssize_t{3}});
                py::array_t<uint64_t> region_ids(count);
                auto id = ids.mutable_unchecked<1>();
                auto position = xyz.mutable_unchecked<2>();
                auto region_id = region_ids.mutable_unchecked<1>();
                py::ssize_t i = 0;
                for(const auto& agent : agents) {
                    const auto& location = agent.location;
                    id(i) = agent.id.get_id();
                    position(i, 0) = location.xy().x();
                    position(i, 1) = location.xy().y();
                    position(i, 2) = location.z();
                    region_id(i) = location.region();
                    ++i;
                }
                return py::make_tuple(ids, xyz, region_ids);
            })
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
        .def("target_ids", &Simulation::stage_ids)
        .def(
            "default_speed_fields",
            [](const Simulation& sim) { return sim.default_speed_fields(); })
        .def(
            "custom_speed_fields",
            [](const Simulation& sim, uint64_t target_id) {
                return sim.custom_speed_fields(target_id);
            },
            py::arg("target_id"))
        .def(
            "has_custom_speed_field",
            [](const Simulation& sim, uint64_t target_id, std::size_t region_id) {
                return sim.has_custom_speed_field(target_id, region_id);
            },
            py::arg("target_id"),
            py::arg("region_id"))
        .def(
            "set_default_speed_field",
            [](Simulation& sim, std::size_t region_id, const SpeedArray& values) {
                sim.set_default_speed_field(region_id, flat_speed_field(sim, region_id, values));
            },
            py::arg("region_id"),
            py::arg("values"))
        .def(
            "set_custom_speed_field",
            [](Simulation& sim,
               uint64_t target_id,
               std::size_t region_id,
               const SpeedArray& values) {
                sim.set_custom_speed_field(
                    target_id, region_id, flat_speed_field(sim, region_id, values));
            },
            py::arg("target_id"),
            py::arg("region_id"),
            py::arg("values"))
        .def(
            "clear_custom_speed_field",
            [](Simulation& sim, uint64_t target_id, std::size_t region_id) {
                sim.clear_custom_speed_field(target_id, region_id);
            },
            py::arg("target_id"),
            py::arg("region_id"))
        .def(
            "travel_times",
            [](Simulation& sim, uint64_t target_id) { return sim.travel_times(target_id); },
            py::arg("target_id"))
        .def(
            "travel_time",
            [](Simulation& sim, uint64_t target_id, const Location& from) {
                return sim.travel_time(target_id, from);
            },
            py::arg("target_id"),
            py::arg("location"))
        .def(
            "agents_in_range",
            [](Simulation& sim, const Location& location, double distance) {
                std::vector<uint64_t> ids{};
                for(const auto id : sim.agents_in_range(location, distance)) {
                    ids.push_back(id.get_id());
                }
                return ids;
            },
            py::arg("location"),
            py::arg("distance"))
        .def(
            "agents_in_polygon",
            [](Simulation& sim, Area polygon, std::size_t region_id) {
                std::vector<uint64_t> ids{};
                for(const auto id : sim.agents_in_polygon(
                        Polygon{boundary_without_holes(std::move(polygon))}, region_id)) {
                    ids.push_back(id.get_id());
                }
                return ids;
            },
            py::arg("polygon"),
            py::arg("region_id"))
        .def("set_tracing", [](Simulation& sim, bool status) { sim.set_tracing(status); })
        .def(
            "set_timer_log_level",
            [](Simulation& sim, size_t level) { sim.set_timer_log_level(level); })
        .def(
            "get_geometry",
            [](const Simulation& sim) { return std::const_pointer_cast<Geometry>(sim.geo()); })
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
