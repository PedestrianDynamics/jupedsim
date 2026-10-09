// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "AgentView.hpp"

#include <pybind11/pybind11.h>

#include <deque>
#include <memory>

namespace py = pybind11;

/// A model state as user code sees it. Custom states are unwrapped so that the
/// _CustomModelState transport type never reaches user code.
py::object state_to_python(const OperationalModelState& state);

/// Maps neighbor states through a Python callable, e.g. to hand a built-in model neighbors of
/// the state type it expects.
class PythonNeighborStateMapper : public NeighborStateMapper
{
public:
    explicit PythonNeighborStateMapper(py::object repack);

    const OperationalModelState&
    map_to_current_state(const OperationalModelState& state) const override;

private:
    py::object _repack;
    /// Deque, not vector: MappedTo() hands out references that must survive later calls.
    mutable std::deque<OperationalModelState> _states{};
};

/// Lifetime of one custom-model callback. Views handed to Python, and every view derived
/// from them, share one scope; it is closed when the callback returns and every later access
/// raises SimulationError.
class CallbackScope
{
public:
    void close() { _open = false; }
    /// Throws SimulationError("<what> is only valid during the callback it was passed to; do
    /// not store it.") when closed.
    void check(const char* what) const;

private:
    bool _open{true};
};

/// Closes a scope on every exit path, including exceptions.
class CloseScopeOnExit
{
public:
    explicit CloseScopeOnExit(CallbackScope& scope) : _scope(scope) {}
    ~CloseScopeOnExit() { _scope.close(); }
    CloseScopeOnExit(const CloseScopeOnExit&) = delete;
    CloseScopeOnExit& operator=(const CloseScopeOnExit&) = delete;

private:
    CallbackScope& _scope;
};

/// jupedsim.AgentView
class PyAgentView
{
public:
    PyAgentView(
        const AgentView& view,
        std::shared_ptr<const CallbackScope> scope,
        std::shared_ptr<const NeighborStateMapper> mapper = {});
    /// The libsimulator view; checks the scope first.
    const AgentView& view() const;
    const std::shared_ptr<const CallbackScope>& scope() const { return _scope; }
    const std::shared_ptr<const NeighborStateMapper>& mapper() const { return _mapper; }

private:
    AgentView _view;
    std::shared_ptr<const CallbackScope> _scope;
    /// Keeps the mapper of a derived view alive as long as the view.
    std::shared_ptr<const NeighborStateMapper> _mapper;
};

/// jupedsim.AgentStep
class PyAgentStep : public PyAgentView
{
public:
    PyAgentStep(
        const AgentStep& step,
        std::shared_ptr<const CallbackScope> scope,
        std::shared_ptr<const NeighborStateMapper> mapper = {});
    /// The libsimulator step; checks the scope first.
    const AgentStep& step() const;

private:
    AgentStep _step;
};

/// jupedsim.NeighborView. Shares the scope and the mapper of the view it was obtained from:
/// its state lives in the agent store or in that mapper.
class PyNeighborView
{
public:
    PyNeighborView(
        const NeighborView& neighbor,
        std::shared_ptr<const CallbackScope> scope,
        std::shared_ptr<const NeighborStateMapper> mapper);
    /// The libsimulator neighbor; checks the scope first.
    const NeighborView& neighbor() const;
    /// A value copy, valid after the callback too.
    Point relative_position() const { return _neighbor.relative_position; }

private:
    NeighborView _neighbor;
    std::shared_ptr<const CallbackScope> _scope;
    std::shared_ptr<const NeighborStateMapper> _mapper;
};
