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
#include <optional>
#include <ranges>
#include <vector>

using OperationalModelState = GenericAgent::ModelState;

class EnvironmentQuery
{
    const CollisionGeometry& _geometry;
    const NeighborhoodSearch<GenericAgent>& _nsearch;
    mutable std::optional<Point> _cached_boundary_pos{};
    mutable std::vector<LineSegment> _cached_boundary{};

    void _fetchBoundaryAt(const Point& from) const
    {
        _cached_boundary_pos = from;
        auto range = _geometry.LineSegmentsInApproxDistanceTo(from);
        _cached_boundary.assign(range.begin(), range.end());
    }

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
        const Point from = Pos(state);
        _fetchBoundaryAt(from);
        std::vector<GenericAgent> neighbors{};
        _nsearch.ForEachInRange(from, radius, [&](const GenericAgent& candidate) {
            if(candidate.position() != from && filter(candidate.position())) {
                neighbors.push_back(candidate);
            }
        });
        return neighbors;
    }

    template <std::predicate<const Point&> Pred = AcceptAll>
    std::vector<GenericAgent>
    AgentsInRange(const Point& from, double radius, Pred filter = {}) const
    {
        std::vector<GenericAgent> neighbors{};
        _nsearch.ForEachInRange(from, radius, [&](const GenericAgent& candidate) {
            if(filter(candidate.position())) {
                neighbors.push_back(candidate);
            }
        });
        return neighbors;
    }

    bool NoGeometryBetween(const Point& from, const Point& to) const
    {
        if(!_cached_boundary_pos || *_cached_boundary_pos != from) {
            _fetchBoundaryAt(from);
        }
        const LineSegment los{from, to};
        return !std::any_of(
            _cached_boundary.begin(), _cached_boundary.end(), [&los](const auto& seg) {
                return intersects(los, seg);
            });
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
