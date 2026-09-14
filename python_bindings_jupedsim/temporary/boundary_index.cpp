// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/BoundaryIndex.hpp"
#include "Geometry/Geometry.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void init_boundary_index(py::module_& m)
{
    m.def("make_naive_boundary_index", [](const Geometry& geo) {
        return MakeNaiveBoundaryIndex(geo.mesh(), geo.region_split());
    });

    m.def("make_portal_boundary_index", [](const Geometry& geo) {
        return MakePortalBoundaryIndex(geo.mesh(), geo.region_split());
    });

    py::classh<BoundaryIndex>(m, "BoundaryIndex")
        .def("query", [](BoundaryIndex& index, const Location& loc, double d) {
            return index.Query(loc, d);
        });
}
