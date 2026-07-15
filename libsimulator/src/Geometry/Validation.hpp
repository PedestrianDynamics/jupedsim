// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <vector>

/// @file Validation.hpp
/// Contains support code to validate inputs or intermediate results.

/// Test if the supplied normal is inclined by 50deg or less over the X/Y plane.
/// @param n normal vector of the plane to test for walkability
/// @pre #n needs to be normalized; otherwise the result is not reliable.
/// @return true if the normal indicates that the plane is walkable.
bool IsWalkableNormal(const Vector3D& n);

/// Test if face in mesh is planar.
/// Degenerate faces (2 or fewer vertices, or all vertices collinear) will return false.
/// @param mesh with face to test
/// @param face to test
/// @param buffer to use for temporary internal storage. Supply this when calling
///        IsFaceInMeshPlanar repeatedly to reduce allocations.
/// @return true if face is planar
bool IsFaceInMeshPlanar(
    const SurfaceMesh& mesh,
    FaceDescriptor<SurfaceMesh> face,
    std::vector<VertexDescriptor<SurfaceMesh>>& buffer);

/// Test if all faces in the given #mesh are planar.
/// @param mesh to test.
/// @return true if all faces are planar
bool AllFacesInMeshPlanar(const SurfaceMesh& mesh);

/// Validate that the given mesh can be processed by jupedsim.
/// Ensures the mesh is a single connected component, has no degenerate faces, all faces are
/// planar, and no incline exceeds 50deg. If the surface normals point downwards, all face
/// orientations are reversed, i.e. the mesh may be modified.
/// @param mesh to check
/// @throws SimulationError with a description of the issue encountered.
void NormaliseAndValidateMesh(SurfaceMesh& mesh);
