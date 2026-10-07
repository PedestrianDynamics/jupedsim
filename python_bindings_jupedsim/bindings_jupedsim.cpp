// SPDX-License-Identifier: LGPL-3.0-or-later
#include "SimulationError.hpp"

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>

#include <string>

namespace py = pybind11;

void init_logging(py::module_& m);
void init_build_info(py::module_& m);
void init_trace(py::module_& m);
void init_generalized_centrifugal_force_model(py::module_& m);
void init_collision_free_speed_model(py::module_& m);
void init_collision_free_speed_model_v2(py::module_& m);
void init_collision_free_speed_model_v3(py::module_& m);
void init_anticipation_velocity_model(py::module_& m);
void init_social_force_model(py::module_& m);
void init_warp_driver_model(py::module_& m);
void init_geometry(py::module_& m);
void init_routing(py::module_& m);
void init_agent(py::module_& m);
void init_transition(py::module_& m);
void init_stage(py::module_& m);
void init_simulation(py::module_& m);
void init_agent_view(py::module_& m);
void init_python_model(py::module_& m);
void init_boundary_index(py::module_& m);
void init_walkable_surface(py::module_& m);

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

PYBIND11_MODULE(py_jupedsim, m)
{
    py::register_exception<SimulationError>(m, "SimulationError").attr("__doc__") =
        "Raised for simulation errors, e.g. when accessing an agent handle whose"
        "agent no longer exists or when calling mutating simulation methods from a"
        "custom-model callback.";
    init_logging(m);
    init_build_info(m);
    init_trace(m);
    init_python_model(m);
    init_generalized_centrifugal_force_model(m);
    init_collision_free_speed_model(m);
    init_collision_free_speed_model_v2(m);
    init_collision_free_speed_model_v3(m);
    init_anticipation_velocity_model(m);
    init_social_force_model(m);
    init_warp_driver_model(m);
    init_agent_view(m);
    init_geometry(m);
    init_routing(m);
    init_agent(m);
    init_transition(m);
    init_stage(m);
    init_simulation(m);
    init_boundary_index(m);
    init_walkable_surface(m);
    set_all(m);
}
