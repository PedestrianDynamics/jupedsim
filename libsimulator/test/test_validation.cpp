// SPDX-License-Identifier: LGPL-3.0-or-later
#include "cfg_cgal.hpp"
#include "geometry/validation.hpp"
#include "simulation_error.hpp"
#include "test_common.hpp"

#include <CGAL/boost/graph/helpers.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <limits>
#include <ostream>
#include <random>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

TEST(NormaliseAndValidateMesh, TriangulatesNonTriangleFaces)
{
    epick::Mesh mesh{};
    const auto v0 = mesh.add_vertex(epick::Point3D{0, 0, 0});
    const auto v1 = mesh.add_vertex(epick::Point3D{10, 0, 0});
    const auto v2 = mesh.add_vertex(epick::Point3D{10, 10, 0});
    const auto v3 = mesh.add_vertex(epick::Point3D{0, 10, 0});
    mesh.add_face(v0, v1, v2, v3);

    normalise_and_validate_mesh(mesh);

    EXPECT_TRUE(CGAL::is_triangle_mesh(mesh));
    EXPECT_EQ(mesh.number_of_faces(), 2u);
}

struct Case {
    std::string name;
    std::vector<epick::Point2D> points;
    double dist;
    bool expected;
};

void PrintTo(const Case& c, std::ostream* out)
{
    *out << fmt::format(
        R"({{name = "{}", points = {}, dist = {}, expected = {}}})",
        c.name,
        c.points,
        c.dist,
        c.expected);
}

struct TestCheckMinClearance : ::testing::TestWithParam<Case> {};

TEST_P(TestCheckMinClearance, Checks)
{
    const auto& c = GetParam();
    const auto actual = check_min_clearance(c.points, c.dist);
    EXPECT_EQ(actual, c.expected);
}

INSTANTIATE_TEST_SUITE_P(
    NoErrorCases,
    TestCheckMinClearance,
    ::testing::Values(
        Case{.name = "Empty", .points = {}, .dist = 0.5, .expected = true},
        Case{.name = "WorksWithOnePoint", .points = {{0, 0}}, .dist = 0.5, .expected = true},
        Case{
            .name = "TwoPointsBelowClearance",
            .points = {{1337, 1338}, {1338, 1337}},
            .dist = 1.75,
            .expected = false},
        Case{
            .name = "TwoPointsAboveClearance",
            .points = {{1337, 1338}, {1338, 1337}},
            .dist = 1,
            .expected = true}

        ),
    [](const auto& info) { return info.param.name; });

TEST(TestCheckMinClerance, ThrowsOnInvalidDist)
{
    ASSERT_THROW(check_min_clearance(std::vector<epick::Point2D>{}, 0), SimulationError);
    ASSERT_THROW(check_min_clearance(std::vector<epick::Point2D>{}, -1), SimulationError);
}

static_assert(CgalPoint<epick::Point2D> && CgalPoint<epick::Point3D>);
static_assert(!CgalPoint<std::pair<double, double>> && !CgalPoint<std::array<double, 2>>);

TEST(TestCheckMinClearance, DuplicatePoints)
{
    const std::vector<epick::Point2D> points{{1, 1}, {5, 5}, {1, 1}};
    EXPECT_FALSE(check_min_clearance(points, 0.1));
}

TEST(TestCheckMinClearance, UsesZFor3DPoints)
{
    // Same x and y, apart only in z.
    const std::vector<epick::Point3D> points{{2, 3, 0}, {2, 3, 1}};
    EXPECT_TRUE(check_min_clearance(points, 1));
    EXPECT_FALSE(check_min_clearance(points, 1.5));
}

TEST(TestCheckMinClearance, WorksOnViews)
{
    // Points 1 apart on a line, unsorted.
    const auto points = std::views::iota(0, 10) | std::views::transform([](int i) {
                            return epick::Point2D((i * 7) % 10, 0);
                        });
    EXPECT_TRUE(check_min_clearance(points, 1));
    EXPECT_FALSE(check_min_clearance(points, 1.5));
}

TEST(TestCheckMinClearance, WorksOnMeshPoints)
{
    epick::Mesh mesh{};
    mesh.add_vertex(epick::Point3D{0, 0, 0});
    mesh.add_vertex(epick::Point3D{0, 0, 3});
    mesh.add_vertex(epick::Point3D{4, 0, 0});
    EXPECT_TRUE(check_min_clearance(mesh.points(), 3));
    EXPECT_FALSE(check_min_clearance(mesh.points(), 3.5));
}

TEST(TestCheckMinClearance, ThrowsOnNonFiniteCoordinates)
{
    const std::vector<epick::Point2D> points{{0, 0}, {std::numeric_limits<double>::quiet_NaN(), 0}};
    EXPECT_THROW(check_min_clearance(points, 1), SimulationError);
    const std::vector<epick::Point3D> far{{0, 0, std::numeric_limits<double>::infinity()}};
    EXPECT_THROW(check_min_clearance(far, 1), SimulationError);
}

TEST(TestCheckMinClearance, NegativeCoordinatesAcrossCellBorders)
{
    // Cells are floor(c / dist): -0.1 and 0.1 lie in neighbouring cells.
    const std::vector<epick::Point2D> points{{-0.1, -0.1}, {0.1, 0.1}};
    EXPECT_FALSE(check_min_clearance(points, 1));
    EXPECT_TRUE(check_min_clearance(points, 0.2));
}

template <typename Point>
bool brute_force_min_clearance(const std::vector<Point>& points, double dist)
{
    for(size_t i = 0; i < points.size(); ++i) {
        for(size_t j = i + 1; j < points.size(); ++j) {
            if(CGAL::squared_distance(points[i], points[j]) < dist * dist) {
                return false;
            }
        }
    }
    return true;
}

TEST(TestCheckMinClearance, MatchesBruteForceOnRandomPoints)
{
    std::mt19937 rng{42};
    std::uniform_real_distribution<double> coordinate{-20.0, 20.0};
    for(int run = 0; run < 200; ++run) {
        const size_t count = 2 + run % 40;
        const double dist = 0.5 + (run % 7);
        std::vector<epick::Point2D> points_2d{};
        std::vector<epick::Point3D> points_3d{};
        for(size_t i = 0; i < count; ++i) {
            points_2d.emplace_back(coordinate(rng), coordinate(rng));
            points_3d.emplace_back(coordinate(rng), coordinate(rng), coordinate(rng));
        }
        EXPECT_EQ(check_min_clearance(points_2d, dist), brute_force_min_clearance(points_2d, dist))
            << "run " << run;
        EXPECT_EQ(check_min_clearance(points_3d, dist), brute_force_min_clearance(points_3d, dist))
            << "run " << run;
    }
}

TEST(FindClearanceViolation, ReportsTheClosePair)
{
    const std::vector<epick::Point2D> points{{0, 0}, {5, 5}, {5.01, 5}, {9, 9}};
    const auto violation = find_clearance_violation(points, 0.1);
    ASSERT_TRUE(violation.has_value());
    EXPECT_EQ(violation->first, epick::Point2D(5, 5));
    EXPECT_EQ(violation->second, epick::Point2D(5.01, 5));
    EXPECT_FALSE(find_clearance_violation(points, 0.001).has_value());
}
