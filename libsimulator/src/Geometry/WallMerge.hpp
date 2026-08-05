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

    /// How high the fused run reaches, lowest and highest z along it. A cheap pre-filter against
    /// the height an agent can touch: a wall too high above him is one of the flight above, a wall
    /// too far below one of the storey he is standing on top of. Both happen a few metres away in
    /// plan, in his own region as readily as in a neighbouring one -- a "U"-stair has them both.
    ///
    /// A run climbing a stair spans the whole climb, which is why it takes two values: it has to
    /// stay in play for whoever is at its foot and for whoever is at its head.
    double zMin;
    double zMax;

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
