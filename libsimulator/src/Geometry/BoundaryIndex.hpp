// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/Location.hpp"
#include "Geometry/RegionSplit.hpp"
#include "LineSegment.hpp"
#include "SegmentGrid.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <memory>
#include <vector>

//==================================================================================================
// BoundaryIndex Interface
//==================================================================================================
/// BoundaryIndex Interface.
/// Spatial index over boundary line segments.
class BoundaryIndex
{
public:
    BoundaryIndex() = default;
    virtual ~BoundaryIndex() = default;
    BoundaryIndex(const BoundaryIndex&) = delete;
    BoundaryIndex(BoundaryIndex&&) = delete;
    void operator=(const BoundaryIndex&) = delete;
    void operator=(BoundaryIndex&&) = delete;

    /// The boundary segments @p loc has to be told about, within @p maximum_distance of it.
    ///
    /// PortalBoundaryIndex answers with sight taken into account: a segment belongs to the
    /// answer iff a straight sight line in the (x, y) frame runs from `loc.xy()` to some point
    /// of it, crossing no wall on the way and never growing longer than @p maximum_distance.
    /// Sight lines leave the region the agent stands in through seams -- the portals between
    /// regions -- so what comes back is what can be seen across the surface and not merely
    /// what happens to lie nearby in plan: a floor folded back overhead is centimetres away in
    /// projection and out of sight. What is returned are the visible pieces themselves, each a
    /// maximal stretch of a wall the sight lines reach, clipped to the radius; a wall hidden
    /// in the middle comes back as two pieces, a wall no sight line reaches not at all, and no
    /// piece appears twice. A wall reachable only through a zero-width slit -- a sight line
    /// grazing a corner, where the seen piece has zero length -- may be omitted: such walls
    /// are a set of measure zero, and there is nothing there to keep an agent away from.
    ///
    /// NaiveBoundaryIndex keeps its naive semantics instead: all walls of the agent's own
    /// region within range, whole and unoccluded.
    ///
    /// @precondition Every region of the split projects injectively onto (x, y), so that the
    /// segments of one region meet only at their endpoints. split_into_regions() guarantees
    /// this; a region map produced by any other means must satisfy it too, or what is answered
    /// at coincident geometry is best effort.
    virtual std::vector<LineSegment> Query(const Location& loc, double maximum_distance) = 0;
};

//==================================================================================================
// NaiveBoundaryIndex
//==================================================================================================
/// NaiveBoundaryIndex
/// Most naive implementation:
/// - Returns only linesegments from current region
/// - Does not remove occluded LineSegments
class NaiveBoundaryIndex final : public BoundaryIndex
{
private:
    std::vector<SegmentGrid<>> regions;

public:
    explicit NaiveBoundaryIndex(std::vector<SegmentGrid<>> regions);
    std::vector<LineSegment> Query(const Location& loc, double maximum_distance) override;
};

//==================================================================================================
// PortalBoundaryIndex
//==================================================================================================
/// The regions of the surface and how they hang together: a vertex carries the walls of one
/// region, an edge the seam leading into a neighbour. Seams are stored once per direction.
using RegionGraph = boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::directedS,
    std::unique_ptr<SegmentGrid<>>,
    LineSegment>;

/// Portal visibility computed by recursion over (region, window) pairs, with CGAL's exact 2D
/// visibility machinery doing the seeing inside each region.
///
/// The agent's own region is asked first: an arrangement of its walls and seams, all opaque,
/// and the visibility region of the agent's point in it. Pieces of that region's boundary
/// lying on walls are answer; pieces lying on seams are windows -- the exact stretch of the
/// seam the agent can see -- and each window recurses into the region behind it, with the
/// window cut out of the seam so that sight passes where sight was shown to pass and nowhere
/// else. Windows only ever open away from the agent, so the query radius bounds the recursion.
class PortalBoundaryIndex final : public BoundaryIndex
{
private:
    std::unique_ptr<RegionGraph> g{};

public:
    explicit PortalBoundaryIndex(std::unique_ptr<RegionGraph> graph);

    std::vector<LineSegment> Query(const Location& loc, double maximum_distance) override;
};

//==================================================================================================
// Factories
//==================================================================================================
/// @precondition mesh has been split into region.
std::unique_ptr<BoundaryIndex>
MakeNaiveBoundaryIndex(const SurfaceMesh& mesh, const RegionSplit& region_split);

/// @precondition mesh has been split into region.
std::unique_ptr<BoundaryIndex>
MakePortalBoundaryIndex(const SurfaceMesh& mesh, const RegionSplit& region_split);

//==================================================================================================
// Supporting code
//==================================================================================================
std::tuple<SurfaceMesh::Face_index, SurfaceMesh::Halfedge_index>
IncidentFaceAndHalfedge(const SurfaceMesh& mesh, SurfaceMesh::Edge_index e);

std::vector<SegmentGrid<>>
CreatePerRegionSegmentGrids(const SurfaceMesh& mesh, const RegionSplit& region_split);

std::unique_ptr<RegionGraph>
CreateRegionGraph(const SurfaceMesh& mesh, const RegionSplit& region_split);
