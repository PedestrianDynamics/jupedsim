// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "cfg_cgal.hpp"
#include "geometry/region_split.hpp"
#include "simulation_error.hpp"

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

/// @file Validation.hpp
/// Contains support code to validate inputs or intermediate results.

/// Test if the supplied normal is inclined by 50deg or less over the X/Y plane.
/// @param n normal vector of the plane to test for walkability
/// @pre #n needs to be normalized; otherwise the result is not reliable.
/// @return true if the normal indicates that the plane is walkable.
bool is_walkable_normal(const epick::Vector3D& n);

/// Test if face in mesh is planar.
/// Degenerate faces (2 or fewer vertices, or all vertices collinear) will return false.
/// @param mesh with face to test
/// @param face to test
/// @param buffer to use for temporary internal storage. Supply this when calling
///        is_face_in_mesh_planar repeatedly to reduce allocations.
/// @return true if face is planar
bool is_face_in_mesh_planar(
    const epick::Mesh& mesh,
    FaceDescriptor<epick::Mesh> face,
    std::vector<VertexDescriptor<epick::Mesh>>& buffer);

/// Test if all faces in the given #mesh are planar.
/// @param mesh to test.
/// @return true if all faces are planar
bool all_faces_in_mesh_planar(const epick::Mesh& mesh);

/// Validate that the given mesh can be processed by jupedsim.
/// Ensures the mesh is a single connected component, has no degenerate faces, all faces are
/// planar, no incline exceeds 50deg, and no faces pass through each other. Non-triangular faces
/// are triangulated and if the surface normals point downwards, all face orientations are
/// reversed, i.e. the mesh may be modified.
/// @param mesh to check
/// @param regions face -> region map of @p mesh.
/// @throws SimulationError with a description of the issue encountered.
void normalise_and_validate_mesh(epick::Mesh& mesh, const RegionMap* regions = nullptr);

/// Validate a triangle mesh whose faces all point upwards, without modifying it.
/// Checks that no incline exceeds 50deg, that no faces pass through each other, and that the
/// mesh is a single connected component.
/// @param mesh to check
/// @param regions face -> region map of @p mesh.
/// @throws SimulationError with a description of the issue encountered.
void validate_mesh(const epick::Mesh& mesh, const RegionMap* regions = nullptr);

namespace detail
{
/// Integer coordinates of a cell in a grid of cubes (squares in 2D).
template <size_t Dimension>
using GridCell = std::array<std::int64_t, Dimension>;

template <size_t Dimension>
struct GridCellHash {
    size_t operator()(const GridCell<Dimension>& cell) const
    {
        size_t hash = 0;
        for(const auto coordinate : cell) {
            hash ^= std::hash<std::int64_t>{}(coordinate) + 0x9e3779b97f4a7c15 + (hash << 6) +
                    (hash >> 2);
        }
        return hash;
    }
};
} // namespace detail

/// A CGAL 2D or 3D point of any kernel.
template <typename T>
concept CgalPoint =
    std::same_as<T, CGAL::Point_2<typename T::R>> || std::same_as<T, CGAL::Point_3<typename T::R>>;

/// Find a pair of points closer than @p dist.
/// Distances are Euclidean in the dimension of the points. Runs in expected linear time.
/// @param points to check, any input range of 2D or 3D CGAL points.
/// @param dist minimum distance between all pairs of points, must be > 0.
/// @return the first pair found that is closer than @p dist, or nothing.
/// @throws SimulationError if @p dist <= 0, or a coordinate is not finite or too large for a
///         grid with cell size @p dist.
template <std::ranges::input_range Range>
    requires CgalPoint<std::ranges::range_value_t<Range>>
auto find_clearance_violation(Range&& points, double dist) -> std::optional<
    std::pair<std::ranges::range_value_t<Range>, std::ranges::range_value_t<Range>>>
{
    if(dist <= 0) {
        throw SimulationError(
            "check_min_clearance precondition violation, called with dist <= 0 [dist = {}]", dist);
    }
    using Point = std::ranges::range_value_t<Range>;
    constexpr size_t dimension = Point::Ambient_dimension::value;
    using Cell = detail::GridCell<dimension>;

    // Grid with cell size dist: a point closer than dist to p lies in p's cell or one of its
    // direct neighbours. Each cell holds only a few points before two of them are too close.
    std::unordered_map<Cell, std::vector<Point>, detail::GridCellHash<dimension>> grid{};
    if constexpr(std::ranges::sized_range<Range>) {
        grid.reserve(std::ranges::size(points));
    }
    // Bound on cell coordinates; leaves room for the neighbour offsets.
    constexpr double max_cell = 9.0e18;
    const double dist_sq = dist * dist;
    // 3^dimension cells: the point's own cell and all its neighbours.
    constexpr size_t neighbourhood = dimension == 2 ? 9 : 27;

    for(auto&& point : points) {
        const Point p(point);
        Cell cell{};
        for(size_t axis = 0; axis < dimension; ++axis) {
            const double coordinate = std::floor(CGAL::to_double(p.cartesian(axis)) / dist);
            // Also rejects NaN and infinity.
            if(!(std::abs(coordinate) < max_cell)) {
                throw SimulationError(
                    "check_min_clearance: coordinate of {} is not finite or too large for dist "
                    "{}",
                    p,
                    dist);
            }
            cell[axis] = static_cast<std::int64_t>(coordinate);
        }

        for(size_t offset = 0; offset < neighbourhood; ++offset) {
            Cell neighbour = cell;
            size_t digits = offset;
            for(size_t axis = 0; axis < dimension; ++axis) {
                neighbour[axis] += static_cast<std::int64_t>(digits % 3) - 1;
                digits /= 3;
            }
            const auto found = grid.find(neighbour);
            if(found == std::end(grid)) {
                continue;
            }
            for(const Point& other : found->second) {
                if(CGAL::to_double(CGAL::squared_distance(p, other)) < dist_sq) {
                    return std::pair{other, p};
                }
            }
        }
        grid[cell].push_back(p);
    }
    return std::nullopt;
}

/// Check all points are pairwise at least @p dist apart.
/// See find_clearance_violation for parameters and exceptions.
/// @return false if at least one pair is closer than @p dist.
template <std::ranges::input_range Range>
    requires CgalPoint<std::ranges::range_value_t<Range>>
bool check_min_clearance(Range&& points, double dist)
{
    return !find_clearance_violation(std::forward<Range>(points), dist).has_value();
}
