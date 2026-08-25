// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/RegionSplit.hpp"

#include "Geometry/ProjectedBoundary.hpp"

#include <CGAL/Polygon_mesh_processing/compute_normal.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <queue>
#include <utility>
#include <vector>

namespace
{
constexpr auto UNASSIGNED = std::numeric_limits<std::size_t>::max();

// Tolerances of the planar floodfill. Membership is always tested against the SEED face's
// plane (never a drifting average), so they bound a patch's total slab thickness and keep
// the result independent of growth order. The values sit orders of magnitude between the
// two things they must separate: remeshing/relaxation jitter (isotropic_remeshing moves
// vertices via inexact closest-point constructions, ~1e-10 m and ~1e-9 rad at building
// scale) far below, real geometry (>= ~2 m layer separation, slope changes of degrees)
// far above. Mis-tuning costs performance only -- the certificate below catches any patch
// the tolerance would break, never an invalid region.
constexpr double plane_distance_tolerance = 1e-4; // metres
constexpr double plane_normal_min_cos = 0.99996192306417; // cos(0.5 deg), fixed literal

namespace PMP = CGAL::Polygon_mesh_processing;

struct SeedPlane {
    Vector3D unit_normal;
    Point3D base;
};

/// One planar patch (phase 1), later a merged region (phase 2). Only union-find roots
/// remain live; an absorbed patch's data is left behind and never read again.
struct Patch {
    SurfaceMesh::Face_index seed{}; // smallest face index, total tie-break of the height key
    double min_z = std::numeric_limits<double>::infinity();
    std::vector<SurfaceMesh::Halfedge_index> boundary{};
    std::vector<std::size_t> neighbors{}; // patch ids, possibly stale; re-rooted on use
    std::uint64_t version = 0; // bumped on every absorb, keys the failed-pair cache
    bool alive = true;
};

/// Ascending (min_z, seed) -- the bottom-up processing order of phase 2.
bool patch_key_less(const Patch& a, const Patch& b)
{
    if(a.min_z != b.min_z) {
        return a.min_z < b.min_z;
    }
    return a.seed < b.seed;
}

bool fits_seed_plane(
    const SurfaceMesh& mesh,
    SurfaceMesh::Face_index f,
    const Vector3D& face_normal,
    const SeedPlane& plane)
{
    if(CGAL::to_double(face_normal * plane.unit_normal) < plane_normal_min_cos) {
        return false;
    }
    for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        const auto distance = CGAL::to_double((mesh.point(v) - plane.base) * plane.unit_normal);
        if(std::abs(distance) > plane_distance_tolerance) {
            return false;
        }
    }
    return true;
}

/// Boundary halfedges of the faces with patch_of[f] == id, walking only @p patch_faces.
std::vector<SurfaceMesh::Halfedge_index> patch_boundary(
    const SurfaceMesh& mesh,
    const std::vector<SurfaceMesh::Face_index>& patch_faces,
    const std::vector<std::size_t>& patch_of,
    std::size_t id)
{
    std::vector<SurfaceMesh::Halfedge_index> boundary{};
    for(const auto f : patch_faces) {
        for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(f), mesh)) {
            const auto opp = mesh.opposite(h);
            if(mesh.is_border(opp) || patch_of[mesh.face(opp)] != id) {
                boundary.emplace_back(h);
            }
        }
    }
    return boundary;
}

double min_face_z(const SurfaceMesh& mesh, SurfaceMesh::Face_index f)
{
    double z = std::numeric_limits<double>::infinity();
    for(const auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
        z = std::min(z, CGAL::to_double(mesh.point(v).z()));
    }
    return z;
}

