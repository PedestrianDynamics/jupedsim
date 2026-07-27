// SPDX-License-Identifier: LGPL-3.0-or-later
#include "python_model.hpp"

#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "OperationalModel.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "OperationalModels/OperationalModelState.hpp"
#include "Point.hpp"
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
    const OperationalModelState& current,
    OperationalModelState& next,
    const Point& destination,
    const EnvironmentQuery& envQuery) const
{
    py::gil_scoped_acquire gil;

    const auto& currentState = std::get<CustomModel::State>(current);
    auto& nextState = std::get<CustomModel::State>(next);

    // Copy-construct the Python-facing state (shares GilSafePyObject by refcount, not clone).
    py::object pythonState = py::cast(currentState);
    py::object destTuple = py::make_tuple(destination.x, destination.y);
    py::object pythonEnvQuery = py::cast(&envQuery, py::return_value_policy::reference);

    py::object pythonUpdate =
        _model.attr("_compute_next_state")(dT, pythonState, destTuple, pythonEnvQuery);

    // nextState shares Python payload with currentState initially; reject returning the same
    // object.
    auto& customModelData = nextState.Get<GilSafePyObject>();
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
                "State returned by compute_next_state() is missing the '{}' attribute.", attr_name);
        }
        throw;
    }

    try {
        nextState.position = intoPoint(py::cast<std::tuple<double, double>>(attr));
    } catch(const py::cast_error&) {
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
    const OperationalModelState& state,
    const EnvironmentQuery& envQuery) const
{
    py::gil_scoped_acquire gil;

    const auto& customState = std::get<CustomModel::State>(state);
    py::object pythonState = py::cast(customState);
    py::object pythonEnvQuery = py::cast(&envQuery, py::return_value_policy::reference);

    _model.attr("_check_model_constraint")(pythonState, pythonEnvQuery);
}

void init_python_model(py::module_& m)
{
    py::class_<OperationalModel, py::smart_holder>(m, "OperationalModel");

    py::class_<CustomModel::State>(m, "_CustomModelState")
        .def(py::init([](py::object model) {
            // Prime the GIL-free position cache from the wrapped state so the
            // framework can spawn the agent at the state's position.
            const auto position =
                intoPoint(py::cast<std::tuple<double, double>>(model.attr("position")));
            CustomModel::State data{GilSafePyObject{std::move(model)}};
            data.position = position;
            return data;
        }))
        .def_property_readonly(
            "model", [](CustomModel::State& data) { return data.Get<GilSafePyObject>().Get(); })
        .def_property_readonly("position", [](const CustomModel::State& data) {
            return std::make_tuple(data.position.x, data.position.y);
        });

    py::class_<PythonModel, OperationalModel, py::smart_holder>(m, "_PythonModel")
        .def(py::init<py::object>(), py::arg("model"));
}
