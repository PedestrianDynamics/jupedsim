// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "simulation_error.hpp"

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>

#include <string>

namespace py = pybind11;

void init_logging(py::module_& m);
void init_build_info(py::module_& m);
void init_trace(py::module_& m);
void init_generalized_centrifugal_force_model(py::module_& m);
void init_collision_free_speed_model_tordeux2016(py::module_& m);
void init_collision_free_speed_model(py::module_& m);
void init_motivation_collision_free_speed_model(py::module_& m);
void init_anticipation_velocity_model(py::module_& m);
void init_social_force_model(py::module_& m);
void init_warp_driver_model(py::module_& m);
void init_geometry(py::module_& m);
void init_polygon2d(py::module_& m);
void init_routing(py::module_& m);
void init_agent(py::module_& m);
void init_transition(py::module_& m);
void init_simulation(py::module_& m);
void init_agent_view(py::module_& m);
void init_python_model(py::module_& m);
void init_boundary_index(py::module_& m);
void init_geometry_builder(py::module_& m);
void init_region(py::module_& m);

// Export every public name, recursing into submodules. Must run after all bindings are registered.
static void set_all(py::module_& m)
{
    py::list names{};
    for(const auto& item : m.attr("__dict__").cast<py::dict>()) {
        const auto name = item.first.cast<std::string>();
        if(name.starts_with('_')) {
            continue;
        }
        names.append(name);
        if(py::isinstance<py::module_>(item.second)) {
            auto sub = py::reinterpret_borrow<py::module_>(item.second);
            set_all(sub);
        }
    }
    names.attr("sort")();
    m.attr("__all__") = names;
}

PYBIND11_MODULE(pyjupedsim, m)
{
    const py::module_ builtins = py::module_::import("builtins");
    auto& simulation_error = py::register_exception<SimulationError>(m, "SimulationError");
    simulation_error.attr("__doc__") = clean_doc(R"(
        Raised for simulation errors, e.g. when accessing an agent handle whose
        agent no longer exists or when calling mutating simulation methods from a
        custom-model callback.
    )");
    // Registered after SimulationError: pybind11 tries the latest translator first.
    auto& not_found_error = py::register_exception<NotFoundError>(
        m, "NotFoundError", py::make_tuple(simulation_error, builtins.attr("KeyError")));
    // Ensure KeyError does not add quotes around "our" message as it takes it as the key
    not_found_error.attr("__str__") = builtins.attr("Exception").attr("__str__");
    not_found_error.attr("__doc__") = clean_doc(R"(
        Raised when an id refers to nothing, e.g. an unknown region or agent id.
        It is both a :class:`SimulationError` and a :class:`KeyError`.
    )");
    py::register_exception<InvalidValueError>(
        m, "InvalidValueError", py::make_tuple(simulation_error, builtins.attr("ValueError")))
        .attr("__doc__") = clean_doc(R"(
        Raised when a value is not allowed, e.g. an invalid polygon or a model
        parameter out of range. It is both a :class:`SimulationError` and a
        :class:`ValueError`.
    )");
    py::register_exception<InvalidTypeError>(
        m, "InvalidTypeError", py::make_tuple(simulation_error, builtins.attr("TypeError")))
        .attr("__doc__") = clean_doc(R"(
        Raised when an object has the wrong type, e.g. a state that belongs to
        another model. It is both a :class:`SimulationError` and a
        :class:`TypeError`.
    )");
    py::register_exception<JupedsimInternalError>(
        m, "JupedsimInternalError", builtins.attr("RuntimeError"))
        .attr("__doc__") = clean_doc(R"(
        Raised when a check fails that should never fail. This is a bug in
        JuPedSim, please report it.
    )");
    init_logging(m);
    init_build_info(m);
    init_trace(m);
    init_python_model(m);
    init_generalized_centrifugal_force_model(m);
    init_collision_free_speed_model_tordeux2016(m);
    init_collision_free_speed_model(m);
    init_motivation_collision_free_speed_model(m);
    init_anticipation_velocity_model(m);
    init_social_force_model(m);
    init_warp_driver_model(m);
    init_agent_view(m);
    init_polygon2d(m);
    init_region(m);
    init_geometry(m);
    init_routing(m);
    init_agent(m);
    init_transition(m);
    init_simulation(m);
    init_boundary_index(m);
    init_geometry_builder(m);
    set_all(m);
}
