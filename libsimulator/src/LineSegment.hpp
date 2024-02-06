// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Point.hpp"

#include <string>
#include <vector>

class LineSegment
{
public:
    Point p1{};
    Point p2{};

    LineSegment() = default;

    LineSegment(Point p1, Point p2);

    ~LineSegment() = default;

    bool operator==(const LineSegment& other) const;
    bool operator!=(const LineSegment& other) const;
    bool operator<(const LineSegment& other) const;

    Point NormalVec() const;

    /**
     *TODO: FIXME
     */
    double NormalComp(const Point& v) const; // Normale Komponente von v auf l

    /**
     * Note that that result must not lie on the segment
     * @return the orthogonal projection of p on the line defined by the segment points.
     */
    Point LotPoint(const Point& p) const;

    /**
     * @return the point on the segment with the minimum distance to p
     */
    Point ShortestPoint(const Point& p) const;

    /**
     * @return the distance from the line to the point p
     */
    double DistTo(const Point& p) const;

    /**
     * @return the lenght square of  the segment
     */
    double LengthSquare() const;
};

template <>
struct fmt::formatter<LineSegment> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const LineSegment& ls, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "LineSegment({}, {})", ls.p1, ls.p2);
    }
};
