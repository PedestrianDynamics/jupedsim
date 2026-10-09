// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

#include <limits>
#include <tuple>

class Point
{
public:
    double x{};
    double y{};

public:
    Point(double x = 0, double y = 0) : x(x), y(y) {};

    bool is_zero_length() const;

    /// Norm
    double norm() const;

    /// Norm square
    inline double norm_square() const { return scalar_product(*this); }

    /// normalized vector
    Point normalized() const;

    /// Return norm and direction in one call
    /// @return Norm and Normalized
    std::tuple<double, Point> norm_and_normalized() const;

    /// dot product
    inline double scalar_product(const Point& v) const { return x * v.x + y * v.y; }

    inline double cross_product(const Point& p) const { return determinant(p); }

    /// determinant of the square matrix formed by the vectors [ this, v]
    inline double determinant(const Point& v) const { return x * v.y - y * v.x; }

    Point transform_to_ellipse_coordinates(const Point& center, double cphi, double sphi) const;
    /// translation and rotation in cartesian system
    Point transform_to_cartesian_coordinates(const Point& center, double cphi, double sphi) const;
    /// rotate the vector by theta
    Point rotate(double ctheta, double stheta) const;

    /// Create a new vector rotated by +90 degree (ccw rotation)
    /// @return rotated vector
    Point rotate90_deg() const;

    /// Tests that the vector is length 1
    /// @return length == 1
    bool is_unit_length() const;

    // operators
    /// addition
    const Point operator+(const Point& p) const;
    /// substraction
    const Point operator-(const Point& p) const;
    /// equal
    bool operator==(const Point& p) const;
    /// not equal
    bool operator!=(const Point& p) const;
    /// Assignement
    Point& operator+=(const Point& p);
    /// unary negation operator
    Point operator-() const;
    bool operator<(const Point& rhs) const;

    bool operator>(const Point& rhs) const;

    bool operator<=(const Point& rhs) const;

    bool operator>=(const Point& rhs) const;
};

/// Euclidean distance between 'a' and 'b'
/// @param [in] Point a
/// @param [in] Point b
/// @return distance between 'a' and 'b'
double distance(const Point& a, const Point& b);

/// Squared euclidean distance between 'a' and 'b'
/// @param [in] Point a
/// @param [in] Point b
/// @return distance between 'a' and 'b'
double distance_squared(const Point& a, const Point& b);

/// multiplication
const Point operator*(const Point& p, const double f);
/// division
const Point operator/(const Point& p, const double f);

template <>
struct fmt::formatter<Point> {
    char presentation{'f'};

    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Point& p, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", p.x, p.y);
    }
};
