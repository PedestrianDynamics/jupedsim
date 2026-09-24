// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry.hpp"

#include "Geometry/Validation.hpp"
#include "GeometryBuilder.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SimulationError.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/number_utils.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <string>
#include <tuple>
#include <vector>

namespace py = pybind11;

namespace
{
std::vector<Point> ring_of(const Poly& ring)
{
    std::vector<Point> out{};
    out.reserve(ring.size());
    for(const auto& p : ring.container()) {
        out.emplace_back(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
    }
    return out;
}
} // namespace

void init_geometry(py::module_& m)
{
    py::class_<PolyWithHoles> polygon2d(m, "Polygon2D");
    polygon2d.doc() = R"(
        A polygon with holes in the (x, y) plane, e.g. the walkable area of one region.
    )";
    polygon2d
        .def(
            "boundary",
            [](const PolyWithHoles& p) { return intoTuples(ring_of(p.outer_boundary())); },
            "Outer boundary as a list of (x, y) points.")
        .def(
            "holes",
            [](const PolyWithHoles& p) {
                std::vector<std::vector<std::tuple<double, double>>> res{};
                for(const auto& hole : p.holes()) {
                    res.emplace_back(intoTuples(ring_of(hole)));
                }
                return res;
            },
            "Holes, each as a list of (x, y) points.")
        .def(
            "as_wkt",
            &as_wkt,
            "The polygon as WKT; every coordinate reads back to the exact same value.");

    // smart_holder: a Simulation shares ownership of its geometry with Python.
    py::class_<Geometry, py::smart_holder>(m, "Geometry")
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
                // The one door a mesh from outside comes through, so the one place to insist it
                // is a surface people can walk on -- and to turn it right side up if it is not.
                NormaliseAndValidateMesh(mesh);
                return std::make_unique<Geometry>(std::move(mesh));
            },
            py::arg("obj_path"))
        .def("is_valid_location", &Geometry::is_valid_location)
        .def(
            "get_location",
            &Geometry::get_location,
            py::arg("x"),
            py::arg("y"),
            py::arg("region_id") = 0,
            // The returned token points into this geometry.
            py::keep_alive<0, 1>())
        .def("region_count", &Geometry::region_count)
        .def("region_id_per_face", &Geometry::region_id_per_face)
        .def("vertices", &Geometry::vertices)
        .def("triangles", &Geometry::triangles)
        .def(
            "polygon",
            &Geometry::polygon,
            py::kw_only(),
            py::arg("region_id") = 0,
            "Polygon2D for the specified region ID");

    py::class_<GeometryBuilder>(m, "GeometryBuilder")
        .def(py::init<>())
        .def(
            "add_accessible_area",
            [](GeometryBuilder& builder, const std::vector<std::tuple<double, double>>& points) {
                builder.AddAccessibleArea(intoPoints(points));
            })
        .def(
            "exclude_from_accessible_area",
            [](GeometryBuilder& builder, const std::vector<std::tuple<double, double>>& points) {
                builder.ExcludeFromAccessibleArea(intoPoints(points));
            })
        .def("build", &GeometryBuilder::Build);
}
