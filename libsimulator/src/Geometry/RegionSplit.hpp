// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <cstddef>

/// Face -> 0-based region id, stored as a property map on the mesh.
using RegionMap = SurfaceMesh::Property_map<SurfaceMesh::Face_index, std::size_t>;

/// Result of a region split: the per-face region ids plus the region count.
struct RegionSplit {
    RegionMap region; ///< face property "f:region" (also queryable on the mesh)
    std::size_t count; ///< number of regions produced
};

/// Partition the faces of @p mesh into maximal regions whose orthogonal
/// projection onto the x/y-plane is injective ("single-valued"): within a
/// region no two surface points share the same (x, y).
///
/// The split criterion is (x, y)-overlap of the surface with itself, NOT slope:
/// a straight ramp or a stair run stays one region even though it is tilted.
/// Only where the surface folds back over its own footprint (e.g. an upper
/// floor above a lower one) does a new region begin.
///
/// Expects a "well-behaved" triangulated 2-manifold (clean, consistently
/// oriented, walkable faces only). SurfaceMesh supports dynamic properties.
/// Adds/overwrites the "f:region" face property on the mesh and returns it
/// together with the region count.
RegionSplit split_into_regions(SurfaceMesh& mesh);
