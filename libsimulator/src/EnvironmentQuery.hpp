// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CollisionGeometry.hpp"
#include "GenericAgent.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"

#include <algorithm>
#include <concepts>
#include <iterator>
#include <ranges>
#include <vector>

using OperationalModelState = GenericAgent::ModelState;

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

    // Returns all agents within 'radius' of 'agent', excluding 'agent' itself.
    // An optional predicate 'filter' further filters the result; it receives the
    // position for which neighbors are returned as well as the candidates. Example:
    //   query.AgentsInRange(state, r, [&envQuery, from=model.position](const Point& to) {
    //   return envQuery.NoGeometryBetween(from, to);})
    template <std::predicate<const Point&> Pred = AcceptAll>
    std::vector<GenericAgent>
    OtherAgentsInRange(const OperationalModelState& state, double radius, Pred filter = {}) const
    {
        Point from = Pos(state);
        return AgentsInRange(from, radius, [&from, &filter](const Point& to) {
            return !(from == to || !filter(to));
        });
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
