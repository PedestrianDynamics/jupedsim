// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <map>
#include <utility>
#include <vector>

/// Meshes the geometry tests share. They are deliberately small and named after what they
/// are as a building, so that a failing test says which situation broke.
namespace fixtures
{

/// Sweep a profile given in (x, z) along y, producing a strip of quads: enough for a floor,
/// a stair run, or a level folding back over another.
inline SurfaceMesh extrude_profile(const std::vector<std::array<double, 2>>& profile, double width)
{
    SurfaceMesh mesh{};
    std::vector<SurfaceMesh::Vertex_index> near{};
    std::vector<SurfaceMesh::Vertex_index> far{};
    for(const auto& [x, z] : profile) {
        near.push_back(mesh.add_vertex(Point3D{x, 0.0, z}));
        far.push_back(mesh.add_vertex(Point3D{x, width, z}));
    }
    for(std::size_t i = 0; i + 1 < profile.size(); ++i) {
        mesh.add_face(near[i], near[i + 1], far[i + 1]);
        mesh.add_face(near[i], far[i + 1], far[i]);
    }
    return mesh;
}

/// A flat 10x10 rectangle whose sides are subdivided by a collinear mid vertex.
inline SurfaceMesh flat_rectangle()
{
    return extrude_profile({{0, 0}, {5, 0}, {10, 0}}, 10.0);
}

/// Realistic geometry: A 4 m wide corridor at z=0, a straight stair run climbing 3 m
/// over 5 m, and an upper corridor at z=3 running back so that it is above the lower one.
///
/// The overhang makes the surface two-valued over x in [5, 10], so the region overlay
/// splits: lower corridor plus stair in one region, the upper corridor in the other.
inline SurfaceMesh two_levels_with_stair()
{
    return extrude_profile({{0, 0}, {10, 0}, {15, 3}, {5, 3}}, 4.0);
}

/// Two floors sharing a footprint with nothing joining them. Non-realistic, but does
/// not matter for the testing purpose.
inline SurfaceMesh stacked_floors()
{
    SurfaceMesh mesh{};
    const auto quad = [&mesh](double z) {
        const auto a = mesh.add_vertex(Point3D{0, 0, z});
        const auto b = mesh.add_vertex(Point3D{10, 0, z});
        const auto c = mesh.add_vertex(Point3D{10, 10, z});
        const auto d = mesh.add_vertex(Point3D{0, 10, z});
        mesh.add_face(a, b, c);
        mesh.add_face(a, c, d);
    };
    quad(0.0);
    quad(3.0);
    return mesh;
}

/// Builds a mesh from quads given as corner coordinates, reusing a vertex whenever the same
/// coordinates come up again. Without that sharing, quads meeting along an edge would stay
/// topologically apart and neither the border walk nor the region flood would cross.
class QuadMesh
{
    SurfaceMesh _mesh{};
    std::map<std::array<double, 3>, SurfaceMesh::Vertex_index> _vertices{};

    SurfaceMesh::Vertex_index at(const std::array<double, 3>& c)
    {
        const auto it = _vertices.find(c);
        if(it != _vertices.end()) {
            return it->second;
        }
        return _vertices[c] = _mesh.add_vertex(Point3D{c[0], c[1], c[2]});
    }

public:
    using Quad = std::array<std::array<double, 3>, 4>;

    void add(const Quad& corners)
    {
        std::array<SurfaceMesh::Vertex_index, 4> v{};
        for(std::size_t i = 0; i < 4; ++i) {
            v[i] = at(corners[i]);
        }
        _mesh.add_face(v[0], v[1], v[2]);
        _mesh.add_face(v[0], v[2], v[3]);
    }

    SurfaceMesh take() { return std::move(_mesh); }
};

/// A switchback stair, the ordinary kind: ground floor, a flight climbing away from it, a
/// landing at the top, and the upper floor turning back over the ground floor.
///
/// Its point is what the extruded fixtures cannot show. The pieces form the chain
/// ground - flight - landing - upper, and only the two ends overlap in (x, y) -- the flight
/// and the landing sit beside the ground floor, not over it. The region flood is greedy from
/// an arbitrary seed, so with the two ends in conflict and the middle free, *where* it cuts
/// depends on which end it starts from. Feeding the quads in the other order moves the cut.
///
/// The wall along y = 8 runs from x = 0 to x = 18 across landing and upper floor: straight
/// in plan, and across whatever region boundary the flood happened to draw.
inline std::vector<QuadMesh::Quad> switchback_stair_quads()
{
    return {
        {{{0, 0, 0}, {10, 0, 0}, {10, 4, 0}, {0, 4, 0}}}, // ground floor, near half
        {{{0, 4, 0}, {10, 4, 0}, {10, 8, 0}, {0, 8, 0}}}, // ground floor, far half
        {{{10, 0, 0}, {14, 0, 3}, {14, 4, 3}, {10, 4, 0}}}, // flight, climbing 3 m over 4 m
        {{{14, 0, 3}, {18, 0, 3}, {18, 4, 3}, {14, 4, 3}}}, // landing, near half
        {{{14, 4, 3}, {18, 4, 3}, {18, 8, 3}, {14, 8, 3}}}, // landing, far half
        {{{0, 4, 3}, {14, 4, 3}, {14, 8, 3}, {0, 8, 3}}}}; // upper floor, back over the ground
}

inline SurfaceMesh switchback_stair(bool upper_first = false)
{
    auto quads = switchback_stair_quads();
    if(upper_first) {
        std::reverse(quads.begin(), quads.end());
    }
    QuadMesh builder{};
    for(const auto& q : quads) {
        builder.add(q);
    }
    return builder.take();
}

/// A rectangular field, 20 x 10, meshed on a 2 m grid, whose height gently undulates. The
/// commonest 3D input there is, and the one a piecewise-planar fixture cannot stand in for:
/// its outline is straight in plan but no two consecutive border edges are collinear in 3D.
inline SurfaceMesh wavy_terrain()
{
    QuadMesh builder{};
    const auto height = [](double x, double y) {
        return 0.6 * std::sin(x * 0.5) * std::cos(y * 0.4);
    };
    for(int i = 0; i < 10; ++i) {
        for(int j = 0; j < 5; ++j) {
            const double x = i * 2.0;
            const double y = j * 2.0;
            builder.add(
                {{{x, y, height(x, y)},
                  {x + 2.0, y, height(x + 2.0, y)},
                  {x + 2.0, y + 2.0, height(x + 2.0, y + 2.0)},
                  {x, y + 2.0, height(x, y + 2.0)}}});
        }
    }
    return builder.take();
}

} // namespace fixtures
