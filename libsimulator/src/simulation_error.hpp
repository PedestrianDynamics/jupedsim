// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <fmt/core.h>
#include <fmt/format.h>

#include <stdexcept>

class SimulationError : public std::runtime_error
{
public:
    template <typename... Args>
    SimulationError(const char* msg, const Args&... args)
        : std::runtime_error(fmt::format(fmt::runtime(msg), args...))
    {
    }
};

/// An id that refers to nothing.
class NotFoundError : public SimulationError
{
public:
    using SimulationError::SimulationError;
};

/// A value that is not allowed, e.g. an invalid polygon or a model parameter out of range.
class InvalidValueError : public SimulationError
{
public:
    using SimulationError::SimulationError;
};

/// An object of the wrong type, e.g. a state that belongs to another model.
class InvalidTypeError : public SimulationError
{
public:
    using SimulationError::SimulationError;
};

/// A check that should never fail: if it does, it is a bug in JuPedSim, not in the caller's
/// input. Deliberately no SimulationError, so catching those does not hide bugs.
class JupedsimInternalError : public std::logic_error
{
public:
    template <typename... Args>
    JupedsimInternalError(const char* msg, const Args&... args)
        : std::logic_error(fmt::format(fmt::runtime(msg), args...))
    {
    }
};