/// Register a finished patch: fill its metadata and boundary.
void finish_patch(
    const SurfaceMesh& mesh,
    const std::vector<SurfaceMesh::Face_index>& patch_faces,
    const std::vector<std::size_t>& patch_of,
    Patch& patch)
{
    patch.seed = patch_faces.front();
    for(const auto f : patch_faces) {
        patch.seed = std::min(patch.seed, f);
        patch.min_z = std::min(patch.min_z, min_face_z(mesh, f));
    }
    patch.boundary = patch_boundary(mesh, patch_faces, patch_of, patch_of[patch_faces.front()]);
}

/// Fallback when an epsilon-tolerant patch fails its certificate (a fold-back within the
/// tolerance slab): re-split exactly. An exactly coplanar connected patch with upward
/// normals projects by the restriction of an affine bijection, hence injectively -- the
/// result needs no certificate and the recursion needs no further fallback.
void resplit_exactly_coplanar(
    const SurfaceMesh& mesh,
    const std::vector<SurfaceMesh::Face_index>& faces_to_split,
    std::vector<std::size_t>& patch_of,
    std::vector<Patch>& patches,
    std::vector<std::uint8_t>& in_set)
{
    for(const auto f : faces_to_split) {
        patch_of[f] = UNASSIGNED;
        in_set[f] = 1;
    }
    std::vector<SurfaceMesh::Face_index> patch_faces{};
    for(const auto seed : faces_to_split) {
        if(patch_of[seed] != UNASSIGNED) {
            continue;
        }
        const auto id = patches.size();
        std::array<Point3D, 3> p{};
        int i = 0;
        for(const auto v : CGAL::vertices_around_face(mesh.halfedge(seed), mesh)) {
            p[i++] = mesh.point(v);
        }
        patch_faces.clear();
        std::queue<SurfaceMesh::Face_index> frontier{};
        patch_of[seed] = id;
        patch_faces.emplace_back(seed);
        frontier.push(seed);
        while(!frontier.empty()) {
            const auto g = frontier.front();
            frontier.pop();
            for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(g), mesh)) {
                const auto opp = mesh.opposite(h);
                if(mesh.is_border(opp)) {
                    continue;
                }
                const auto nbr = mesh.face(opp);
                if(!in_set[nbr] || patch_of[nbr] != UNASSIGNED) {
                    continue;
                }
                bool coplanar = true;
                for(const auto v : CGAL::vertices_around_face(mesh.halfedge(nbr), mesh)) {
                    coplanar = coplanar && CGAL::coplanar(p[0], p[1], p[2], mesh.point(v));
                }
                if(!coplanar) {
                    continue;
                }
                patch_of[nbr] = id;
                patch_faces.emplace_back(nbr);
                frontier.push(nbr);
            }
        }
        finish_patch(mesh, patch_faces, patch_of, patches.emplace_back());
    }
    for(const auto f : faces_to_split) {
        in_set[f] = 0;
    }
}

/// Phase 1: epsilon-tolerant planar floodfill. Seeds in ascending face-index order; a
/// neighbor joins while it fits the seed's plane. Every patch is certified; failures are
/// re-split exactly.
std::vector<std::size_t> grow_planar_patches(const SurfaceMesh& mesh, std::vector<Patch>& patches)
{
    std::vector<std::size_t> patch_of(mesh.number_of_faces(), UNASSIGNED);
    std::vector<Vector3D> normal_of(mesh.number_of_faces());
    for(const auto f : faces(mesh)) {
        normal_of[f] = PMP::compute_face_normal(f, mesh);
    }

    std::vector<std::uint8_t> in_set(mesh.number_of_faces(), 0);
    std::vector<SurfaceMesh::Face_index> patch_faces{};
    for(const auto seed : faces(mesh)) {
        if(patch_of[seed] != UNASSIGNED) {
            continue;
        }
        const auto id = patches.size();
        const SeedPlane plane{
            normal_of[seed], mesh.point(*std::begin(CGAL::vertices_around_face(mesh.halfedge(seed), mesh)))};
        patch_faces.clear();
        std::queue<SurfaceMesh::Face_index> frontier{};
        patch_of[seed] = id;
        patch_faces.emplace_back(seed);
        frontier.push(seed);
        while(!frontier.empty()) {
            const auto g = frontier.front();
            frontier.pop();
            for(const auto h : CGAL::halfedges_around_face(mesh.halfedge(g), mesh)) {
                const auto opp = mesh.opposite(h);
                if(mesh.is_border(opp)) {
                    continue;
                }
                const auto nbr = mesh.face(opp);
                if(patch_of[nbr] != UNASSIGNED ||
                   !fits_seed_plane(mesh, nbr, normal_of[nbr], plane)) {
                    continue;
                }
                patch_of[nbr] = id;
                patch_faces.emplace_back(nbr);
                frontier.push(nbr);
            }
        }
        auto& patch = patches.emplace_back();
        finish_patch(mesh, patch_faces, patch_of, patch);
        if(!is_projection_strictly_simple(mesh, patch.boundary)) {
            patches.pop_back();
            resplit_exactly_coplanar(mesh, patch_faces, patch_of, patches, in_set);
        }
    }
    return patch_of;
}

