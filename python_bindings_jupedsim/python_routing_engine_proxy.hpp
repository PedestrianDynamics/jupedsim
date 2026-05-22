// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "RoutingEngine.hpp"
#include "conversion.hpp"

#include <pybind11/pybind11.h>

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace py = pybind11;

// Adapts a Python object that implements the routing engine protocol
// to the C++ RoutingEngine interface.  Holds a reference to the Python
// object, so the Python side stays alive for as long as this proxy is alive.
class PythonRoutingEngineProxy : public RoutingEngine
{
    py::object _engine;
    // Caching all function pointers to avoid doing this in each call.
    // In addition it raises an error during construction rather than usage.
    py::function _name_fct;
    py::function _set_geometry_fct;
    py::function _compute_all_waypoints_fct;
    py::function _is_routable_fct;
    py::function _clone_fct;

public:
    explicit PythonRoutingEngineProxy(py::object engine)
        : _engine(std::move(engine))
        , _name_fct(_engine.attr("name"))
        , _set_geometry_fct(_engine.attr("set_geometry"))
        , _compute_all_waypoints_fct(_engine.attr("compute_all_waypoints"))
        , _is_routable_fct(_engine.attr("is_routable"))
        , _clone_fct(_engine.attr("clone"))
    {
    }

    std::string name() const override
    {
        py::gil_scoped_acquire lock{};
        return _name_fct().cast<std::string>();
    }

    void SetGeometry(const PolyWithHoles& poly) override
    {
        py::gil_scoped_acquire lock{};
        _set_geometry_fct(CollisionGeometry{poly});
    }

    std::vector<Point> ComputeAllWaypoints(Point from, Point destination) override
    {
        py::gil_scoped_acquire lock{};
        return intoPoints(_compute_all_waypoints_fct(intoTuple(from), intoTuple(destination))
                              .cast<std::vector<std::tuple<double, double>>>());
    }

    bool IsRoutable(Point p) const override
    {
        py::gil_scoped_acquire lock{};
        return _is_routable_fct(intoTuple(p)).cast<bool>();
    }

    std::unique_ptr<RoutingEngine> Clone() const override
    {
        py::gil_scoped_acquire lock{};
        return std::make_unique<PythonRoutingEngineProxy>(_clone_fct());
    }
};
