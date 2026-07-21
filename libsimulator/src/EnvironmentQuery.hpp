// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"

#include <algorithm>
#include <concepts>
#include <iterator>
#include <ranges>
#include <vector>

class EnvironmentQuery
{
    const CollisionGeometry& _geometry;
    const NeighborhoodSearch<GenericAgent>& _nsearch;

public:
    EnvironmentQuery(
        const CollisionGeometry& geometry,
        const NeighborhoodSearch<GenericAgent>& nsearch)
        : _geometry(geometry), _nsearch(nsearch)
    {
    }

    struct AcceptAll {
        bool operator()(const GenericAgent&) const { return true; }
    };

    // Returns all agents within 'radius' of 'agent', excluding 'agent' itself.
    // An optional predicate 'include' further filters the result; it receives each
    // candidate agent and must return true to keep it. Example:
    //   query.AgentsInRange(self, r, query.VisibleFrom(self.position()))
    template <std::predicate<const GenericAgent&> Pred = AcceptAll>
    std::vector<GenericAgent>
    AgentsInRange(const GenericAgent& agent, double radius, Pred include = {}) const
    {
        auto neighbors = _nsearch.GetNeighboringAgents(agent.position(), radius);
        std::erase_if(neighbors, [&](const GenericAgent& candidate) {
            return candidate.id == agent.id || !include(candidate);
        });
        return neighbors;
    }

    // Returns a predicate that is true when 'from' has an unobstructed line of sight
    // to the candidate agent. Pass directly as the 'include' argument to AgentsInRange:
    //   query.AgentsInRange(self, r, query.VisibleFrom(self.position()))
    auto VisibleFrom(const Point& from) const
    {
        return [this, from](const GenericAgent& candidate) {
            return VisibilityBetween(from, candidate.position());
        };
    }

    bool VisibilityBetween(const Point& from, const Point& to) const
    {
        return !_geometry.IntersectsAny(LineSegment{from, to});
    }

    const std::vector<LineSegment>& LineSegmentsInRange(const Point& p) const
    {
        return _geometry.LineSegmentsInApproxDistanceTo(p);
    }

    CollisionGeometry::LineSegmentRange
    LineSegmentsInRange(const Point& p, const double distance) const
    {
        return _geometry.LineSegmentsInDistanceTo(distance, p);
    }

    bool IntersectsAny(const LineSegment& ls) const { return _geometry.IntersectsAny(ls); }

    bool InsideGeometry(const Point& p) const { return _geometry.InsideGeometry(p); }

    const CollisionGeometry& Geometry() const { return _geometry; }
};
