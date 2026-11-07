// SPDX-License-Identifier: LGPL-3.0-or-later
#include "simulation.hpp"

#include "conversion.hpp"
#include "floorfield_routing_engine.hpp"
#include "generic_agent.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "geometry/walkable_surface.hpp"
#include "kernel.hpp"
#include "operational_model.hpp"
#include "operational_model_state.hpp"
#include "polygon.hpp"
#include "route.hpp"
#include "rule.hpp"
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
#include <map>
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
using Area = std::variant<WalkableSurface::Polygon, std::vector<Point>>;

std::vector<Point> boundary_without_holes(Area area)
{
    if(auto* polygon = std::get_if<WalkableSurface::Polygon>(&area)) {
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
                region_id,
                grid.height,
                grid.width));
    }
    return {values.data(), values.data() + values.size()};
}
} // namespace

void init_simulation(py::module_& m)
{
    py::class_<Simulation>(m, "_Simulation")
        .def(
            // The model is moved out of the Python object into Simulation. After this constructor
            // returns, the Python model object passed here is disowned/invalid and must not be
            // reused. The same goes for the geometry.
            py::init([](std::unique_ptr<OperationalModel> model,
                        std::unique_ptr<Geometry> geometry,
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
            "create_waypoint_stage",
            [](Simulation& sim, Areas areas) {
                return sim.add_stage(WaypointDescription{stage_areas(std::move(areas))}).get_id();
            },
            py::arg("areas"))
        .def(
            "create_exit_stage",
            [](Simulation& sim, Areas areas) {
                return sim.add_stage(ExitDescription{stage_areas(std::move(areas))}).get_id();
            },
            py::arg("areas"))
        .def(
            "create_direct_steering_stage",
            [](Simulation& sim) { return sim.add_stage(DirectSteeringDescription{}).get_id(); })
        .def(
            "add_route",
            [](Simulation& sim, const std::map<uint64_t, PyRule>& route) {
                auto native_route = std::map<BaseStage::ID, RuleDescription>{};
                for(const auto& [stage_id, rule] : route) {
                    native_route.emplace(stage_id, rule.description);
                }
                return sim.add_route(native_route).get_id();
            },
            py::arg("route"))
        .def(
            "add_agent",
            [](Simulation& sim,
               uint64_t route_id,
               uint64_t stage_id,
               std::tuple<double, double> position,
               OperationalModelState state,
               std::size_t region_id) {
                return sim
                    .add_agent(
                        route_id, stage_id, into_point(position), std::move(state), region_id)
                    .get_id();
            },
            py::kw_only(),
            py::arg("route_id"),
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
                using Record = std::tuple<
                    GenericAgent::ID::UnderlyingType,
                    std::optional<BaseStage::ID::UnderlyingType>,
                    double>;
                std::vector<Record> records;
                for(const auto& record : sim.removed_agents()) {
                    std::optional<BaseStage::ID::UnderlyingType> exit;
                    if(record.exit) {
                        exit = record.exit->get_id();
                    }
                    records.emplace_back(record.id.get_id(), exit, record.time);
                }
                return records;
            })
        .def("iterate", [](Simulation& sim) { sim.iterate(); })
        .def("close", [](Simulation& sim) { sim.close(); })
        .def(
            "switch_agent_route",
            [](Simulation& sim, uint64_t agent_id, uint64_t route_id, uint64_t stage_id) {
                sim.switch_agent_route(agent_id, route_id, stage_id);
            },
            py::kw_only(),
            py::arg("agent_id"),
            py::arg("route_id"),
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
        .def("stage_ids", &Simulation::stage_ids)
        .def(
            "default_speed_fields",
            [](const Simulation& sim) { return sim.default_speed_fields(); })
        .def(
            "custom_speed_fields",
            [](const Simulation& sim, uint64_t stage_id) {
                return sim.custom_speed_fields(stage_id);
            },
            py::arg("stage_id"))
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
               uint64_t stage_id,
               std::size_t region_id,
               const SpeedArray& values) {
                sim.set_custom_speed_field(
                    stage_id, region_id, flat_speed_field(sim, region_id, values));
            },
            py::arg("stage_id"),
            py::arg("region_id"),
            py::arg("values"))
        .def(
            "clear_custom_speed_field",
            [](Simulation& sim, uint64_t stage_id, std::size_t region_id) {
                sim.clear_custom_speed_field(stage_id, region_id);
            },
            py::arg("stage_id"),
            py::arg("region_id"))
        .def(
            "travel_times",
            [](Simulation& sim, uint64_t stage_id) { return sim.travel_times(stage_id); },
            py::arg("stage_id"))
        .def(
            "travel_time",
            [](Simulation& sim, uint64_t stage_id, const Location& where) {
                return sim.travel_time(stage_id, where);
            },
            py::arg("stage_id"),
            py::arg("where"))
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
            [](Simulation& sim, Area polygon) {
                auto agents_in_range =
                    sim.agents_in_polygon(boundary_without_holes(std::move(polygon)));
                auto agents = std::vector<uint64_t>();
                agents.reserve(agents_in_range.size());
                for(auto agent : agents_in_range) {
                    agents.emplace_back(agent.get_id());
                }
                return agents;
            })
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
