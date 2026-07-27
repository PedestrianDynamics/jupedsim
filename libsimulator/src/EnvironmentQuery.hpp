// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "OperationalModelState.hpp"
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
        bool operator()(const Point&) const { return true; }
    };

    // Returns the model states of all agents within 'radius' of 'state', excluding the agent
    // with the same position as 'state'. An optional predicate 'filter' further narrows
    // the result; it receives the candidate's position.
    // Example:
    //   envQuery.OtherAgentsInRange(state, r, [&envQuery, from=model.position](const Point& to) {
    //       return envQuery.NoGeometryBetween(from, to); })
    template <std::predicate<const Point&> Pred = AcceptAll>
    std::vector<OperationalModelState>
    OtherAgentsInRange(const OperationalModelState& state, double radius, Pred filter = {}) const
    {
        const Point from = Position(state);
        auto agents = AgentsInRange(
            from, radius, [&from, &filter](const Point& to) { return from != to && filter(to); });
        std::vector<OperationalModelState> states;
        states.reserve(agents.size());
        for(auto& a : agents) {
            states.push_back(std::move(a.state));
        }
        return states;
    }

    template <std::predicate<const Point&> Pred = AcceptAll>
    std::vector<GenericAgent>
    AgentsInRange(const Point& from, double radius, Pred filter = {}) const
    {
        auto neighbors = _nsearch.GetNeighboringAgents(from, radius);
        std::erase_if(neighbors, [&](const GenericAgent& candidate) {
            return !filter(candidate.position());
        });
        return neighbors;
    }

    bool NoGeometryBetween(const Point& from, const Point& to) const
    {
        const LineSegment los{from, to};
        const double dist = Distance(from, to);
        auto blocked = [&los](const auto& boundaries) {
            return std::any_of(boundaries.begin(), boundaries.end(), [&los](const auto& seg) {
                return intersects(los, seg);
            });
        };
        return !blocked(LineSegmentsInRange(from, dist));
    }

    CollisionGeometry::LineSegmentRange
    LineSegmentsInRange(const Point& p, double distance = -1.0) const
    {
        if(distance < 0.0) {
            return _geometry.LineSegmentsInApproxDistanceTo(p);
        } else {
            return _geometry.LineSegmentsInDistanceTo(distance, p);
        }
    }

    bool InsideGeometry(const Point& p) const { return _geometry.InsideGeometry(p); }

    const CollisionGeometry& Geometry() const { return _geometry; }
};
