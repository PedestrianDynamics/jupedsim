// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"
#include "Point.hpp"

#include <cstddef>
#include <vector>

/// Cut a polyline into the longest straight runs it will take.
///
/// Returns the index of the first point of each run, always starting with 0. The last run
/// ends at the polyline's last point, so a return of {0, 4} over seven points means two runs,
/// points 0..4 and 4..6 -- consecutive runs share the corner between them rather than leaving
/// a gap.
///
/// A run keeps growing while it does not turn back on itself and every point it swallowed
/// stays within @p eps of the run's own chord. The chord, so that the deviation of the whole
/// run is bounded rather than only that of neighbouring pairs.
///
/// This is "anchor and float" in the spirit of Lang (1969): anchor at the start, push the far
/// end outwards, cut back one step at the first violation. Deliberately not Douglas-Peucker,
/// which recurses on the largest deviation and gives the same answer whichever way the line
/// is walked -- here the walk direction is the input, and the geometry it simplifies is a
/// boundary that is always traversed the same way round.
std::vector<std::size_t> straight_runs(const std::vector<Point>& points, double eps);

/// The tolerance both mesh merges run at, scaled to the extent of @p mesh: enough to absorb
/// the floating-point noise in coordinates that are meant to be collinear, and far below any
/// corner a real geometry has.
double mesh_merge_tolerance(const SurfaceMesh& mesh);
