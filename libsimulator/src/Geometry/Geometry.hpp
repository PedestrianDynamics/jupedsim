// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/BoundaryIndex.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/RegionSplit.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

/// Default z-hint tolerance: When a `Location` is created, how far the z-value
/// is allowed to be away from the surface to still be accepted.
inline constexpr double ZHintTolerance = 0.1;

/// Height difference above which two people cannot touch each other. This is used
/// as a quick pre-filter.
inline constexpr double InteractionHeight = 2.0;

/// The 3D geometry
class Geometry
{
public:
    /// Result of projecting a query point onto the surface along -z.
    struct FaceLocation {
        SurfaceMesh::Face_index face;
        K::Point_3 point;
    };

    /// Take an already-built surface mesh (e.g. from a mesh builder or a test).
    explicit Geometry(SurfaceMesh mesh);

    /// Build from a 2D walkable area, lifted flat to z=0 by constrained Delaunay
    /// triangulation. Keeps the polygon it was lifted from (see polygon()).
    explicit Geometry(PolyWithHoles poly);

    ~Geometry() = default;

    // Non-copyable and non-movable: Any instance should be held by unique_ptr
    // to ensure exposed addresses do not move.
    Geometry(const Geometry&) = delete;
    Geometry& operator=(const Geometry&) = delete;
    Geometry(Geometry&&) = delete;
    Geometry& operator=(Geometry&&) = delete;

    const SurfaceMesh& mesh() const { return _mesh; }
    const AABBTree& aabb_tree() const { return *_aabbTree; }

    /// Returns the polygon iff the geometry was built from one. Otherwise returns
    /// nullptr.
    const PolyWithHoles* polygon() const { return _polygon ? &*_polygon : nullptr; }

    /// Face and on-surface point hit by the -z ray through @p p, or
    /// `null_face()` if the ray misses the walkable surface.
    FaceLocation face_below(const Point3D& p) const;

    /// Locate @p xy within region @p region_id: the face of that region whose
    /// (x,y)-projection contains @p xy, and the on-surface point (its z on that
    /// face's plane). `null_face()` if @p xy is outside the region's footprint.
    FaceLocation locate_in_region(std::size_t region_id, const Point2D& xy) const;

    /// Locate @p xy on the mesh face whose surface z is nearest to @p z: among all
    /// faces stacked over @p xy pick the one whose on-surface z deviates least
    /// from the provided z. `null_face()` if no mesh face comes within @p tolerance.
    FaceLocation locate_near_z(const Point2D& xy, double z, double tolerance) const;

    /// Creates a `Location` object by ray-casting the 3D point in z-direction and
    /// finding the closest point to hit any part of the 3D surface. If there is no
    /// such point within @p tol on z coordinate, returns no value.
    std::optional<Location>
    get_location(double x, double y, double z_hint, double tol = ZHintTolerance) const;

    /// True iff @p p projects (along -z) onto the walkable surface.
    bool is_valid_location(const Point3D& p) const;

    // -- `EnvironmentQuery` API -----------------------------------------------

    /// The wall segments within @p distance of @p who that @p who can see, each clipped to
    /// that distance. Sight lines crosses regions seams.
    std::vector<LineSegment> line_segments_in_range(const Location& who, double distance) const;

    /// True iff the straight horizontal step @p direction, taken from @p who, crosses no
    /// wall and does not run off the surface.
    bool no_geometry_between(const Location& who, Point direction) const;

    /// True iff @p who can see @p other: the way there has to be clear, and it has to lead
    /// to the mesh face @p other is standing on.
    bool no_geometry_between(const Location& who, const Location& other) const;

    // -- region related API ---------------------------------------------------

    std::size_t region_count() const { return _regionCount; }

    /// Region id (0-based) of a single face, as assigned by the region overlay.
    std::size_t region_of(SurfaceMesh::Face_index face) const { return _region[face]; }

    /// One 0-based region id per triangle, in mesh face order.
    std::vector<std::size_t> region_id_per_face() const;

    const RegionSplit& region_split() const { return _regionSplit; }

    // -- Viewer API -----------------------------------------------------------

    /// Vertex coordinates (x, y, z), indexable 0..n-1.
    std::vector<std::array<double, 3>> vertices() const;

    /// Triangles as vertex-index triples, matching region_id_per_face() order.
    std::vector<std::array<std::size_t, 3>> triangles() const;

private:
    /// Create internal structures like building the AABB tree and the region overlay.
    void build();

    /// The region a straight horizontal step from @p who along @p direction ends up in, or
    /// nothing when a wall stops it or it runs off the surface.
    std::optional<std::size_t> region_reached(const Location& who, Point direction) const;

    SurfaceMesh _mesh{};
    std::optional<PolyWithHoles> _polygon{};
    std::unique_ptr<AABBTree> _aabbTree{};
    std::unique_ptr<BoundaryIndex> _boundaryIndex{};
    std::unique_ptr<RegionGraph> _regionGraph{};
    RegionMap _region{};
    std::size_t _regionCount{0};
    RegionSplit _regionSplit{};
};
