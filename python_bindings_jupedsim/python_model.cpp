// SPDX-License-Identifier: LGPL-3.0-or-later
#include "python_model.hpp"

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModels/CustomModel/CustomModelData.hpp"
#include "SimulationError.hpp"
#include "conversion.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdexcept>
#include <tuple>
#include <utility>

namespace py = pybind11;

GilSafePyObject::GilSafePyObject(py::object obj) : _obj(std::move(obj))
{
}

GilSafePyObject::GilSafePyObject(const GilSafePyObject& other)
{
    py::gil_scoped_acquire gil;
    _obj = other._obj; // share by reference (incref), do not clone
}

GilSafePyObject& GilSafePyObject::operator=(const GilSafePyObject& other)
{
    if(this == &other) {
        return *this;
    }

    py::gil_scoped_acquire gil;
    _obj = other._obj; // share by reference (incref), do not clone
    return *this;
}

GilSafePyObject& GilSafePyObject::operator=(GilSafePyObject&& other) noexcept
{
    if(this == &other) {
        return *this;
    }

    // Move-assignment decrefs the previously held object, so it must hold the GIL.
    py::gil_scoped_acquire gil;
    _obj = std::move(other._obj);
    return *this;
}

GilSafePyObject::~GilSafePyObject()
{
    py::gil_scoped_acquire gil;
    _obj = py::object();
}

const py::object& GilSafePyObject::Get() const
{
    return _obj;
}

py::object& GilSafePyObject::Get()
{
    return _obj;
}

void GilSafePyObject::Set(py::object obj)
{
    py::gil_scoped_acquire gil;
    _obj = std::move(obj);
}

PythonModel::PythonModel(py::object model) : _model(std::move(model))
{
    py::gil_scoped_acquire gil;
    if(!_model || _model.is_none()) {
        throw std::invalid_argument("_PythonModel requires a CustomOperationalModel instance");
    }
    if(!py::hasattr(_model, "_compute_next_state") ||
       !py::hasattr(_model, "_check_model_constraint")) {
        throw std::invalid_argument("_PythonModel requires a CustomOperationalModel instance");
    }
}

void PythonModel::ComputeNextState(
    double dT,
    const GenericAgent& current,
    GenericAgent& next,
    const CollisionGeometry& geometry,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch) const
{
    py::gil_scoped_acquire gil;

    py::object pythonAgent = py::cast(current);
    py::object pythonGeometry = py::cast(&geometry, py::return_value_policy::reference);
    py::object pythonNeighborhoodSearch =
        py::cast((&neighborhoodSearch), py::return_value_policy::reference);

    py::object pythonUpdate = _model.attr("_compute_next_state")(
        dT, pythonAgent, pythonGeometry, pythonNeighborhoodSearch);

    // "next" shares the Python state object with "current" (GilSafePyObject copies are
    // refcounted, not cloned), so this also rejects returning the current state instance.
    auto& customModelData = std::get<CustomModelData>(next.model).Get<GilSafePyObject>();
    if(pythonUpdate.is(customModelData.Get())) {
        throw SimulationError(
            "Current and updated model state are the same instance. "
            "compute_next_state() must return a new state object, "
            "e.g. dataclasses.replace(ped.model, ...).");
    }

    constexpr auto attr_name = "position";
    py::object attr;
    try {
        attr = pythonUpdate.attr(attr_name);
    } catch(const py::error_already_set& ex) {
        if(ex.matches(PyExc_AttributeError)) {
            throw SimulationError(
                "State returned by compute_next_state() is missing the '{}' attribute.",
                attr_name);
        }
        throw;
    }

    try {
        next.pos = intoPoint(py::cast<std::tuple<double, double>>(attr));
    } catch(const py::cast_error&) {
        // Diagnostics run Python code on the offending object; they must not
        // be able to replace the error they describe.
        std::string actualType = "<unknown>";
        std::string valueRepr = "<unprintable>";
        try {
            actualType = std::string(py::str(py::type::of(attr).attr("__name__")));
        } catch(const py::error_already_set&) {
        }
        try {
            valueRepr = std::string(py::repr(attr));
        } catch(const py::error_already_set&) {
        }

        throw SimulationError(
            "State returned by compute_next_state() has attribute '{}' of wrong type: "
            "expected tuple[float, float], got {} ({})",
            attr_name,
            actualType,
            valueRepr);
    }
    customModelData.Set(pythonUpdate);
}

void PythonModel::CheckModelConstraint(
    const GenericAgent& agent,
    const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
    const CollisionGeometry& geometry) const
{
    py::gil_scoped_acquire gil;

    py::object pythonAgent = py::cast(agent);
    py::object pythonNeighborhoodSearch =
        py::cast((&neighborhoodSearch), py::return_value_policy::reference);
    py::object pythonGeometry = py::cast(&geometry, py::return_value_policy::reference);

    _model.attr("_check_model_constraint")(pythonAgent, pythonNeighborhoodSearch, pythonGeometry);
}

void init_python_model(py::module_& m)
{
    py::class_<OperationalModel, py::smart_holder>(m, "OperationalModel");

    py::class_<CustomModelData>(m, "_CustomModelData")
        .def(py::init([](py::object model) {
            return CustomModelData{GilSafePyObject{std::move(model)}};
        }))
        .def_property_readonly(
            "model", [](CustomModelData& data) { return data.Get<GilSafePyObject>().Get(); });

    py::class_<PythonModel, OperationalModel, py::smart_holder>(m, "_PythonModel")
        .def(py::init<py::object>(), py::arg("model"));
}
