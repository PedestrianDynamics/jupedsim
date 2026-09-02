// SPDX-License-Identifier: LGPL-3.0-or-later
#include "python_model.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "OperationalModel.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "SimulationError.hpp"
#include "conversion.hpp"

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

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

/// Best-effort "<repr> (of type <T>)" for an object that failed a conversion. The
/// diagnostics run Python code on the offending object; they must not be able to
/// replace the error they describe.
static std::string Describe(const py::object& obj)
{
    std::string type = "<unknown>";
    std::string repr = "<unprintable>";
    try {
        type = std::string(py::str(py::type::of(obj).attr("__name__")));
    } catch(const py::error_already_set&) {
    }
    try {
        repr = std::string(py::repr(obj));
    } catch(const py::error_already_set&) {
    }
    return fmt::format("{} (of type {})", repr, type);
}

Point PythonModel::ComputeNextState(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    py::gil_scoped_acquire gil;

    py::object pythonState = std::get<CustomModel::State>(current).Get<GilSafePyObject>().Get();
    py::object pythonStep = py::cast(&step, py::return_value_policy::reference);

    py::object pythonUpdate = _model.attr("_compute_next_state")(pythonState, pythonStep);

    if(!py::isinstance<py::tuple>(pythonUpdate)) {
        throw SimulationError(
            "compute_next_state() must return a (state, movement) pair, got {}",
            Describe(pythonUpdate));
    }
    auto update = py::reinterpret_borrow<py::tuple>(pythonUpdate);
    if(update.size() != 2) {
        throw SimulationError(
            "compute_next_state() must return a (state, movement) pair, got {} values",
            update.size());
    }
    py::object nextState = update[0];
    py::object movementValue = update[1];

    // "next" shares the Python state object with "current" (GilSafePyObject copies are
    // refcounted, not cloned), so this also rejects returning the current state instance.
    auto& customModelData = std::get<CustomModel::State>(next).Get<GilSafePyObject>();
    if(nextState.is(customModelData.Get())) {
        throw SimulationError(
            "Current and updated model state are the same instance. "
            "compute_next_state() must return a new state object, "
            "e.g. dataclasses.replace(state, ...).");
    }

    Point movement{};
    try {
        movement = intoPoint(py::cast<std::tuple<double, double>>(movementValue));
    } catch(const py::cast_error&) {
        throw SimulationError(
            "Movement returned by compute_next_state() is of wrong type: "
            "expected tuple[float, float], got {}",
            Describe(movementValue));
    }
    customModelData.Set(nextState);
    return movement;
}

void PythonModel::CheckModelConstraint(const GenericAgent& agent, const AgentView& view) const
{
    py::gil_scoped_acquire gil;

    py::object pythonState = std::get<CustomModel::State>(agent.state).Get<GilSafePyObject>().Get();
    py::object pythonView = py::cast(&view, py::return_value_policy::reference);

    _model.attr("_check_model_constraint")(pythonState, pythonView);
}

/// The Python surface wraps the native view in jupedsim.agent_view.AgentStep. A model that
/// delegates hands on the step it was given, so accept either the wrapper or the native object.
static const AgentStep& asAgentStep(const py::object& step)
{
    const py::object native = py::hasattr(step, "_obj") ? step.attr("_obj") : step;
    try {
        return *native.cast<const AgentStep*>();
    } catch(const py::cast_error&) {
        throw SimulationError(
            "compute_next_state() expects the step it was called with, got {}", Describe(step));
    }
}

void init_python_model(py::module_& m)
{
    py::class_<OperationalModel, py::smart_holder>(m, "OperationalModel")
        .def(
            "compute_next_state",
            [](const OperationalModel& self, OperationalModelState state, py::object step) {
                const AgentStep& agentStep = asAgentStep(step);
                const OperationalModelState current{std::move(state)};
                if(ModelTypeOf(current) != self.Type()) {
                    throw SimulationError(
                        "{} cannot compute a state of type '{}'",
                        ToString(self.Type()),
                        ToString(ModelTypeOf(current)));
                }
                OperationalModelState next{current};

                Point movement{};
                try {
                    movement = self.ComputeNextState(current, next, agentStep);
                } catch(const std::bad_variant_access&) {
                    // The state handed in is of the right type, so it was a neighbor's.
                    throw SimulationError(
                        agentStep.HasNeighborMapping() ?
                            "{} encountered a neighbor it cannot read. The mapping passed to "
                            "with_neighbor_states() has to return '{}' states." :
                            "{} encountered a neighbor it cannot read. Map neighbors to '{}' "
                            "states with AgentStep.with_neighbor_states() before delegating.",
                        ToString(self.Type()),
                        ToString(self.Type()));
                }

                return std::make_tuple(
                    static_cast<const OperationalModelState&>(next),
                    std::make_tuple(movement.x, movement.y));
            },
            py::arg("state"),
            py::arg("step"),
            "Run this model for one step on 'state', as perceived through 'step'. Returns "
            "(next_state, movement). The agent's stored state is not touched.");

    py::class_<CustomModel::State>(m, "_CustomModelState")
        .def(py::init([](py::object model) {
            return CustomModel::State{GilSafePyObject{std::move(model)}};
        }))
        .def_property_readonly(
            "model", [](CustomModel::State& data) { return data.Get<GilSafePyObject>().Get(); });

    py::class_<PythonModel, OperationalModel, py::smart_holder>(m, "_PythonModel")
        .def(py::init<py::object>(), py::arg("model"));
}
