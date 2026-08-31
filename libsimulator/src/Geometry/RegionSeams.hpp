// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Geometry/RegionSplit.hpp"
#include "LineSegment.hpp"

#include <cstddef>
#include <vector>

/// Mesh edges connecting 2 regions.
///
/// The assumption is that no seam edges are "connected". There might be several seams
/// (like several doors to a room), but the edges are not adjcent.
///
/// Note: Every seam edge is reported twice, once from each side, because the two sides answer
/// different questions: standing in @c region, this is the way to @c neighbor.
struct RegionSeam {
    /// The seam in the horizontal plane.
    LineSegment segment;

    /// The region this edge is a boundary of.
    std::size_t region;

    /// The region on the other side.
    std::size_t neighbor;
};

/// Collect the seams of @p mesh under the region assignment @p region
std::vector<RegionSeam> extract_region_seams(const SurfaceMesh& mesh, const RegionMap& region);