/// Path-compressing union-find over patch ids.
std::size_t find_root(std::vector<std::size_t>& parent, std::size_t id)
{
    while(parent[id] != id) {
        parent[id] = parent[parent[id]];
        id = parent[id];
    }
    return id;
}

/// The boundary a merge of roots @p a and @p b would have: both boundaries minus their
/// shared seam. Pure -- failed merge attempts must not mutate anything.
std::vector<SurfaceMesh::Halfedge_index> merged_boundary(
    const SurfaceMesh& mesh,
    const Patch& a,
    const Patch& b,
    const std::vector<std::size_t>& patch_of,
    std::vector<std::size_t>& parent,
    std::size_t root_a,
    std::size_t root_b)
{
    std::vector<SurfaceMesh::Halfedge_index> result{};
    result.reserve(a.boundary.size() + b.boundary.size());
    const auto keep = [&](SurfaceMesh::Halfedge_index h) {
        const auto opp = mesh.opposite(h);
        if(mesh.is_border(opp)) {
            return true;
        }
        const auto r = find_root(parent, patch_of[mesh.face(opp)]);
        return r != root_a && r != root_b;
    };
    std::copy_if(a.boundary.begin(), a.boundary.end(), std::back_inserter(result), keep);
    std::copy_if(b.boundary.begin(), b.boundary.end(), std::back_inserter(result), keep);
    return result;
}

