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

Cell makeCell(Point p)
{
    return {floor(p.x / CELL_EXTEND) * CELL_EXTEND, floor(p.y / CELL_EXTEND) * CELL_EXTEND};
}

bool IsN8Adjacent(const Cell& a, const Cell& b)
{
    const auto dx = static_cast<int>(abs(a.x - b.x) / CELL_EXTEND);
    const auto dy = static_cast<int>(abs(a.y - b.y) / CELL_EXTEND);
    if((dx == 0 && dy == 0) || dx > 1 || dy > 1) {
        return false;
    }
    return true;
}

std::set<Cell> cellsFromLineSegment(LineSegment ls)
{
    const auto firstCell = makeCell(ls.p1);
    const auto lastCell = makeCell(ls.p2);
    if(firstCell == lastCell) {
        return {firstCell};
    }

    if(IsN8Adjacent(firstCell, lastCell)) {
        return {firstCell, lastCell};
    }

    std::set<Cell> cells{firstCell, lastCell};

    const auto toMultiple = [](double x) { return ceil(x / CELL_EXTEND) * CELL_EXTEND; };
    const AABB bounds(ls.p1, ls.p2);
    const auto vec_p1p2 = ls.p2 - ls.p1;
    std::vector<Point> intersections{};
    for(double x_intersect = toMultiple(bounds.xmin); x_intersect <= bounds.xmax;
        x_intersect += CELL_EXTEND) {
        const double fact = (x_intersect - ls.p1.x) / vec_p1p2.x;
        intersections.emplace_back(x_intersect, ls.p1.y + fact * vec_p1p2.y);
    }
    for(double y_intersect = toMultiple(bounds.ymin); y_intersect <= bounds.ymax;
        y_intersect += CELL_EXTEND) {
        const double fact = (y_intersect - ls.p1.y) / vec_p1p2.y;
        intersections.emplace_back(ls.p1.x + fact * vec_p1p2.x, y_intersect);
    }
    std::sort(std::begin(intersections), std::end(intersections));
    for(size_t index = 1; index < intersections.size(); ++index) {
        cells.insert(makeCell((intersections[index - 1] + intersections[index]) / 2));
    }
    return cells;
}
