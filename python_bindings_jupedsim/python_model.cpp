// SPDX-License-Identifier: LGPL-3.0-or-later
#include "python_model.hpp"

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModel.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
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

void PythonModel::GetNeighbors(
    const GenericState& /*current*/,
    const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
    const CollisionGeometry& /*geometry*/,
    StateContainer& neighbor_states) const
{
    _neighborhoodSearch = &neighborhoodsearch;
    neighbor_states.clear();
}

void PythonModel::ComputeNextState(
    double dT,
    const GenericState& current,
    GenericState& next,
    const AgentJourney& journey,
    const CollisionGeometry& geometry,
    const StateContainer& /*neighborStates*/) const
{
    py::gil_scoped_acquire gil;

    // Rebuild a transient agent view for the Python callback. Journey and stage ids are
    // not part of the operational state and stay invalid here.
    GenericAgent agent{
        Id(current),
        jps::UniqueID<Journey>::Invalid,
        jps::UniqueID<BaseStage>::Invalid,
        current};
    agent.journey = journey;

    py::object pythonAgent = py::cast(agent);
    py::object pythonGeometry = py::cast(&geometry, py::return_value_policy::reference);
    py::object pythonNeighborhoodSearch =
        py::cast(_neighborhoodSearch, py::return_value_policy::reference);

    py::object pythonUpdate = _model.attr("_compute_next_state")(
        dT, pythonAgent, pythonGeometry, pythonNeighborhoodSearch);

    // "next" shares the Python state object with "current" (GilSafePyObject copies are
    // refcounted, not cloned), so this also rejects returning the current state instance.
    auto& nextStateData = std::get<CustomModel::State>(next);
    auto& customStateData = nextStateData.Get<GilSafePyObject>();
    if(pythonUpdate.is(customStateData.Get())) {
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
        // Sync the GIL-free position cache from the returned Python state so the
        // framework can read the agent position without acquiring the GIL.
        nextStateData.position = intoPoint(py::cast<std::tuple<double, double>>(attr));
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
    customStateData.Set(pythonUpdate);
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
            "model", [](CustomModel::State& data) { return data.Get<GilSafePyObject>().Get(); });

    py::class_<PythonModel, OperationalModel, py::smart_holder>(m, "_PythonModel")
        .def(py::init<py::object>(), py::arg("model"));
}
