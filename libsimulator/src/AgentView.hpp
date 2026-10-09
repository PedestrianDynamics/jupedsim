// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <cmath>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <vector>

/// A neighbouring agent as seen from the agent that asked for it.
struct NeighborView {
    Point relative_position;
    const OperationalModelState* state;

private:
    /// Internal Location of the agent. Only AgentView has access to it.
    friend class AgentView;
    NeighborView(Point relative, const OperationalModelState* model, const Location* where)
        : relative_position(relative), state(model), _location(where)
    {
    }
    const Location* _location;
};

/// A wall segment as seen from the agent that asked for it. It carries what agents typically
/// use/compute to avoid repetitions plus harmonizes computation.
struct WallView {
    /// The segment itself, relative to the agent.
    LineSegment segment;
    /// The point on the segment closest to the agent.
    Point closest_point;
    /// Distance to that point.
    double distance;
    /// Unit vector pointing from the wall towards the agent, i.e. the direction a repulsion
    /// acts in. Zero for an agent standing exactly on the wall, where no direction exists.
    Point normal;
};

inline bool intersects(const LineSegment& los, const WallView& wall)
{
    return intersects(los, wall.segment);
}
/// Substitutes the model state neighbors are seen with.
///
/// A model that delegates a step to another model hands over a view of the world, but the
/// delegate expects neighbors to carry its own state type. An implementation maps each
/// neighbor's stored state to one the delegate understands and owns the mapped states for as
/// long as it lives.
class NeighborStateMapper
{
public:
    virtual ~NeighborStateMapper() = default;
    virtual const OperationalModelState&
    map_to_current_state(const OperationalModelState& agent) const = 0;
};

/// What an agent perceives of its surroundings, expressed relative to where it
/// stands. Agents do not know absolute positions as they do not need to.
class AgentView
{
public:
    AgentView(
        const EnvironmentQuery& world,
        const GenericAgent& agent,
        const NeighborStateMapper* neighbor_state_mapper = nullptr)
        : _world(world), _agent(agent), _neighbor_state_mapper(neighbor_state_mapper)
    {
    }

    struct AcceptAllNeighbors {
        bool operator()(const NeighborView&) const { return true; }
    };

    /// All agents within 'radius', excluding this agent.
    template <std::predicate<const NeighborView&> Pred = AcceptAllNeighbors>
    std::vector<NeighborView> other_agents_in_range(double radius, Pred filter = {}) const
    {
        std::vector<NeighborView> neighbors{};
        // The grid searches by (x, y). Only filters out the asking agent itself plus applies
        // a quick z-filter - whether agents are "too far" away in z.
        const double z = location().z();
        _world.for_each_agent_in_range(location().xy(), radius, [&](const GenericAgent& candidate) {
            if(candidate.id == _agent.id) {
                return;
            }
            if(std::abs(candidate.location.z() - z) > interaction_height) {
                return;
            }
            const NeighborView neighbor{
                candidate.location.xy() - location().xy(),
                _neighbor_state_mapper ?
                    &_neighbor_state_mapper->map_to_current_state(candidate.state) :
                    &candidate.state,
                &candidate.location};
            if(filter(neighbor)) {
                neighbors.push_back(neighbor);
            }
        });
        return neighbors;
    }

    /// Whether neighbors are seen through a substituted state rather than their own.
    bool has_neighbor_mapping() const { return _neighbor_state_mapper != nullptr; }

    /// Whether the straight line to a point at 'relative_position' is free of geometry.
    bool no_geometry_between(Point relative_position) const
    {
        return _world.no_geometry_between(location(), relative_position);
    }

    /// Whether 'neighbor' can be seen from here. In practice whetehr the direct path to
    /// the neighbor can be walked on the surface.
    bool no_geometry_between(const NeighborView& neighbor) const
    {
        return _world.no_geometry_between(location(), *neighbor._location);
    }

private:
    /// The segments as seen from the agent. The query hands over what it found; the view
    /// owns it from here and turns it into WallViews one at a time, as they are asked for.
    /// Must stay above walls_in_range() in code: an 'auto' return type is deduced from the
    /// body, so unlike other members this one cannot be called before it is defined.
    auto as_seen_from_agent(std::vector<LineSegment> segments) const
    {
        return std::move(segments) |
               std::views::transform([origin = location().xy()](const LineSegment& s) {
                   const LineSegment segment{s.p1 - origin, s.p2 - origin};
                   const Point closest_point = segment.shortest_point(Point{});
                   return WallView{
                       .segment = segment,
                       .closest_point = closest_point,
                       .distance = closest_point.norm(),
                       .normal = (Point{} - closest_point).normalized()};
               });
    }

public:
    /// Wall segments within 'distance' of the agent, relative to it. Returns lazy range.
    auto walls_in_range(double distance) const
    {
        return as_seen_from_agent(_world.line_segments_in_range(location(), distance));
    }

    /// The same view, but with neighbors seen through 'states'. 'states' has to outlive the
    /// returned view. AgentStep shadows this function. AgentStep has an
    /// additional member (dt) that needs to be passed to the returned AgentStep.
    AgentView with_neighbor_state_mapping(const NeighborStateMapper& states) const
    {
        return AgentView{_world, _agent, &states};
    }

protected:
    /// Where the agent stands on the surface.
    const Location& location() const { return _agent.location; }

    const EnvironmentQuery& _world;
    const GenericAgent& _agent;
    /// Null in the common case, where neighbors are seen with their own state.
    const NeighborStateMapper* _neighbor_state_mapper;
};

/// An AgentView plus what only holds for one step (dt + next target).
class AgentStep : public AgentView
{
public:
    AgentStep(
        const EnvironmentQuery& world,
        const GenericAgent& agent,
        double dt,
        const NeighborStateMapper* neighbor_state_mapper = nullptr)
        : AgentView(world, agent, neighbor_state_mapper), _dt(dt)
    {
    }

    /// The same step, but with neighbors seen through 'states'. 'states' has to outlive the
    /// returned step.
    AgentStep with_neighbor_state_mapping(const NeighborStateMapper& states) const
    {
        return AgentStep{_world, _agent, _dt, &states};
    }

    double dt() const { return _dt; }

    /// Unit vector along the route to the final target. Zero when the agent has already
    /// reached it.
    Point route_orientation() const { return _agent.route_orientation; }

private:
    double _dt;
};

// Both views are passed by reference and never deleted through a base pointer; keeping
// them non-polymorphic keeps them free of a vtable and fully inlinable.
static_assert(!std::is_polymorphic_v<AgentView>);
static_assert(!std::is_polymorphic_v<AgentStep>);
