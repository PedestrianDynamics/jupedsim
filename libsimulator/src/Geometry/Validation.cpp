// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Validation.hpp"

#include "SimulationError.hpp"

#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

bool IsWalkableNormal(const Vector3& n)
{
    constexpr double max_incline_deg = 50.0;
    constexpr double max_incline_rad = max_incline_deg * std::numbers::pi / 180.0;
    // @TODO(kkratz): Replace with constexpr when moving to C++23
    static const double min_z = std::cos(max_incline_rad);
    return n.z() >= min_z;
}

bool IsFaceInMeshPlanar(
    const CGALMesh& mesh,
    FaceDescriptor<CGALMesh> face,
    std::vector<VertexDescriptor<CGALMesh>>& buffer)
{
    buffer.clear();
    for(auto half_edge : CGAL::halfedges_around_face(CGAL::halfedge(face, mesh), mesh)) {
        buffer.emplace_back(CGAL::source(half_edge, mesh));
    }

    if(buffer.size() < 3) {
        return false;
    }

    const auto& p0 = mesh.point(buffer[0]);
    const auto& p1 = mesh.point(buffer[1]);
    const auto& p2 = mesh.point(buffer[2]);

    if(CGAL::collinear(p0, p1, p2)) {
        return false;
    }

    return std::all_of(std::begin(buffer), std::end(buffer), [&p0, &p1, &p2, &mesh](auto v) {
        return CGAL::coplanar(p0, p1, p2, mesh.point(v));
    });
}

bool AllFacesInMeshPlanar(const CGALMesh& mesh)
{
    std::vector<VertexDescriptor<CGALMesh>> buffer{};
    buffer.reserve(3);
    return std::all_of(
        std::begin(CGAL::faces(mesh)), std::end(CGAL::faces(mesh)), [&buffer, &mesh](auto face) {
            return IsFaceInMeshPlanar(mesh, face, buffer);
        });
}

void NormaliseAndValidateMesh(CGALMesh& mesh)
{
    namespace PMP = CGAL::Polygon_mesh_processing;
    auto fcc = mesh.add_property_map<CGALMesh::Face_index, size_t>("f:cc").first;
    const auto count = PMP::connected_components(mesh, fcc);
    mesh.remove_property_map(fcc);
    if(count != 1) {
        throw SimulationError("Expected exactly 1 connected component, got: {}", count);
    }

    if(!AllFacesInMeshPlanar(mesh)) {
        throw SimulationError("Not all faces are planar.");
    }

    const auto n = PMP::compute_face_normal(*std::begin(CGAL::faces(mesh)), mesh);
    if(!IsWalkableNormal(n)) {
        PMP::reverse_face_orientations(mesh);
    }

    for(auto&& face : CGAL::faces(mesh)) {
        const auto n = PMP::compute_face_normal(face, mesh);
        if(!IsWalkableNormal(n)) {
            throw SimulationError("Face {} inclination exceeds 50deg.", face.idx());
        }
    }
}
