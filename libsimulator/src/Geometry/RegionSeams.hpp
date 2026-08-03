// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/RegionSplit.hpp"
#include "LineSegment.hpp"

#include <cstddef>
#include <vector>

/// Where one region ends and the next begins: an interior mesh edge whose two sides belong
/// to different regions.
///
/// Seams are not walls. Nothing stops there -- the surface runs on, only the region id
/// changes, because further along it starts to lie over itself in (x, y). Models therefore
/// never see them; they exist so that a query starting in one region can find out what it
/// has to look at beyond.
///
/// Every seam edge is reported twice, once from each side, because the two sides answer
/// different questions: standing in @c region, this is the way to @c neighbor.
struct RegionSeam {
    /// The seam in the horizontal plane.
    LineSegment segment;

    /// The region this edge is a boundary of.
    std::size_t region;

    /// The region on the other side.
    std::size_t neighbor;
};

/// Collect the seams of @p mesh under the region assignment @p region, fusing collinear runs
/// the same way walls are fused: straight to within @p eps, and never across a bend or a
/// change of neighbour.
///
/// Merging is not cosmetic here. Every visibility query walks a region's seams to find out
/// whether its chord leaves the region, so the segment count is what that scan costs -- once
/// per neighbour per step. It is paid for once, when the geometry is built.
std::vector<RegionSeam>
extract_region_seams(const SurfaceMesh& mesh, const RegionMap& region, double eps);
