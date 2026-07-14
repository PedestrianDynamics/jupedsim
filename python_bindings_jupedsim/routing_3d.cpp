// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"
#include "Geometry/Geometry3D.hpp"
#include "SimulationError.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "type_casters.hpp"

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/boost/graph/helpers.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <memory>
#include <string>
#include <utility>

namespace py = pybind11;

void init_routing_3d(py::module_& m)
{
    py::class_<Geometry3D>(m, "Geometry3D")
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
                return std::make_unique<Geometry3D>(std::move(mesh));
            },
            py::arg("obj_path"))
        .def_static(
            // Lift a 2D CollisionGeometry to a flat surface at z=0.
            "from_2d",
            [](const CollisionGeometry& geometry) {
                return std::make_unique<Geometry3D>(geometry.Polygon());
            },
            py::arg("geometry"))
        .def("is_valid_location", &Geometry3D::is_valid_location)
        .def("region_count", &Geometry3D::region_count)
        .def("region_id_per_face", &Geometry3D::region_id_per_face)
        .def("vertices", &Geometry3D::vertices)
        .def("triangles", &Geometry3D::triangles);

    py::class_<RoutingEngine3D>(m, "RoutingEngine3D")
        .def("is_valid_location", &RoutingEngine3D::IsValidLocation)
        .def("get_shortest_path", &RoutingEngine3D::GetShortestPath)
        .def("get_orientation", &RoutingEngine3D::GetOrientation);

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
