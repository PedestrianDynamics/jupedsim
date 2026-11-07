// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "generic_agent.hpp"
#include "routing_target.hpp"
#include "simulation_clock.hpp"
#include "unique_id.hpp"

#include <fmt/core.h>

#include <cassert>
#include <cstddef>
#include <optional>
#include <vector>

class BaseStage
{
public:
    using ID = jps::UniqueID<BaseStage>;

protected:
    ID _id;
    size_t _targeting{0};

public:
    virtual ~BaseStage() = default;
    virtual bool is_completed(const GenericAgent& agent) = 0;
    virtual RoutingTarget target(const GenericAgent& agent) = 0;
    /// The area this stage routes to, or nullptr if it has none.
    virtual const Destination* destination() const { return nullptr; }
    /// A closed stage is no valid target: rules pass over it.
    virtual bool closed() const { return false; }
    ID id() const { return _id; }
    size_t count_targeting() const { return _targeting; }
    void increase_targeting() { _targeting = _targeting + 1; }
    void decrease_targeting()
    {
        assert(_targeting >= 1);
        _targeting = _targeting - 1;
    }
};

/// An agent leaving the simulation: through @p exit, or without one when it was marked for
/// removal. @p time is the elapsed time at which it reached the exit or was marked.
struct RemovalRecord {
    GenericAgent::ID id;
    std::optional<BaseStage::ID> exit;
    double time;
};

template <>
struct fmt::formatter<BaseStage> {

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const BaseStage& s, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "(id={}, targeting={})", s.id(), s.count_targeting());
    }
};

class Waypoint : public BaseStage
{
    Destination _destination;

public:
    explicit Waypoint(Destination destination);
    ~Waypoint() override = default;
    bool is_completed(const GenericAgent& agent) override;
    RoutingTarget target(const GenericAgent& agent) override;
    const Destination* destination() const override { return &_destination; }
};

/// Notifies simulation of all agents that need to be removed at the beginning of the next iteration
/// While closed, agents reaching it stay in the simulation.
class Exit : public BaseStage
{
    Destination _destination;
    std::vector<RemovalRecord>& _to_remove;
    const SimulationClock& _clock;
    bool _closed{false};

public:
    Exit(
        Destination destination,
        std::vector<RemovalRecord>& to_remove,
        const SimulationClock& clock);
    ~Exit() override = default;
    bool is_completed(const GenericAgent& agent) override;
    RoutingTarget target(const GenericAgent& agent) override;
    const Destination* destination() const override { return &_destination; }
    bool closed() const override { return _closed; }
    void close() { _closed = true; }
    void open() { _closed = false; }
};

class DirectSteering : public BaseStage
{
public:
    DirectSteering() = default;
    ~DirectSteering() override = default;
    /// True once the agent has reached its point target, see has_reached().
    bool is_completed(const GenericAgent& agent) override;
    RoutingTarget target(const GenericAgent& agent) override;
};
