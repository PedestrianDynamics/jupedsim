// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry/Geometry.hpp"
#include "Geometry/Location.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <concepts>
#include <vector>

class EnvironmentQuery
{
    const Geometry& _geometry;
    const NeighborhoodSearch<GenericAgent>& _nsearch;

public:
    EnvironmentQuery(const Geometry& geometry, const NeighborhoodSearch<GenericAgent>& nsearch)
        : _geometry(geometry), _nsearch(nsearch)
    {
    }

    struct AcceptAll {
        bool operator()(const GenericAgent&) const { return true; }
    };

    /// Calls 'fn' for every agent within 'radius' of 'from'.
    /// Note: No z filtering is applied.
    template <std::invocable<const GenericAgent&> Fn>
    void for_each_agent_in_range(const Point& from, double radius, Fn fn) const
    {
        _nsearch.for_each_in_range(from, radius, fn);
    }

    template <std::predicate<const GenericAgent&> Pred = AcceptAll>
    std::vector<GenericAgent>
    agents_in_range(const Point& from, double radius, Pred filter = {}) const
    {
        std::vector<GenericAgent> neighbors{};
        for_each_agent_in_range(from, radius, [&](const GenericAgent& candidate) {
            if(filter(candidate)) {
                neighbors.push_back(candidate);
            }
        });
        return neighbors;
    }

    bool no_geometry_between(const Location& who, Point direction) const
    {
        return _geometry.no_geometry_between(who, direction);
    }

    bool no_geometry_between(const Location& who, const Location& other) const
    {
        return _geometry.no_geometry_between(who, other);
    }

    std::vector<LineSegment> line_segments_in_range(const Location& who, double distance) const
    {
        return _geometry.line_segments_in_range(who, distance);
    }
};
