// SPDX-License-Identifier: LGPL-3.0-or-later
#include "region.hpp"

#include "conversion.hpp"
#include "geometry/geometry.hpp"
#include "id_and_name.hpp"
#include "simulation_error.hpp"

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <cstddef>
#include <functional>

namespace py = pybind11;

static const Geometry& built_geometry(const RegionHandle& r)
{
    if(!r.scope->geometry) {
        throw SimulationError(
            "Region {} is not built yet; call build() first.", id_and_name(r.id, r.name));
    }
    return *r.scope->geometry;
}

void init_region(py::module_& m)
{
    py::class_<RegionHandle> region(m, "Region");
    region.doc() = clean_doc(R"(
        Handle to a region of a geometry: a floor, or a connector such as a staircase.

        Returned by :meth:`~jupedsim.GeometryBuilder.add_region`,
        :meth:`~jupedsim.GeometryBuilder.connect_regions`, :meth:`~jupedsim.Geometry.region`,
        :meth:`~jupedsim.Geometry.regions` and :attr:`~jupedsim.Location.region`; never created
        directly. Handles are equal if they refer to the same region.
    )");
    region
        .def_property_readonly(
            "id",
            [](const RegionHandle& r) { return r.id; },
            clean_doc("Numeric id of the region in its geometry.").c_str())
        .def_property_readonly(
            "name",
            [](const RegionHandle& r) { return r.name; },
            clean_doc("Name of the region or ``None``.").c_str())
        .def(
            "at",
            [](const RegionHandle& r, double x, double y) {
                return built_geometry(r).get_location(x, y, r.id);
            },
            py::arg("x"),
            py::arg("y"),
            // The location points into the geometry this handle keeps alive.
            py::keep_alive<0, 1>(),
            clean_doc(R"(
                The location ``(x, y)`` in this region.

                Raises:
                    SimulationError: If the geometry is not built yet, or ``(x, y)`` does not
                        lie in this region.
            )")
                .c_str())
        .def(
            "polygon",
            [](const RegionHandle& r) { return built_geometry(r).polygon(r.id); },
            clean_doc(R"(
                The 2D polygon of this region as given to the builder, projected onto
                ``(x, y)`` for connectors.

                Raises:
                    SimulationError: If the geometry is not built yet.
            )")
                .c_str())
        .def(
            "__eq__",
            [](const RegionHandle& a, const py::object& b) -> py::object {
                if(!py::isinstance<RegionHandle>(b)) {
                    return py::module_::import("builtins").attr("NotImplemented");
                }
                return py::bool_(a == b.cast<const RegionHandle&>());
            },
            py::arg("other").none(true))
        .def("__hash__", [](const RegionHandle& r) { return std::hash<std::size_t>{}(r.id); })
        .def("__repr__", [](const RegionHandle& r) {
            if(r.name) {
                return py::str("Region(id={}, name={!r})").format(r.id, *r.name);
            }
            return py::str("Region(id={})").format(r.id);
        });
}
