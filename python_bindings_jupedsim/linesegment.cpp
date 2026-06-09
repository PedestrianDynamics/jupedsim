#include "LineSegment.hpp"

#include "conversion.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;
// Type caster: According to https://pybind11.readthedocs.io/en/stable/advanced/cast/custom.html
// which coincidentally also shows a conversion for Point2D. :)
namespace pybind11
{
namespace detail
{
template <>
struct type_caster<Point> {
    PYBIND11_TYPE_CASTER(Point, const_name("tuple[float, float]"));

    bool load(handle src, bool)
    {
        if(!isinstance<sequence>(src)) {
            return false;
        }
        auto seq = reinterpret_borrow<sequence>(src);
        if(seq.size() != 2) {
            return false;
        }
        value.x = seq[0].cast<double>();
        value.y = seq[1].cast<double>();
        return true;
    }

    static handle cast(const Point& src, return_value_policy, handle)
    {
        return make_tuple(src.x, src.y).release();
    }
};
} // namespace detail
} // namespace pybind11

void init_linesegment(py::module_& m)
{
    py::class_<LineSegment>(m, "LineSegment")
        .def(py::init<>())
        .def(py::init<Point, Point>(), py::kw_only(), py::arg("p1"), py::arg("p2"))
        .def_readwrite("p1", &LineSegment::p1)
        .def_readwrite("p2", &LineSegment::p2)
        .def("shortest_point", &LineSegment::ShortestPoint, py::arg("p"))
        .def("dist_to", &LineSegment::DistTo, py::arg("p"));
}