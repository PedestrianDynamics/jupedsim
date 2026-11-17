// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "destination.hpp"
#include "generic_agent.hpp"
#include "unique_id.hpp"

#include <fmt/core.h>

#include <cassert>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>

class BaseStage
{
public:
    using ID = jps::UniqueID<BaseStage>;

protected:
    ID _id;
    std::optional<std::string> _name;
    Destination _destination;
    size_t _targeting{0};

public:
    BaseStage(Destination destination, std::optional<std::string> name)
        : _name(std::move(name)), _destination(destination)
    {
    }
    virtual ~BaseStage() = default;
    bool is_completed(const GenericAgent& agent) const
    {
        return _destination.contains(agent.location);
    }
    /// The area this stage routes to.
    const Destination& destination() const { return _destination; }
    ID id() const { return _id; }
    const std::optional<std::string>& name() const { return _name; }
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
        if(s.name()) {
            return fmt::format_to(
                ctx.out(),
                "(id={}, name={}, targeting={})",
                s.id(),
                *s.name(),
                s.count_targeting());
        }
        return fmt::format_to(ctx.out(), "(id={}, targeting={})", s.id(), s.count_targeting());
    }
};

class Waypoint : public BaseStage
{
public:
    using BaseStage::BaseStage;
};

/// Agents that complete an exit leave the simulation at the beginning of the next iteration.
class Exit : public BaseStage
{
public:
    using BaseStage::BaseStage;
};
