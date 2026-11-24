// SPDX-License-Identifier: LGPL-3.0-or-later
#include "geometry/geometry.hpp"

#include "cfg_cgal.hpp"
#include "conversion.hpp"
#include "polygon.hpp"
#include "region.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace py = pybind11;

void init_polygon2d(py::module_& m)
{
    py::class_<epick::PolyWithHoles> polygon2d(m, "Polygon2D");
    polygon2d.doc() = clean_doc(R"(
        A polygon with holes in the (x, y) plane, e.g. the walkable area of one region.
    )");
    polygon2d.def(
        "as_wkt",
        &as_wkt,
        clean_doc("The polygon as WKT; every coordinate reads back to the exact same value.")
            .c_str());
}

void init_geometry(py::module_& m)
{
    // smart_holder: a Simulation shares ownership of its geometry with Python.
    py::class_<Geometry, py::smart_holder>(m, "Geometry")
        .def("is_valid_location", &Geometry::is_valid_location)
        .def(
            "region",
            [](const std::shared_ptr<Geometry>& geometry, const py::object& key) {
                if(py::isinstance<py::bool_>(key) ||
                   !(py::isinstance<py::int_>(key) || py::isinstance<py::str>(key))) {
                    throw py::type_error(
                        py::str("key must be a region id or name, got {}")
                            .format(py::type::of(key).attr("__name__")));
                }
                if(py::isinstance<py::int_>(key)) {
                    const auto id = key.cast<long long>();
                    if(id < 0 || static_cast<std::size_t>(id) >= geometry->region_count()) {
                        throw py::key_error(py::str("no region with id {}").format(id));
                    }
                    return region_of(geometry, static_cast<std::size_t>(id));
                }
                const auto name = key.cast<std::string>();
                for(std::size_t id = 0; id < geometry->region_count(); ++id) {
                    if(geometry->region_name(id) == name) {
                        return region_of(geometry, id);
                    }
                }
                throw py::key_error(py::str("no region named {!r}").format(name));
            },
            py::arg("key"),
            clean_doc(R"(
                Look a region up by id or by name.

                Raises:
                    KeyError: If no region has this id or name.
                    TypeError: If ``key`` is neither an int nor a str.
            )")
                .c_str())
        .def(
            "regions",
            [](const std::shared_ptr<Geometry>& geometry) {
                std::vector<RegionHandle> regions;
                regions.reserve(geometry->region_count());
                for(std::size_t id = 0; id < geometry->region_count(); ++id) {
                    regions.push_back(region_of(geometry, id));
                }
                return regions;
            },
            clean_doc("All regions, in ascending order of their ids.").c_str())
        .def("region_count", &Geometry::region_count)
        .def("region_id_per_face", &Geometry::region_id_per_face)
        .def("vertices", &Geometry::vertices)
        .def("triangles", &Geometry::triangles);
}
