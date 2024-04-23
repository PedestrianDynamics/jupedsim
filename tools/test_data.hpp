
#pragma once

#include <tuple>
#include <vector>

namespace help
{
using Point = std::tuple<float, float>;
using Line = std::tuple<Point, Point>;

using TestCase = std::vector<std::tuple<std::vector<Line>, Point>>;

const TestCase cases = {

    {{{{4.6000000000000005, 0.0}, {4.6000000000000005, 4.6000000000000005}},
      {{4.6000000000000005, 4.6000000000000005}, {0.0, 4.6000000000000005}},
      {{0.0, 4.6000000000000005}, {0.0, 0.0}},
      {{0.0, 0.0}, {4.6000000000000005, 0.0}}},
     {2.3, 2.3}}

};
} // namespace help
