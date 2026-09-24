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

class WalkableSurface;

/// Default tolerance of get_location_near_z: how far the surface may lie from the given z.
inline constexpr double NearZTolerance = 0.1;

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

    /// Seam as edge:
    /// - ring 0 is boundary, ring k is hole k-1.
    /// - edge goes from vertex index to successor.
    /// - source region is always left of seam.
    struct SeamEdge {
        size_t ring;
        size_t index;
    };

    // Note: There is always at most 1 connection between 2 regions as regions are connected
    //       via Connectors which add a region of their own.
    using RegionGraph2D =
        boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, PolyWithHoles, SeamEdge>;

    /// 3D mesh. Perform auto-split into regions.
    explicit Geometry(SurfaceMesh mesh);
    /// Special constructor for WalkableSurface: Ensures consistency of parameters.
    explicit Geometry(
        SurfaceMesh&& mesh,
        RegionSplit&& regionSplit,
        std::unique_ptr<RegionGraph2D> regionGraph2d);

    ~Geometry() = default;

    // Non-copyable and non-movable: Any instance should be held by unique_ptr
    // to ensure exposed addresses do not move.
    Geometry(const Geometry&) = delete;
    Geometry& operator=(const Geometry&) = delete;
    Geometry(Geometry&&) = delete;
    Geometry& operator=(Geometry&&) = delete;

    const SurfaceMesh& mesh() const { return _mesh; }
    const AABBTree& aabb_tree() const { return *_aabbTree; }

    /// Returns the 2D polygon of the specified region. Throws in case of error.
    PolyWithHoles polygon(size_t region_id) const;

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

    /// The place at (@p x, @p y) in region @p region_id.
    /// Without @p region_id, searches for the region containing (@p x, @p y). On a seam,
    /// takes the lowest id of the regions meeting there.
    /// Throws if (@p x, @p y) is not on the walkable surface (not in region @p region_id, if
    /// given), if @p region_id does not exist, or if region-id is not specified and several
    /// regions lie on top of each other at (@p x, @p y).
    Location
    get_location(double x, double y, std::optional<std::size_t> region_id = std::nullopt) const;

    /// The place at (@p x, @p y) on the surface closest to height @p z, if one comes within
    /// @p tol.
    std::optional<Location>
    get_location_near_z(double x, double y, double z, double tol = NearZTolerance) const;

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

    std::size_t region_count() const { return _regionSplit.count; }

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

    /// Every face the vertical line through @p xy crosses, with its on-surface point, ordered
    /// by region id, then by face index.
    std::vector<FaceLocation> faces_at(const Point2D& xy) const;

    Location location_at(Point xy, const FaceLocation& where) const;

    /// The region a straight horizontal step from @p who along @p direction ends up in, or
    /// nothing when a wall stops it or it runs off the surface.
    std::optional<std::size_t> region_reached(const Location& who, Point direction) const;

    SurfaceMesh _mesh{};
    std::unique_ptr<AABBTree> _aabbTree{};
    std::unique_ptr<BoundaryIndex> _boundaryIndex{};
    std::unique_ptr<RegionGraph> _regionGraph{};
    std::unique_ptr<RegionGraph2D> _regionGraph2D{};
    RegionMap _region{};
    RegionSplit _regionSplit{};
};