/// Phase 2: bottom-up greedy merging. Regions in ascending (min_z, seed) order absorb
/// adjacent regions whenever the merged projected boundary stays strictly simple. Repeats
/// until a full sweep merges nothing.
void merge_to_fixpoint(
    const SurfaceMesh& mesh,
    const std::vector<std::size_t>& patch_of,
    std::vector<Patch>& patches,
    std::vector<std::size_t>& parent)
{
    // Region adjacency from one sweep over the interior edges.
    for(const auto e : edges(mesh)) {
        const auto h = mesh.halfedge(e, 0);
        const auto opp = mesh.halfedge(e, 1);
        if(mesh.is_border(h) || mesh.is_border(opp)) {
            continue;
        }
        const auto a = patch_of[mesh.face(h)];
        const auto b = patch_of[mesh.face(opp)];
        if(a != b) {
            patches[a].neighbors.emplace_back(b);
            patches[b].neighbors.emplace_back(a);
        }
    }
    for(auto& patch : patches) {
        std::sort(patch.neighbors.begin(), patch.neighbors.end());
        patch.neighbors.erase(
            std::unique(patch.neighbors.begin(), patch.neighbors.end()), patch.neighbors.end());
    }

    // (root a, root b) -> versions at the last failed attempt; unchanged versions mean the
    // attempt would fail again. Purely an optimization, results are identical without it.
    std::map<std::pair<std::size_t, std::size_t>, std::pair<std::uint64_t, std::uint64_t>>
        failed{};

    const auto neighbor_roots = [&](std::size_t root) {
        std::vector<std::size_t> roots{};
        roots.reserve(patches[root].neighbors.size());
        for(const auto n : patches[root].neighbors) {
            const auto r = find_root(parent, n);
            if(r != root) {
                roots.emplace_back(r);
            }
        }
        std::sort(roots.begin(), roots.end(), [&](std::size_t x, std::size_t y) {
            return patch_key_less(patches[x], patches[y]);
        });
        roots.erase(std::unique(roots.begin(), roots.end()), roots.end());
        return roots;
    };

    bool merged_any = true;
    [[maybe_unused]] std::size_t sweeps = 0;
    while(merged_any) {
        merged_any = false;
        assert(++sweeps <= patches.size() + 1);
        std::vector<std::size_t> order{};
        for(std::size_t i = 0; i < patches.size(); ++i) {
            if(patches[i].alive) {
                order.emplace_back(i);
            }
        }
        std::sort(order.begin(), order.end(), [&](std::size_t x, std::size_t y) {
            return patch_key_less(patches[x], patches[y]);
        });
        for(const auto a : order) {
            if(!patches[a].alive) {
                continue; // absorbed earlier in this sweep
            }
            bool absorbed_neighbor = true;
            while(absorbed_neighbor) {
                absorbed_neighbor = false;
                for(const auto b : neighbor_roots(a)) {
                    const auto key = std::minmax(a, b);
                    const auto versions = std::make_pair(patches[a].version, patches[b].version);
                    if(const auto it = failed.find({key.first, key.second});
                       it != failed.end() && it->second == versions) {
                        continue;
                    }
                    auto candidate =
                        merged_boundary(mesh, patches[a], patches[b], patch_of, parent, a, b);
                    if(!is_projection_strictly_simple(mesh, candidate)) {
                        failed[{key.first, key.second}] = versions;
                        continue;
                    }
                    patches[a].boundary = std::move(candidate);
                    patches[a].min_z = std::min(patches[a].min_z, patches[b].min_z);
                    patches[a].seed = std::min(patches[a].seed, patches[b].seed);
                    patches[a].neighbors.insert(
                        patches[a].neighbors.end(),
                        patches[b].neighbors.begin(),
                        patches[b].neighbors.end());
                    ++patches[a].version;
                    patches[b].alive = false;
                    parent[b] = a;
                    merged_any = true;
                    absorbed_neighbor = true;
                    break; // the neighbor set changed, re-enumerate
                }
            }
        }
    }

    // Leave the final root of every patch in parent[] for the caller.
    for(std::size_t i = 0; i < parent.size(); ++i) {
        find_root(parent, i);
    }
}
} // namespace

RegionSplit split_into_regions(const SurfaceMesh& mesh)
{
    // number_of_faces() counts live faces only; with garbage, face indices exceed it
    // and the vector would be indexed out of bounds.
    assert(!mesh.has_garbage());
    if(mesh.number_of_faces() == 0) {
        return {{}, 0};
    }

    std::vector<Patch> patches{};
    const auto patch_of = grow_planar_patches(mesh, patches);

    std::vector<std::size_t> parent(patches.size());
    for(std::size_t i = 0; i < parent.size(); ++i) {
        parent[i] = i;
    }
    merge_to_fixpoint(mesh, patch_of, patches, parent);

    // Compact region ids in first-seen-face order.
    std::vector<std::size_t> region(mesh.number_of_faces(), UNASSIGNED);
    std::map<std::size_t, std::size_t> compact{};
    std::size_t next_id = 0;
    for(const auto f : faces(mesh)) {
        const auto root = parent[patch_of[f]];
        auto [it, inserted] = compact.try_emplace(root, next_id);
        if(inserted) {
            ++next_id;
        }
        region[f] = it->second;
    }
    return {region, next_id};
}
