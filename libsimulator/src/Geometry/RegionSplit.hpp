// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <cstddef>

/// Face -> 0-based region id, indexed by the face index.
using RegionMap = std::vector<std::size_t>;

/// Result of a region split: the per-face region ids plus the region count.
struct RegionSplit {
    RegionMap region; ///< region id per face, indexed by face index
    std::size_t count; ///< number of regions produced
};

/// Partition the faces of @p mesh into regions that are closed-injective under
/// orthogonal projection onto the x/y-plane: two faces of one region share an
/// (x, y) point only where they share mesh elements -- a shared mesh edge may
/// coincide along that edge, a shared mesh vertex at that vertex. Any other
/// coincidence starts a new region: a positive-area overlap (the surface folds
/// back over its own footprint), a collinear segment overlap not backed by a
/// shared edge (e.g. a stair edge over a wall of the floor it climbs from), or
/// an isolated point contact between distinct surface points.
///
/// Equivalently (slopes are <= 50deg and faces consistently oriented, so
/// projection is orientation-preserving per face): each region's projected
/// boundary -- walls and seams together -- forms strictly simple loops. The
/// boundary of a region may not even touch itself: two lobes meeting at a
/// genuinely shared vertex stay separate regions.
///
/// The split criterion is coincidence, NOT slope: a straight ramp or a stair
/// run stays one region even though it is tilted. Grown planar-first and merged
/// bottom-up by height, the result is deterministic: it depends only on the
/// mesh's vertex/face order, never on growth order or platform.
///
/// Expects a "well-behaved" triangulated 2-manifold (clean, consistently
/// oriented, walkable faces only, no garbage). Note that unwelded duplicate
/// vertices (identical coordinates, distinct vertex indices) count as distinct
/// surface points: contacts across them split, and since the faces are not
/// mesh-adjacent no seam will connect the parts -- weld the mesh first.
/// The mesh is not modified.
RegionSplit split_into_regions(const SurfaceMesh& mesh);
