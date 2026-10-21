// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "generic_agent.hpp"
#include "geometric_functions.hpp"
#include "line_segment.hpp"
#include "point.hpp"
#include "unique_id.hpp"
#include "util.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <unordered_set>
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
    ID id() const { return _id; }
    size_t count_targeting() const { return _targeting; }
    void increase_targeting() { _targeting = _targeting + 1; }
    void decrease_targeting()
    {
        assert(_targeting >= 1);
        _targeting = _targeting - 1;
    }
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
class Exit : public BaseStage
{
    Destination _destination;
    std::vector<GenericAgent::ID>& _to_remove;

public:
    Exit(Destination destination, std::vector<GenericAgent::ID>& to_remove);
    ~Exit() override = default;
    bool is_completed(const GenericAgent& agent) override;
    RoutingTarget target(const GenericAgent& agent) override;
    const Destination* destination() const override { return &_destination; }
};

class DirectSteering : public BaseStage
{
public:
    DirectSteering() = default;
    ~DirectSteering() override = default;
    bool is_completed(const GenericAgent&) override { return false; };
    RoutingTarget target(const GenericAgent& agent) override;
};
