// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "cfg_cgal.hpp"
#include "point.hpp"

#include <fmt/core.h>

#include <cstddef>
#include <optional>

class Geometry;

/// A point on the walkable surface: its (x, y) and the region it lies in. z is derived
/// from these and cached.
class Location
{
public:
    /// Horizontal position.
    Point xy() const { return _xy; }

    std::size_t region() const { return _region_id; }

    /// Cache of z coordinate.
    double z() const { return _z; }

    Point3D position_3d() const { return Point3D{_xy.x, _xy.y, _z}; }

    /// Move on surface along the provided horizontal @p xy_direction.
    /// Throws if the path hits a wall. On throw the location is left unchanged.
    void move_on_surface(Point xy_direction);

    /// Feasibility variant of `move_on_surface`: walk along @p xy_direction and
    /// return the resulting Location, or `nullopt` if the straight path leaves
    /// the walkable area. Does not modify `*this`.
    std::optional<Location> try_move_on_surface(Point xy_direction) const;

    /// Whether walking straight from here arrives where @p other stands.
    bool can_walk_straight_to(const Location& other) const;

private:
    // Location uses private constructor. Only Geometry can create Location objects.
    friend class Geometry;

    Location(
        const Geometry* geometry,
        Point xy,
        std::size_t region_id,
        SurfaceMesh::Face_index face,
        double z)
        : _geometry(geometry), _xy(xy), _region_id(region_id), _face(face), _z(z)
    {
    }

    const Geometry* _geometry;
    Point _xy;
    std::size_t _region_id;
    SurfaceMesh::Face_index _face; // cache; always valid (move throws before invalidating)
    double _z; // cache
};

template <>
struct fmt::formatter<Location> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Location& l, FormatContext& ctx) const
    {
        return fmt::format_to(
            ctx.out(), "({}, {}, {}, region={})", l.xy().x, l.xy().y, l.z(), l.region());
    }
};
