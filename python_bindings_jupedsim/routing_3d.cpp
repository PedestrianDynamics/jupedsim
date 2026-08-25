// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry2D.hpp"
#include "Geometry/Geometry3D.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/Validation.hpp"
#include "SimulationError.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "type_casters.hpp"

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/boost/graph/helpers.h>
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

    // smart_holder: a mesh-built Simulation takes ownership of the geometry, which means
    // handing a `unique_ptr` from Python into C++.
    py::class_<Geometry3D, py::smart_holder>(m, "Geometry3D")
        .def_static(
            // Read a surface mesh from an OBJ file (triangulating if needed).
            "from_obj",
            [](const std::string& obj_path) {
                // Use the PMP reader, not CGAL::IO::read_polygon_mesh as the PMP
                // reader repairs some common issues like triangle orientation.
                namespace PMP = CGAL::Polygon_mesh_processing;
                SurfaceMesh mesh{};
                if(!PMP::IO::read_polygon_mesh(obj_path, mesh) || mesh.is_empty()) {
                    throw SimulationError("Could not read a mesh from OBJ file '{}'", obj_path);
                }
                if(!CGAL::is_triangle_mesh(mesh)) {
                    PMP::triangulate_faces(mesh);
                }
                // The one door a mesh from outside comes through, so the one place to insist it
                // is a surface people can walk on -- and to turn it right side up if it is not.
                NormaliseAndValidateMesh(mesh);
                return std::make_unique<Geometry3D>(std::move(mesh));
            },
            py::arg("obj_path"))
        .def_static(
            // Lift a Geometry2D to a flat surface at z=0.
            "from_2d",
            [](const Geometry2D& geometry) {
                return std::make_unique<Geometry3D>(geometry.Polygon());
            },
            py::arg("geometry"))
        .def("is_valid_location", &Geometry3D::is_valid_location)
        .def(
            // Ray-cast (x, y) against the surface near z_hint -> Location, or None if no
            // sheet comes within tol of the hint.
            "get_location",
            [](const Geometry3D& geo, double x, double y, double z_hint, double tol) {
                return geo.get_location(x, y, z_hint, tol);
            },
            py::arg("x"),
            py::arg("y"),
            py::arg("z_hint") = 0.0,
            py::arg("tol") = ZHintTolerance,
            // The returned token points into this geometry.
            py::keep_alive<0, 1>())
        .def("region_count", &Geometry3D::region_count)
        .def("region_id_per_face", &Geometry3D::region_id_per_face)
        .def("vertices", &Geometry3D::vertices)
        .def("triangles", &Geometry3D::triangles);

    py::class_<RoutingEngine3D>(m, "RoutingEngine3D")
        .def("is_valid_location", &RoutingEngine3D::IsValidLocation)
        .def("get_shortest_path", &RoutingEngine3D::GetShortestPath)
        .def("get_orientation", &RoutingEngine3D::GetOrientation)
        .def("wall_clearance", &RoutingEngine3D::WallClearance);

    py::class_<SurfaceMeshShortestPathRoutingEngine, RoutingEngine3D>(
        m, "SurfaceMeshShortestPathRoutingEngine")
        // The engine borrows the geometry; keep_alive ties the Python-side
        // Geometry3D's lifetime to the engine so the borrow can't dangle.
        .def(
            py::init([](const Geometry3D& geometry) {
                return std::make_unique<SurfaceMeshShortestPathRoutingEngine>(geometry);
            }),
            py::arg("geometry"),
            py::keep_alive<1, 2>());
}
