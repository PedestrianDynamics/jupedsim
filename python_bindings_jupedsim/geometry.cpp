// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Geometry.hpp"

#include "Geometry/Validation.hpp"
#include "GeometryBuilder.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/number_utils.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

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

/// The polygon a geometry was lifted from.
///
/// A mesh-built world has none, and it is not merely missing: the outline of a surface is a
/// bundle of loops per region, and saying which of them is "the" boundary and which are holes
/// is a question of its own.
const PolyWithHoles& polygon_of(const Geometry& geo)
{
    const auto* poly = geo.polygon();
    if(poly == nullptr) {
        throw SimulationError(
            "This geometry was built from a surface mesh, which has no polygon underneath.");
    }
    return *poly;
}
} // namespace

void init_geometry(py::module_& m)
{
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
            // Ray-cast (x, y) against the surface near z_hint -> Location, or None if no
            // sheet comes within tol of the hint.
            "get_location",
            [](const Geometry& geo, double x, double y, double z_hint, double tol) {
                return geo.get_location(x, y, z_hint, tol);
            },
            py::arg("x"),
            py::arg("y"),
            py::arg("z_hint") = 0.0,
            py::arg("tol") = ZHintTolerance,
            // The returned token points into this geometry.
            py::keep_alive<0, 1>())
        .def("region_count", &Geometry::region_count)
        .def("region_id_per_face", &Geometry::region_id_per_face)
        .def("vertices", &Geometry::vertices)
        .def("triangles", &Geometry::triangles)
        .def(
            "boundary",
            [](const Geometry& geo) {
                return intoTuples(ring_of(polygon_of(geo).outer_boundary()));
            })
        .def("holes", [](const Geometry& geo) {
            std::vector<std::vector<std::tuple<double, double>>> res{};
            for(const auto& hole : polygon_of(geo).holes()) {
                res.emplace_back(intoTuples(ring_of(hole)));
            }
            return res;
        });

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
        .def("build", [](GeometryBuilder& builder) {
            return std::make_unique<Geometry>(builder.Build());
        });
}
