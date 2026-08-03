// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "EnvironmentQuery.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <concepts>
#include <ranges>
#include <type_traits>
#include <vector>

/// A neighbouring agent as seen from the agent that asked for it.
struct NeighborView {
    Point RelativePosition;
    const OperationalModelState* state;
};

/// What an agent perceives of its surroundings, expressed relative to where it
/// stands. Agents do not know absolute positions as they do not need to.
class AgentView
{
public:
    AgentView(const EnvironmentQuery& world, const GenericAgent& agent)
        : _world(world), _agent(agent)
    {
    }

    struct AcceptAllNeighbors {
        bool operator()(const NeighborView&) const { return true; }
    };

    /// All agents within 'radius', excluding this agent. 'filter' is never called with it.
    template <std::predicate<const NeighborView&> Pred = AcceptAllNeighbors>
    std::vector<NeighborView> OtherAgentsInRange(double radius, Pred filter = {}) const
    {
        std::vector<NeighborView> neighbors{};
        _world.ForEachAgentInRange(_agent.position, radius, [&](const GenericAgent& candidate) {
            if(candidate.id == _agent.id) {
                return;
            }
            const NeighborView neighbor{candidate.position - _agent.position, &candidate.model};
            if(filter(neighbor)) {
                neighbors.push_back(neighbor);
            }
        });
        return neighbors;
    }

    /// Whether the straight line to a point at 'RelativePosition' is free of geometry.
    /// 'boundaries' must be in the same relative coordinate frame (agent at origin),
    /// i.e. come from WallsNearby() or WallsInRange().
    template <typename Range>
    bool NoGeometryBetween(Point RelativePosition, const Range& boundaries) const
    {
        const LineSegment los{Point{}, RelativePosition};
        return !std::ranges::any_of(
            boundaries, [&los](const LineSegment& seg) { return intersects(los, seg); });
    }

    /// Whether the point reached by moving 'RelativePosition' is inside the walkable area.
    bool InsideGeometry(Point RelativePosition) const
    {
        return _world.InsideGeometry(_agent.position + RelativePosition);
    }

    /// Temporary: WarpDriver stores an absolute anchor position in its model state to detect
    /// stuck agents. Goes away once that anchor is kept as an accumulated displacement.
    Point position() const { return _agent.position; }

private:
    /// The segments as relative ones. Lazy range, no copies.
    auto Relative(CollisionGeometry::LineSegmentRange segments) const
    {
        return segments | std::views::transform([origin = _agent.position](const LineSegment& s) {
                   return LineSegment{s.p1 - origin, s.p2 - origin};
               });
    }

public:
    /// Wall segments in the grid cells around the agent, relative to it. The returned segments
    /// depend on the underlying grid cell size.
    /// Returned as lazy range.
    auto WallsNearby() const { return Relative(_world.LineSegmentsInRange(_agent.position)); }

    /// Wall segments within 'distance' of the agent, relative to it. Returns lazy range.
    auto WallsInRange(double distance) const
    {
        return Relative(_world.LineSegmentsInRange(_agent.position, distance));
    }

protected:
    const EnvironmentQuery& _world;
    const GenericAgent& _agent;
};

/// An AgentView plus what only holds for one step (dT + next target).
class AgentStep : public AgentView
{
public:
    AgentStep(const EnvironmentQuery& world, const GenericAgent& agent, double dt)
        : AgentView(world, agent), _dt(dt)
    {
    }

    double dt() const { return _dt; }

    Point ToNextTarget() const { return _agent.nextTarget - _agent.position; }

private:
    double _dt;
};

// Both views are passed by reference and never deleted through a base pointer; keeping
// them non-polymorphic keeps them free of a vtable and fully inlinable.
static_assert(!std::is_polymorphic_v<AgentView>);
static_assert(!std::is_polymorphic_v<AgentStep>);
