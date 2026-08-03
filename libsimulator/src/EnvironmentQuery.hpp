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

    /// Calls 'fn' for every agent within 'radius' of 'from'.
    template <std::invocable<const GenericAgent&> Fn>
    void ForEachAgentInRange(const Point& from, double radius, Fn fn) const
    {
        _nsearch.ForEachInRange(from, radius, fn);
    }

    template <std::predicate<const GenericAgent&> Pred = AcceptAll>
    std::vector<GenericAgent>
    AgentsInRange(const Point& from, double radius, Pred filter = {}) const
    {
        std::vector<GenericAgent> neighbors{};
        ForEachAgentInRange(from, radius, [&](const GenericAgent& candidate) {
            if(filter(candidate)) {
                neighbors.push_back(candidate);
            }
        });
        return neighbors;
    }

    template <typename Range>
    bool NoGeometryBetween(const Point& from, const Point& to, const Range& boundaries) const
    {
        const LineSegment los{from, to};
        return !std::any_of(std::begin(boundaries), std::end(boundaries), [&los](const auto& seg) {
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
