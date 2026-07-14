// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "OperationalModels/CustomModel/CustomModel.hpp"

#include <pybind11/pybind11.h>

namespace py = pybind11;

/// GIL-safe owner of a py::object, used as the type-erased payload for
/// CustomModel::State (per-agent custom state).
///
/// Copy and copy-assignment SHARE the wrapped object by reference: they incref
/// the same Python object, they do NOT clone it. This is what makes per-step
/// agent value-copies (e.g. neighbor queries) cheap -- the agent struct is
/// copied, but its Python state is only ref-counted, not deep-copied.
///
/// Every operation that changes a Python reference count -- copy, copy-assign,
/// move-assign, Set, and destruction -- acquires the GIL. The holder is therefore
/// safe even if the simulation step is ever run with the GIL released.
/// Move-construction is refcount-neutral (it transfers the pointer and leaves the
/// source empty), so it stays defaulted.
class GilSafePyObject
{
public:
    explicit GilSafePyObject(py::object obj);
    GilSafePyObject(const GilSafePyObject& other);
    GilSafePyObject& operator=(const GilSafePyObject& other);
    GilSafePyObject(GilSafePyObject&&) noexcept = default;
    GilSafePyObject& operator=(GilSafePyObject&&) noexcept;
    ~GilSafePyObject();

    const py::object& Get() const;
    py::object& Get();
    void Set(py::object obj);

private:
    py::object _obj;
};

class PythonModel final : public CustomModel
{
public:
    explicit PythonModel(py::object model);

    /// The Python callback queries neighbors itself with arbitrary radii, so no neighbor
    /// states are collected here; a handle to the neighborhood search is kept for the
    /// ComputeNextState call instead.
    void GetNeighbors(
        const GenericState& current,
        const NeighborhoodSearch<GenericAgent>& neighborhoodsearch,
        const CollisionGeometry& geometry,
        StateContainer& neighbor_states) const override;

    void ComputeNextState(
        double dT,
        const GenericState& current,
        GenericState& next,
        const AgentJourney& journey,
        const CollisionGeometry& geometry,
        const StateContainer& neighborStates) const override;

    void CheckModelConstraint(
        const GenericAgent& agent,
        const NeighborhoodSearch<GenericAgent>& neighborhoodSearch,
        const CollisionGeometry& geometry) const override;

private:
    py::object _model;
    // Set by GetNeighbors immediately before each ComputeNextState call; the search outlives
    // the simulation step, so the pointer stays valid for the paired ComputeNextState.
    mutable const NeighborhoodSearch<GenericAgent>* _neighborhoodSearch{nullptr};
};
