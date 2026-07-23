// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Point.hpp"

#include <cstddef>

class Geometry3D;

/// General `Location` interface. The "truth" is 2D + region, but it provides
/// a cached z-coordinate.
class Location
{
public:
    /// "horizonal" part. Common between 3D and 2D+regin views.
    Point xy() const { return _xy; }

    std::size_t region() const { return _regionId; }

    /// Cache of z coordinate.
    double z() const { return _z; }

    Point3D position_3d() const { return Point3D{_xy.x, _xy.y, _z}; }

    /// Move on surface along the provided horizontal @p xy_direction.
    /// Throws if the path leaves the walkable area. On throw the
    /// location is left unchanged.
    void move_on_surface(Point xy_direction);

private:
    // Location uses private constructor. Only Geometry3D can create Location objects.
    friend class Geometry3D;

    Location(
        const Geometry3D* geometry,
        Point xy,
        std::size_t regionId,
        SurfaceMesh::Face_index face,
        double z)
        : _geometry(geometry), _xy(xy), _regionId(regionId), _face(face), _z(z)
    {
    }

    const Geometry3D* _geometry;
    Point _xy;
    std::size_t _regionId;
    SurfaceMesh::Face_index _face; // cache; always valid (move throws before invalidating)
    double _z; // cache
};
