// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/RegionSplit.hpp"
#include "LineSegment.hpp"

#include <boost/container/small_vector.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

/// One wall as the models get to see it: Fuse collinear edges into a single segment.
///
/// Fusing matters because models accumulate repulsion per segment -- the same straight wall
/// chopped into N pieces would push N times as hard, making the force depend on mesh
/// resolution.
struct MergedWall {
    /// The wall in the horizontal plane.
    LineSegment segment;

    /// Position of this wall in the geometry's wall list. Two entrances into the same
    /// region can deliver the same wall twice, index can detect and deduplicate.
    std::uint32_t index;

    /// Every region containing this wall. Typically just one.
    boost::container::small_vector<std::size_t, 2> regions;
};

/// Lets `SegmentGrid` index a wall by its geometry while still handing the whole wall back,
/// identity included.
inline const LineSegment& segment_of(const MergedWall& wall)
{
    return wall.segment;
}

/// Fuse the mesh's border edges into walls, walking the border cycles of @p mesh. A run ends
/// where the border bends by more than @p eps or where it doubles back on itself.
///
/// Runs on the mesh rather than on projected segments: there the borders of two floors are
/// separate cycles, while projected they are the same line. A region boundary does not break,
/// so a long wall crossing one stays a single wall.
///
/// Collinearity is judged in 2D - models repel horizontally, and judging in 3D would break a
/// "wavy" terrain outline at every mesh edge.
std::vector<MergedWall>
merge_border_walls(const SurfaceMesh& mesh, const RegionMap& region, double eps);

/// Tolerance for `merge_border_walls`, scaled to the mesh extent: enough to absorb the
/// floating-point noise in coordinates that are meant to be collinear.
double wall_merge_tolerance(const SurfaceMesh& mesh);
