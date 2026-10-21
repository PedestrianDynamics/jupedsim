// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "floorfield_routing_engine.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "geometry/validation.hpp"
#include "simulation_error.hpp"
#include "surface_mesh_shortest_path_routing_engine.hpp"
#include "type_casters.hpp"

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <memory>
#include <string>
#include <utility>

namespace py = pybind11;

void init_routing(py::module_& m)
{
    const FloorfieldOptions floorfield_defaults{};
    py::class_<FloorfieldOptions> floorfield_options(m, "FloorfieldOptions");
    floorfield_options.doc() = clean_doc(R"(
        Parameters of the static floor fields used to route agents to their stage areas.

        Args:
            cell_size: Edge length of a floor field grid cell [m]. Smaller cells give
                smoother routes at the cost of memory and setup time.
            wall_influence_radius: Distance from walls at which the speed field is reduced to avoid 
            routing agents to close to walls. The reduction scales linearly from the influence radius to the wall.
    )");
    floorfield_options
        .def(
            py::init([](double cell_size, double wall_influence_radius) {
                return FloorfieldOptions{cell_size, wall_influence_radius};
            }),
            py::kw_only(),
            py::arg("cell_size") = floorfield_defaults.cell_size,
            py::arg("wall_influence_radius") = floorfield_defaults.wall_influence_radius)
        .def_readwrite("cell_size", &FloorfieldOptions::cell_size)
        .def_readwrite("wall_influence_radius", &FloorfieldOptions::wall_influence_radius)
        .def("__repr__", [](const FloorfieldOptions& self) {
            return py::str("FloorfieldOptions(cell_size={!r}, wall_influence_radius={!r})")
                .format(self.cell_size, self.wall_influence_radius);
        });

    py::class_<Location> location(m, "Location");
    location.doc() = clean_doc(R"(
        A point on the walkable surface, together with the region it lies in.

        Locations cannot be created directly, get them from the simulation:

        .. code:: python

            sim.get_location(x, y, region_id=upper_floor)
            sim.agent(agent_id).location

        A location is read-only and stays valid as long as its simulation exists.
        It does not move with an agent: read
        :attr:`~jupedsim.Agent.location` again to get the agent's current
        location.
    )");
    location
        .def_property_readonly(
            "x",
            [](const Location& l) { return l.xy().x; },
            clean_doc("x coordinate in metres.").c_str())
        .def_property_readonly(
            "y",
            [](const Location& l) { return l.xy().y; },
            clean_doc("y coordinate in metres.").c_str())
        .def_property_readonly(
            "z", &Location::z, clean_doc("Height of the surface here, in metres.").c_str())
        .def_property_readonly(
            "region_id", &Location::region, clean_doc("Region this location lies in.").c_str())
        .def("__repr__", [](const Location& l) {
            // Python float formatting (3.0, not fmt's 3), as the former Python wrapper printed.
            return py::str("Location({!r}, {!r}, {!r})").format(l.xy().x, l.xy().y, l.z());
        });

    py::class_<SurfaceMeshShortestPathRoutingEngine>(m, "SurfaceMeshShortestPathRoutingEngine")
        // The engine borrows the geometry; keep_alive ties the Python-side
        // Geometry's lifetime to the engine so the borrow can't dangle.
        .def(
            py::init([](const Geometry& geometry) {
                return std::make_unique<SurfaceMeshShortestPathRoutingEngine>(geometry);
            }),
            py::arg("geometry"),
            py::keep_alive<1, 2>())
        .def("is_valid_location", &SurfaceMeshShortestPathRoutingEngine::is_valid_location)
        .def("get_shortest_path", &SurfaceMeshShortestPathRoutingEngine::get_shortest_path)
        .def("get_orientation", &SurfaceMeshShortestPathRoutingEngine::get_orientation)
        .def("wall_clearance", &SurfaceMeshShortestPathRoutingEngine::wall_clearance);
}
