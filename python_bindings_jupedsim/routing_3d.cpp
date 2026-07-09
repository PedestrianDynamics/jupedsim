// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SimulationError.hpp"
#include "SurfaceMeshShortestPathRoutingEngine.hpp"
#include "type_casters.hpp"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/boost/graph/IO/polygon_mesh_io.h>
#include <CGAL/boost/graph/helpers.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <memory>
#include <string>
#include <utility>

namespace py = pybind11;

void init_routing_3d(py::module_& m)
{
    py::class_<RoutingEngine3D>(m, "RoutingEngine3D")
        .def("is_valid_location", &RoutingEngine3D::IsValidLocation)
        .def("get_shortest_path", &RoutingEngine3D::GetShortestPath)
        .def("get_orientation", &RoutingEngine3D::GetOrientation);

    py::class_<SurfaceMeshShortestPathRoutingEngine, RoutingEngine3D>(
        m, "SurfaceMeshShortestPathRoutingEngine")
        .def(py::init([](const std::string& path) {
            SurfaceMesh mesh{};
            if(!CGAL::IO::read_polygon_mesh(std::string(path), mesh) || mesh.is_empty()) {
                throw SimulationError("Could not read a mesh from OBJ file '{}'", path);
            }

            // Triangulate if not a triangle mesh
            if(!CGAL::is_triangle_mesh(mesh)) {
                CGAL::Polygon_mesh_processing::triangulate_faces(mesh);
            }

            return std::make_unique<SurfaceMeshShortestPathRoutingEngine>(std::move(mesh));
        }));
}
