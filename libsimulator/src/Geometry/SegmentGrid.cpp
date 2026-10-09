// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/SegmentGrid.hpp"

#include "AABB.hpp"
#include "GeometricFunctions.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>

Cell make_cell(Point p)
{
    return {floor(p.x / cell_extend) * cell_extend, floor(p.y / cell_extend) * cell_extend};
}

bool is_n4_adjacent(const Cell& a, const Cell& b)
{
    const auto dx = static_cast<int>(abs(a.x - b.x) / cell_extend);
    const auto dy = static_cast<int>(abs(a.y - b.y) / cell_extend);
    return dx + dy == 1;
}

std::set<Cell> cells_from_line_segment(LineSegment ls)
{
    const auto first_cell = make_cell(ls.p1);
    const auto last_cell = make_cell(ls.p2);
    if(first_cell == last_cell) {
        return {first_cell};
    }

    if(is_n4_adjacent(first_cell, last_cell)) {
        return {first_cell, last_cell};
    }

    std::set<Cell> cells{first_cell, last_cell};

    const auto to_multiple = [](double x) { return ceil(x / cell_extend) * cell_extend; };
    const AABB bounds(ls.p1, ls.p2);
    const auto vec_p1p2 = ls.p2 - ls.p1;
    std::vector<Point> intersections{};
    for(double x_intersect = to_multiple(bounds.xmin); x_intersect <= bounds.xmax;
        x_intersect += cell_extend) {
        const double fact = (x_intersect - ls.p1.x) / vec_p1p2.x;
        intersections.emplace_back(x_intersect, ls.p1.y + fact * vec_p1p2.y);
    }
    for(double y_intersect = to_multiple(bounds.ymin); y_intersect <= bounds.ymax;
        y_intersect += cell_extend) {
        const double fact = (y_intersect - ls.p1.y) / vec_p1p2.y;
        intersections.emplace_back(ls.p1.x + fact * vec_p1p2.x, y_intersect);
    }
    std::sort(std::begin(intersections), std::end(intersections));
    for(size_t index = 1; index < intersections.size(); ++index) {
        cells.insert(make_cell((intersections[index - 1] + intersections[index]) / 2));
    }
    return cells;
}
