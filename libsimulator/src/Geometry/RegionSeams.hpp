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

/// Collect the seams of @p mesh under the region assignment @p region.
///
/// Unmerged on purpose for now: fusing collinear runs would have to keep runs with different
/// neighbours apart, and until there is a query that feels the segment count it would be
/// optimising blind.
std::vector<RegionSeam> extract_region_seams(const SurfaceMesh& mesh, const RegionMap& region);
