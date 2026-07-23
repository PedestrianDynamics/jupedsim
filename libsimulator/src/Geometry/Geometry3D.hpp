// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/Geometry2D.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/RegionSplit.hpp"
#include "Point.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

/// Default z-hint tolerance: When a `Location` is created, how far the z-value
/// is allowed to be away from the surface to still be accepted.
inline constexpr double ZHintTolerance = 0.1;

/// The single source of truth for a 3D navigation geometry: owns the surface
/// mesh, its AABB tree (for -z projection queries) and the single-valued region
/// overlay. Routing engines borrow it (non-owning), and the viewer reads its
/// render data from here -- so mesh, routing and colouring all agree by
/// construction (one load, one face order).
class Geometry3D
{
public:
    /// Result of projecting a query point onto the surface along -z.
    struct FaceLocation {
        SurfaceMesh::Face_index face;
        K::Point_3 point;
    };

    /// Take an already-built surface mesh (e.g. from a mesh builder or a test).
    explicit Geometry3D(SurfaceMesh mesh);

    /// Build from a 2D walkable area, lifted flat to z=0. Uses the same
    /// constrained Delaunay triangulation as the 2D RoutingEngine, so the 2D
    /// and 3D pipelines run on the identical triangle set -- the basis for
    /// exact parity comparisons. Also keeps the 2D view (geometry_2d()).
    explicit Geometry3D(PolyWithHoles poly);

    ~Geometry3D() = default;

    // Non-copyable and non-movable: Any instance should be held by unique_ptr
    // to ensure exposed addresses do not move.
    Geometry3D(const Geometry3D&) = delete;
    Geometry3D& operator=(const Geometry3D&) = delete;
    Geometry3D(Geometry3D&&) = delete;
    Geometry3D& operator=(Geometry3D&&) = delete;

    const SurfaceMesh& mesh() const { return _mesh; }
    const AABBTree& aabb_tree() const;

    /// The projected 2D view of the walkable area, present iff the geometry
    /// was built from a polygon. A mesh-built geometry returns a nullptr.
    const Geometry2D* geometry_2d() const { return _geometry2D.get(); }

    /// Face and on-surface point hit by the -z ray through @p p, or
    /// `null_face()` if the ray misses the walkable surface.
    FaceLocation face_below(const Point3D& p) const;

    /// Locate @p xy within region @p region_id: the face of that region whose
    /// (x,y)-projection contains @p xy, and the on-surface point (its z on that
    /// face's plane). `null_face()` if @p xy is outside the region's footprint.
    FaceLocation locate_in_region(std::size_t region_id, const Point2D& xy) const;

    /// Locate @p xy on the sheet whose surface z is nearest to @p z: among all
    /// faces stacked over @p xy pick the one whose on-surface z deviates least
    /// from the hint. `null_face()` if no sheet comes within @p tolerance.
    FaceLocation locate_near_z(const Point2D& xy, double z, double tolerance) const;

    /// Creates a `Location` object by ray-casting the 3D point in z-direction and
    /// finding the closest point to hit any part of the 3D surface. Only accepts
    /// points at most @p tol away in terms of z-coordinate.
    std::optional<Location>
    get_location(double x, double y, double z_hint, double tol = ZHintTolerance) const;

    /// True iff @p p projects (along -z) onto the walkable surface.
    bool is_valid_location(const Point3D& p) const;

    // -- region overlay & render data (see split_into_regions) --------------

    std::size_t region_count() const { return _regionCount; }

    /// Region id (0-based) of a single face, as assigned by the region overlay.
    std::size_t region_of(SurfaceMesh::Face_index face) const { return _region[face]; }

    /// One 0-based region id per triangle, in mesh face order.
    std::vector<std::size_t> region_id_per_face() const;

    /// Vertex coordinates (x, y, z), indexable 0..n-1.
    std::vector<std::array<double, 3>> vertices() const;

    /// Triangles as vertex-index triples, matching region_id_per_face() order.
    std::vector<std::array<std::size_t, 3>> triangles() const;

private:
    /// Compact indices, build the AABB tree and the region overlay.
    void build();

    SurfaceMesh _mesh{};
    std::unique_ptr<Geometry2D> _geometry2D{};
    std::unique_ptr<AABBTree> _aabbTree{};
    RegionMap _region{};
    std::size_t _regionCount{0};
};
