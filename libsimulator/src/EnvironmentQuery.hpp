// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "GenericAgent.hpp"
#include "Geometry/Geometry3D.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/WallRange.hpp"
#include "LineSegment.hpp"
#include "NeighborhoodSearch.hpp"
#include "Point.hpp"
#include "SimulationError.hpp"

#include <concepts>
#include <vector>

class EnvironmentQuery
{
    const Geometry3D& _geometry;
    const NeighborhoodSearch<GenericAgent>& _nsearch;

public:
    EnvironmentQuery(const Geometry3D& geometry, const NeighborhoodSearch<GenericAgent>& nsearch)
        : _geometry(geometry), _nsearch(nsearch)
    {
    }

    struct AcceptAll {
        bool operator()(const GenericAgent&) const { return true; }
    };

    /// Calls 'fn' for every agent within 'radius' of 'from'.
    /// Note: No z filtering is applied.
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

    bool NoGeometryBetween(const Location& who, Point direction) const
    {
        return _geometry.no_geometry_between(who, direction);
    }

    WallRange LineSegmentsInRange(const Location& who, double distance = -1.0) const
    {
        return _geometry.line_segments_in_range(who, distance);
    }

    bool InsideGeometry(const Location& who, Point direction) const
    {
        return _geometry.inside_geometry(who, direction);
    }

    /// Interim: stage slots still used this. To be removed later.
    bool NoGeometryBetween(const Point& from, const Point& to) const
    {
        const auto* flat = _geometry.geometry_2d();
        if(flat == nullptr) {
            throw SimulationError("Point-based visibility needs a stage Location on the mesh.");
        }
        return !flat->IntersectsAny(LineSegment{from, to});
    }
};
