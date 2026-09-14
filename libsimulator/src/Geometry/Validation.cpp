// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Validation.hpp"

#include "SimulationError.hpp"

#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/boost/graph/helpers.h>
#include <boost/property_map/property_map.hpp>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numbers>
#include <utility>
#include <vector>

bool IsWalkableNormal(const Vector3D& n)
{
    constexpr double max_incline_deg = 50.0;
    constexpr double max_incline_rad = max_incline_deg * std::numbers::pi / 180.0;
    // @TODO(kkratz): Replace with constexpr when moving to C++23
    static const double min_z = std::cos(max_incline_rad);
    return n.z() >= min_z;
}

bool IsFaceInMeshPlanar(
    const SurfaceMesh& mesh,
    FaceDescriptor<SurfaceMesh> face,
    std::vector<VertexDescriptor<SurfaceMesh>>& buffer)
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

bool AllFacesInMeshPlanar(const SurfaceMesh& mesh)
{
    std::vector<VertexDescriptor<SurfaceMesh>> buffer{};
    buffer.reserve(3);
    return std::all_of(
        std::begin(CGAL::faces(mesh)), std::end(CGAL::faces(mesh)), [&buffer, &mesh](auto face) {
            return IsFaceInMeshPlanar(mesh, face, buffer);
        });
}

void NormaliseAndValidateMesh(SurfaceMesh& mesh, const RegionMap* regions)
{
    namespace PMP = CGAL::Polygon_mesh_processing;
    if(!CGAL::is_triangle_mesh(mesh)) {
        PMP::triangulate_faces(mesh);
    }

    std::vector<size_t> component(mesh.number_of_faces(), 0);
    const auto component_of =
        boost::make_iterator_property_map(std::begin(component), get(CGAL::face_index, mesh));
    const auto count = PMP::connected_components(mesh, component_of);
    if(count > 1) {
        if(regions == nullptr) {
            throw SimulationError("Expected exactly 1 connected component, got: {}", count);
        }
        const auto first = *mesh.faces().begin();
        for(const auto face : mesh.faces()) {
            if(component[face] != component[first]) {
                throw SimulationError(
                    "Region {} is not connected to region {}.",
                    (*regions)[face],
                    (*regions)[first]);
            }
        }
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

    std::vector<std::pair<SurfaceMesh::Face_index, SurfaceMesh::Face_index>> intersecting{};
    PMP::self_intersections(mesh, std::back_inserter(intersecting));
    if(!intersecting.empty()) {
        if(regions == nullptr) {
            throw SimulationError("Mesh faces pass through each other.");
        }
        const auto& [one, other] = intersecting.front();
        throw SimulationError(
            "Region {} and region {} pass through each other.", (*regions)[one], (*regions)[other]);
    }
}
