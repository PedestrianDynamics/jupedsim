// SPDX-License-Identifier: LGPL-3.0-or-later
#include "python_model.hpp"

#include "AgentView.hpp"
#include "GenericAgent.hpp"
#include "OperationalModel.hpp"
#include "OperationalModels/CustomModel/CustomModel.hpp"
#include "SimulationError.hpp"
#include "callback_views.hpp"
#include "conversion.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <fmt/format.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
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

const py::object& GilSafePyObject::get() const
{
    return _obj;
}

py::object& GilSafePyObject::get()
{
    return _obj;
}

void GilSafePyObject::set(py::object obj)
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
    if(!py::hasattr(_model, "compute_next_state") ||
       !py::hasattr(_model, "check_model_constraint")) {
        throw std::invalid_argument("_PythonModel requires a CustomOperationalModel instance");
    }
}

/// Best-effort "<repr> (of type <T>)" for an object that failed a conversion. The
/// diagnostics run Python code on the offending object; they must not be able to
/// replace the error they describe.
static std::string describe(const py::object& obj)
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

Point PythonModel::compute_next_state(
    const OperationalModelState& current,
    OperationalModelState& next,
    const AgentStep& step) const
{
    py::gil_scoped_acquire gil;

    py::object python_state = std::get<CustomModel::State>(current).get<GilSafePyObject>().get();
    // The views handed to Python expire with the callback, also when it raises.
    auto scope = std::make_shared<CallbackScope>();
    const CloseScopeOnExit close_scope{*scope};
    py::object python_step = py::cast(PyAgentStep{step, scope});

    py::object python_update = _model.attr("compute_next_state")(python_state, python_step);

    if(!py::isinstance<py::tuple>(python_update)) {
        throw SimulationError(
            "compute_next_state() must return a (state, movement) pair, got {}",
            describe(python_update));
    }
    auto update = py::reinterpret_borrow<py::tuple>(python_update);
    if(update.size() != 2) {
        throw SimulationError(
            "compute_next_state() must return a (state, movement) pair, got {} values",
            update.size());
    }
    py::object next_state = update[0];
    py::object movement_value = update[1];

    // "next" shares the Python state object with "current" (GilSafePyObject copies are
    // refcounted, not cloned), so this also rejects returning the current state instance.
    auto& custom_model_data = std::get<CustomModel::State>(next).get<GilSafePyObject>();
    if(next_state.is(custom_model_data.get())) {
        throw SimulationError(
            "Current and updated model state are the same instance. "
            "compute_next_state() must return a new state object, "
            "e.g. dataclasses.replace(state, ...).");
    }

    Point movement{};
    try {
        movement = into_point(py::cast<std::tuple<double, double>>(movement_value));
    } catch(const py::cast_error&) {
        throw SimulationError(
            "Movement returned by compute_next_state() is of wrong type: "
            "expected tuple[float, float], got {}",
            describe(movement_value));
    }
    custom_model_data.set(next_state);
    return movement;
}

void PythonModel::check_model_constraint(const GenericAgent& agent, const AgentView& view) const
{
    py::gil_scoped_acquire gil;

    py::object python_state =
        std::get<CustomModel::State>(agent.state).get<GilSafePyObject>().get();
    auto scope = std::make_shared<CallbackScope>();
    const CloseScopeOnExit close_scope{*scope};
    py::object python_view = py::cast(PyAgentView{view, scope});

    _model.attr("check_model_constraint")(python_state, python_view);
}

/// A model that delegates hands on the step it was given (or one derived from it).
static const AgentStep& as_agent_step(const py::object& step)
{
    try {
        return step.cast<const PyAgentStep&>().step();
    } catch(const py::cast_error&) {
        throw SimulationError(
            "compute_next_state() expects the step it was called with, got {}", describe(step));
    }
}

void init_python_model(py::module_& m)
{
    py::class_<CustomModel::State>(m, "_CustomModelState")
        .def(py::init([](py::object model) {
            return CustomModel::State{GilSafePyObject{std::move(model)}};
        }))
        .def_property_readonly(
            "model", [](CustomModel::State& data) { return data.get<GilSafePyObject>().get(); });

    py::class_<OperationalModel, py::smart_holder>(m, "OperationalModel")
        .def(
            "compute_next_state",
            [](const OperationalModel& self, OperationalModelState state, py::object step) {
                const AgentStep& agent_step = as_agent_step(step);
                const OperationalModelState current{std::move(state)};
                if(model_type_of(current) != self.type()) {
                    throw SimulationError(
                        "{} cannot compute a state of type '{}'",
                        to_string(self.type()),
                        to_string(model_type_of(current)));
                }
                OperationalModelState next{current};

                Point movement{};
                try {
                    movement = self.compute_next_state(current, next, agent_step);
                } catch(const std::bad_variant_access&) {
                    // The state handed in is of the right type, so it was a neighbor's.
                    throw SimulationError(
                        agent_step.has_neighbor_mapping() ?
                            "{} encountered a neighbor it cannot read. The mapping passed to "
                            "with_neighbor_state_mapping() has to return '{}' states." :
                            "{} encountered a neighbor it cannot read. Map neighbors to '{}' "
                            "states with AgentStep.with_neighbor_state_mapping() before "
                            "delegating.",
                        to_string(self.type()),
                        to_string(self.type()));
                }

                return std::make_tuple(
                    static_cast<const OperationalModelState&>(next),
                    std::make_tuple(movement.x, movement.y));
            },
            py::arg("state"),
            py::arg("step"),
            clean_doc(R"(
                Run this model for one step.

                Lets a custom model delegate to a built-in one. The agent's stored
                state is not touched.

                Args:
                    state: Model state of this model's type to advance.
                    step: The :class:`~jupedsim.AgentStep` the custom model was
                        called with, optionally mapped with
                        :meth:`~jupedsim.AgentStep.with_neighbor_state_mapping`.

                Returns:
                    A pair ``(next_state, movement)``; ``movement`` is the offset
                    ``(dx, dy)`` the agent wants to move by.

                Raises:
                    SimulationError: If ``state`` is not of this model's type,
                        ``step`` is not an :class:`~jupedsim.AgentStep`, or a
                        neighbor carries a state this model cannot read.
            )")
                .c_str());

    py::class_<PythonModel, OperationalModel, py::smart_holder>(m, "_PythonModel")
        .def(py::init<py::object>(), py::arg("model"));
}
