// SPDX-License-Identifier: LGPL-3.0-or-later
#include "callback_views.hpp"

#include "operational_models/custom_model/custom_model.hpp"
#include "python_model.hpp"
#include "simulation_error.hpp"
#include "type_casters.hpp" // IWYU pragma: keep

#include <pybind11/stl.h>

#include <utility>
#include <variant>

py::object state_to_python(const OperationalModelState& state)
{
    const auto& variant = static_cast<const OperationalModelState&>(state);
    if(const auto* custom = std::get_if<CustomModel::State>(&variant)) {
        return custom->get<GilSafePyObject>().get();
    }
    return py::cast(variant);
}

PythonNeighborStateMapper::PythonNeighborStateMapper(py::object repack) : _repack(std::move(repack))
{
}

const OperationalModelState&
PythonNeighborStateMapper::map_to_current_state(const OperationalModelState& state) const
{
    py::object repacked = _repack(state_to_python(state));
    try {
        _states.emplace_back(repacked.cast<OperationalModelState>());
    } catch(const py::cast_error&) {
        // Anything that is not a built-in state is a custom state, exactly as when an
        // agent is added. This is what a Python model delegating to another Python model
        // returns.
        _states.emplace_back(CustomModel::State{GilSafePyObject{std::move(repacked)}});
    }
    return _states.back();
}

void CallbackScope::check(const char* what) const
{
    if(!_open) {
        throw SimulationError(
            "{} is only valid during the callback it was passed to; do not store it.", what);
    }
}

PyAgentView::PyAgentView(
    const AgentView& view,
    std::shared_ptr<const CallbackScope> scope,
    std::shared_ptr<const NeighborStateMapper> mapper)
    : _view(view), _scope(std::move(scope)), _mapper(std::move(mapper))
{
}

const AgentView& PyAgentView::view() const
{
    _scope->check("AgentView");
    return _view;
}

PyAgentStep::PyAgentStep(
    const AgentStep& step,
    std::shared_ptr<const CallbackScope> scope,
    std::shared_ptr<const NeighborStateMapper> mapper)
    : PyAgentView(static_cast<const AgentView&>(step), std::move(scope), std::move(mapper))
    , _step(step)
{
}

const AgentStep& PyAgentStep::step() const
{
    scope()->check("AgentStep");
    return _step;
}

PyNeighborView::PyNeighborView(
    const NeighborView& neighbor,
    std::shared_ptr<const CallbackScope> scope,
    std::shared_ptr<const NeighborStateMapper> mapper)
    : _neighbor(neighbor), _scope(std::move(scope)), _mapper(std::move(mapper))
{
}

const NeighborView& PyNeighborView::neighbor() const
{
    _scope->check("NeighborView");
    return _neighbor;
}
