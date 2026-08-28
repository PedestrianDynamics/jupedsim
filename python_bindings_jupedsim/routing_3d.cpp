// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/Validation.hpp"
#include "SimulationError.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "type_casters.hpp"

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <memory>
#include <string>
#include <utility>

namespace py = pybind11;

void init_routing_3d(py::module_& m)
{
    // A place on the surface, handed out by the simulation and passed back to it. Read-only and
    // deliberately narrow: no region ids, no face handles -- the coordinates are all a caller
    // can do anything with, everything else is the geometry's business.
    py::class_<Location>(m, "Location")
        .def_property_readonly("x", [](const Location& l) { return l.xy().x; })
        .def_property_readonly("y", [](const Location& l) { return l.xy().y; })
        .def_property_readonly("z", &Location::z)
        .def("__repr__", [](const Location& l) {
            return fmt::format("Location({}, {}, {})", l.xy().x, l.xy().y, l.z());
        });

    py::class_<RoutingEngine3D>(m, "RoutingEngine3D")
        .def("is_valid_location", &RoutingEngine3D::IsValidLocation)
        .def("get_shortest_path", &RoutingEngine3D::GetShortestPath)
        .def("get_orientation", &RoutingEngine3D::GetOrientation)
        .def("wall_clearance", &RoutingEngine3D::WallClearance);

    py::class_<SurfaceMeshShortestPathRoutingEngine, RoutingEngine3D>(
        m, "SurfaceMeshShortestPathRoutingEngine")
        // The engine borrows the geometry; keep_alive ties the Python-side
        // Geometry's lifetime to the engine so the borrow can't dangle.
        .def(
            py::init([](const Geometry& geometry) {
                return std::make_unique<SurfaceMeshShortestPathRoutingEngine>(geometry);
            }),
            py::arg("geometry"),
            py::keep_alive<1, 2>());
}
