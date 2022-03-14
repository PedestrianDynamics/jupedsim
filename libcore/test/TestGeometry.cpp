#include "Geometry.hpp"
#include "geometry/Line.hpp"
#include "geometry/Point.hpp"

#include <deque>
#include <gtest/gtest.h>

TEST(Geometry, CanBuildEmpty)
{
    GeometryBuilder geo{};
    ASSERT_NO_THROW(geo.Build());
}

TEST(Geometry, CanIterate)
{
    GeometryBuilder builder{};
    const auto geo = builder.Build();
    for(const auto & line : geo.LineSegmentsInDistanceTo(5, Point(0, 0))) {
        (void) line;
    }
}

class GeometryFilterByDistance : public ::testing::Test
{
public:
    GeometryFilterByDistance() : geometry(build()){};

protected:
    Geometry geometry;

private:
    static Geometry build()
    {
        GeometryBuilder b{};
        b.AddLineSegment(-1, 0, 1, 0).AddLineSegment(-1, 1, 1, 1);
        return b.Build();
    }
};

TEST_F(GeometryFilterByDistance, NoneAreInRange)
{
    const auto range = geometry.LineSegmentsInDistanceTo(1, Point{0, -2});
    ASSERT_EQ(range.begin(), range.end());
}

TEST_F(GeometryFilterByDistance, SomeAreInRange)
{
    const auto range = geometry.LineSegmentsInDistanceTo(2, Point{0, -2});
    for(const auto & line : range) {
        ASSERT_EQ(line, Line(Point(-1, 0), Point(1, 0)));
    }
}

TEST_F(GeometryFilterByDistance, AllAreInRange)
{
    const auto range = geometry.LineSegmentsInDistanceTo(1, Point{0, -2});
    std::deque<Line> expected{Line(Point(-1, 0), Point(1, 0)), Line(Point(-1, 1), Point(1, 1))};
    for(const auto & line : range) {
        ASSERT_EQ(line, expected.front());
        expected.pop_front();
    }
}
