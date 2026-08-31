// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "CfgCgal.hpp"

#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

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

/// The two-level fold again, but meshed across the sweep as well as along it, so the seam
/// where the levels meet runs over @p strips edges instead of the single one
/// `extrude_profile` lays across the whole width.
inline SurfaceMesh two_levels_with_wide_seam(std::size_t strips = 4)
{
    const std::vector<std::array<double, 2>> profile{{0, 0}, {10, 0}, {15, 3}, {5, 3}};
    constexpr double width = 4.0;

    SurfaceMesh mesh{};
    std::vector<std::vector<SurfaceMesh::Vertex_index>> rows(strips + 1);
    for(std::size_t s = 0; s <= strips; ++s) {
        const double y = width * static_cast<double>(s) / static_cast<double>(strips);
        for(const auto& [x, z] : profile) {
            rows[s].push_back(mesh.add_vertex(Point3D{x, y, z}));
        }
    }
    for(std::size_t s = 0; s < strips; ++s) {
        for(std::size_t i = 0; i + 1 < profile.size(); ++i) {
            mesh.add_face(rows[s][i], rows[s][i + 1], rows[s + 1][i + 1]);
            mesh.add_face(rows[s][i], rows[s + 1][i + 1], rows[s + 1][i]);
        }
    }
    return mesh;
}

/// Two floors sharing a footprint with nothing joining them. Non-realistic, but does
/// not matter for the testing purpose.
inline SurfaceMesh stacked_floors(double upper_z = 3.0)
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
    quad(upper_z);
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
/// and the landing sit beside the ground floor, not over it. The landing and the upper
/// floor are coplanar and welded, so the region overlay fuses them and the cut falls at the
/// flight/landing seam (x = 14) -- regardless of the order the quads are fed in, which is
/// what the `upper_first` flag exists to demonstrate.
///
/// The wall along y = 0 runs from x = 0 to x = 18 across ground floor, flight and landing:
/// straight in plan, and across the region boundary. The wall along y = 8 (landing + upper
/// floor) is equally straight but lies wholly inside the fused region.
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

/// A stair straight up to a higher landing: ground floor, a flight climbing 3 m over 5 m across
/// the full width, and the landing carrying on at the top. Nothing lies over anything.
///
/// The plain climb, and the one a whole simulation can walk end to end: the way up is straight
/// in plan, so it passes no wall corner -- which a route that keeps no distance from walls
/// cannot get an agent around: That requires "wall clearance".
inline SurfaceMesh straight_stair_to_a_landing()
{
    QuadMesh builder{};
    builder.add({{{0, 0, 0}, {10, 0, 0}, {10, 8, 0}, {0, 8, 0}}}); // ground floor
    builder.add({{{10, 0, 0}, {15, 0, 3}, {15, 8, 3}, {10, 8, 0}}}); // flight, full width
    builder.add({{{15, 0, 3}, {20, 0, 3}, {20, 8, 3}, {15, 8, 3}}}); // landing at the top
    return builder.take();
}

/// A stair that turns on a landing and climbs on: the first flight out along y in [0, 2], the
/// landing at the far end, the second flight back along y in [3, 5], ending 6 m up. Between
/// the flights the stair well, open at the near end.
///
/// The flights lie beside each other in plan and overlap nowhere, so this is a single region --
/// and still the far wall of the well stands 3 m up and climbs to 6, one metre in plan from
/// someone at the foot of the first flight.
///
/// The landing is cut into strips matching the flights: sharing a vertex is not sharing an
/// edge, and a T-junction would leave the flights topologically apart.
inline SurfaceMesh stair_turning_on_a_landing()
{
    QuadMesh builder{};
    builder.add({{{0, 0, 0}, {6, 0, 3}, {6, 2, 3}, {0, 2, 0}}}); // first flight, climbing
    builder.add({{{6, 0, 3}, {8, 0, 3}, {8, 2, 3}, {6, 2, 3}}}); // landing, beside the flight
    builder.add({{{6, 2, 3}, {8, 2, 3}, {8, 3, 3}, {6, 3, 3}}}); // landing, across the well
    builder.add({{{6, 3, 3}, {8, 3, 3}, {8, 5, 3}, {6, 5, 3}}}); // landing, beside the flight
    builder.add({{{0, 3, 6}, {6, 3, 3}, {6, 5, 3}, {0, 5, 6}}}); // second flight, climbing on
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

/// A corridor 45 m long and 2 m wide with door recesses down one side, given as the single
/// polygonal face a floor plan arrives as and cut into triangles the way any polygon face is:
/// by clipping ears off it.
///
/// That is what the other fixtures cannot show. Ear clipping is not Delaunay, so the corridor
/// ends up carrying triangles that span its whole length on a couple of square metres, and
/// locating a point on one of those loses three orders of magnitude more than the coordinates
/// themselves carry. Every hand-meshed fixture here is small and well shaped, and neither is a
/// property of real input.
inline SurfaceMesh corridor_with_door_recesses()
{
    constexpr double length = 45.0;
    constexpr double width = 2.0;
    constexpr double depth = 0.3;
    // Walked back along the far wall, so the ring stays counter-clockwise.
    constexpr std::array<std::array<double, 2>, 7> doors{
        {{42, 41}, {36, 35}, {30, 29}, {24, 23}, {18, 17}, {12, 11}, {6, 5}}};

    std::vector<std::array<double, 2>> ring{{0, 0}, {length, 0}, {length, width}};
    for(const auto& [near, far] : doors) {
        ring.push_back({near, width});
        ring.push_back({near, width + depth});
        ring.push_back({far, width + depth});
        ring.push_back({far, width});
    }
    ring.push_back({0, width});

    SurfaceMesh mesh{};
    std::vector<SurfaceMesh::Vertex_index> corners{};
    for(const auto& [x, y] : ring) {
        corners.push_back(mesh.add_vertex(Point3D{x, y, 0.0}));
    }
    mesh.add_face(corners);
    CGAL::Polygon_mesh_processing::triangulate_faces(mesh);
    return mesh;
}

} // namespace fixtures
