/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "CollisionGeometry.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <deque>
#include <gtest/gtest.h>

// TEST(CollisionGeometry, CanBuildEmpty)
//{
//     CollisionGeometryBuilder geo{};
//     ASSERT_NO_THROW(geo.Build());
// }
//
// TEST(Geometry, CanIterate)
//{
//     CollisionGeometryBuilder builder{};
//     const auto geo = builder.Build();
//     for(const auto& line : geo.LineSegmentsInDistanceTo(5, Point(0, 0))) {
//         (void) line;
//     }
// }
//
//  class CollisionGeometryFilterByDistance : public ::testing::Test
//{
//  public:
//      CollisionGeometryFilterByDistance() : geometry(build()){};
//
//  protected:
//      CollisionGeometry geometry;
//
//  private:
//      static CollisionGeometry build()
//      {
//          CollisionGeometryBuilder b{};
//          b.AddLineSegment(-1, 0, 1, 0).AddLineSegment(-1, 1, 1, 1);
//          return b.Build();
//      }
//  };
//
//  TEST_F(CollisionGeometryFilterByDistance, NoneAreInRange)
//{
//      const auto range = geometry.LineSegmentsInDistanceTo(1, Point{0, -2});
//      ASSERT_EQ(range.begin(), range.end());
//  }
//
//  TEST_F(CollisionGeometryFilterByDistance, SomeAreInRange)
//{
//      const auto range = geometry.LineSegmentsInDistanceTo(2, Point{0, -2});
//      for(const auto& line : range) {
//          ASSERT_EQ(line, LineSegment(Point(-1, 0), Point(1, 0)));
//      }
//  }
//
//  TEST_F(CollisionGeometryFilterByDistance, AllAreInRange)
//{
//      const auto range = geometry.LineSegmentsInDistanceTo(1, Point{0, -2});
//      std::deque<LineSegment> expected{
//          LineSegment(Point(-1, 0), Point(1, 0)), LineSegment(Point(-1, 1), Point(1, 1))};
//      for(const auto& line : range) {
//          ASSERT_EQ(line, expected.front());
//          expected.pop_front();
//      }
//  }
