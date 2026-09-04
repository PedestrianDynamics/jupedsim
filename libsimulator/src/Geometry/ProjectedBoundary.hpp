// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <functional>
#include <vector>

/// @file ProjectedBoundary.hpp
/// Boundary extraction for face subsets and the strict simplicity certificate their
/// X/Y-projection must pass to guarantee an injective projection.

/// Boundary halfedges of the face set {f : keep(f)}: halfedges h with keep(face(h)) whose
/// opposite halfedge lies on the mesh border or in a face with !keep(face(opposite(h))).
/// Deterministic order: faces ascending by index, halfedges in face-circulation order.
std::vector<SurfaceMesh::Halfedge_index>
region_boundary(const SurfaceMesh& mesh, const std::function<bool(SurfaceMesh::Face_index)>& keep);

/// Strict Lipman certificate: true iff the X/Y-projection of @p boundary consists of
/// strictly simple loops. No two boundary segments may intersect, with one exception: two
/// segments sharing a mesh vertex may meet exactly at that vertex's projection. A repeated
/// boundary vertex (a pinch), a zero-angle spike, a collinear fold-back of consecutive
/// segments, and any contact between segments not sharing a mesh vertex are violations.
///
/// For a connected face set whose faces all project with positive orientation (walkable
/// meshes: every face normal has z > 0), a strictly simple projected boundary is
/// equivalent to the projection of the whole face set being injective.
bool is_projection_strictly_simple(
    const SurfaceMesh& mesh,
    const std::vector<SurfaceMesh::Halfedge_index>& boundary);
