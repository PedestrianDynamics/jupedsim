// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_logging(py::module_& m);
void init_build_info(py::module_& m);
void init_trace(py::module_& m);
void init_generalized_centrifugal_force_model(py::module_& m);
void init_collision_free_speed_model(py::module_& m);
void init_social_force_model(py::module_& m);
void init_geometry(py::module_& m);
void init_routing(py::module_& m);
void init_agent(py::module_& m);
void init_transition(py::module_& m);
void init_journey(py::module_& m);
void init_stage(py::module_& m);
void init_simulation(py::module_& m);

PYBIND11_MODULE(py_jupedsim, m)
{
    init_logging(m);
    init_build_info(m);
    init_trace(m);
    init_generalized_centrifugal_force_model(m);
    init_collision_free_speed_model(m);
    init_social_force_model(m);
    init_geometry(m);
    init_routing(m);
    init_agent(m);
    init_transition(m);
    init_journey(m);
    init_stage(m);
    init_simulation(m);
}
