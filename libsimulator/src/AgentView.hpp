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
    Point RelativePosition;
    const OperationalModelState* state;

private:
    /// Where they are actually standing, which is none of a model's business. It rides along
    /// because telling whether they can be seen means telling which sheet of the surface they
    /// are on, and only the region in here says that.
    friend class AgentView;
    NeighborView(Point relative, const OperationalModelState* model, const Location* where)
        : RelativePosition(relative), state(model), _location(where)
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

    /// All agents within 'radius', excluding this agent.
    template <std::predicate<const NeighborView&> Pred = AcceptAllNeighbors>
    std::vector<NeighborView> OtherAgentsInRange(double radius, Pred filter = {}) const
    {
        std::vector<NeighborView> neighbors{};
        // The grid searches by (x, y), so on a mesh it offers up whoever stands on the storey
        // above as readily as whoever stands next to you. Someone that far above cannot be
        // touched, so they are not a neighbour.
        //
        // This says nothing about walls: collecting candidates never did, in 2D either. A
        // model that wants line of sight passes 'filter' for it.
        const double z = location().z();
        _world.ForEachAgentInRange(_agent.Position(), radius, [&](const GenericAgent& candidate) {
            if(candidate.id == _agent.id) {
                return;
            }
            if(std::abs(candidate.location->z() - z) > InteractionHeight) {
                return;
            }
            const NeighborView neighbor{
                candidate.Position() - _agent.Position(), &candidate.state, &*candidate.location};
            if(filter(neighbor)) {
                neighbors.push_back(neighbor);
            }
        });
        return neighbors;
    }

    /// Whether the straight line to a point at 'RelativePosition' is free of geometry.
    /// Also the answer to whether the agent can move there: on a surface, what blocks the
    /// view blocks the step.
    bool NoGeometryBetween(Point RelativePosition) const
    {
        return _world.NoGeometryBetween(location(), RelativePosition);
    }

    /// Whether 'neighbor' can be seen from here. Same question as above plus the one a
    /// direction cannot carry: whether the way there leads to the surface they are standing
    /// on, rather than to one stacked over or under it.
    bool NoGeometryBetween(const NeighborView& neighbor) const
    {
        return _world.NoGeometryBetween(location(), *neighbor._location);
    }

private:
    /// The segments as seen from the agent. Lazy range, no copies.
    /// Must stay above WallsNearby() and WallsInRange() in code: an 'auto' return type is
    /// deduced from the body, so unlike other members this one cannot be called before it is
    /// defined.
    auto AsSeenFromAgent(WallRange segments) const
    {
        return segments | std::views::transform([origin = _agent.Position()](const LineSegment& s) {
                   const LineSegment segment{s.p1 - origin, s.p2 - origin};
                   const Point closest_point = segment.ShortestPoint(Point{});
                   return WallView{
                       .segment = segment,
                       .closest_point = closest_point,
                       .distance = closest_point.Norm(),
                       .normal = (Point{} - closest_point).Normalized()};
               });
    }

public:
    /// Walls in the grid cells around the agent. Which ones are returned depends on the
    /// underlying grid cell size.
    /// Returned as lazy range.
    auto WallsNearby() const { return AsSeenFromAgent(_world.LineSegmentsInRange(location())); }

    /// Wall segments within 'distance' of the agent, relative to it. Returns lazy range.
    auto WallsInRange(double distance) const
    {
        return AsSeenFromAgent(_world.LineSegmentsInRange(location(), distance));
    }

protected:
    /// 3D location wrapper of the agent's position on surface.
    const Location& location() const { return *_agent.location; }

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

    /// Normalized 2D vector pointing at the next target. Zero when the agent has already
    /// reached it.
    Point orientation_to_next_target() const
    {
        return (_agent.nextTarget - _agent.Position()).Normalized();
    }

private:
    double _dt;
};

// Both views are passed by reference and never deleted through a base pointer; keeping
// them non-polymorphic keeps them free of a vtable and fully inlinable.
static_assert(!std::is_polymorphic_v<AgentView>);
static_assert(!std::is_polymorphic_v<AgentStep>);
