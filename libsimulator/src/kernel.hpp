// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <logger.hpp>

#include <cmath>
#include <limits>
#include <tuple>

using K = CGAL::Exact_predicates_inexact_constructions_kernel;

class Point;

#if defined(_MSC_VER)
#define JPS_COLD_NOINLINE __declspec(noinline)
#else
#define JPS_COLD_NOINLINE [[gnu::cold, gnu::noinline]]
#endif

/// Out of line and cold: keeps the formatting code out of every inlined division.
JPS_COLD_NOINLINE inline void warn_small_divisor(double f)
{
    LOG_WARNING("Vector::operator/ dividend {} is too small. Using 1 instead.", f);
}

/// Direction, force, velocity or offset. Defaults to the zero vector.
class Vector
{
    K::Vector_2 _v{0.0, 0.0};

public:
    Vector() = default;
    Vector(double x, double y) : _v(x, y) {}
    explicit Vector(const K::Vector_2& v) : _v(v) {}

    double x() const { return _v.x(); }
    double y() const { return _v.y(); }
    const K::Vector_2& cgal() const { return _v; }

    bool is_zero_length() const
    {
        constexpr double epsilon = 1e-6;
        return std::abs(x()) < epsilon && std::abs(y()) < epsilon;
    }
    bool is_unit_length() const
    {
        constexpr double tolerance = 1e-9;
        return std::abs(1 - norm_square()) <= tolerance;
    }
    double norm_square() const { return _v.squared_length(); }
    double norm() const { return std::sqrt(norm_square()); }
    /// @return norm and direction; (0, (0, 0)) for a vector of length <= epsilon
    std::tuple<double, Vector> norm_and_normalized() const
    {
        const double n = norm();
        if(n > std::numeric_limits<double>::epsilon()) {
            return {n, Vector{_v / n}};
        }
        return {0.0, Vector{}};
    }
    Vector normalized() const
    {
        const double n = norm();
        return n > std::numeric_limits<double>::epsilon() ? Vector{_v / n} : Vector{};
    }
    double scalar_product(const Vector& v) const { return CGAL::scalar_product(_v, v._v); }
    double cross_product(const Vector& v) const { return determinant(v); }
    double determinant(const Vector& v) const { return CGAL::determinant(_v, v._v); }
    /// Rotate ccw by the angle with cosine @p c and sine @p s
    Vector rotate(double c, double s) const { return {x() * c - y() * s, x() * s + y() * c}; }
    Vector rotate90_deg() const { return Vector{_v.perpendicular(CGAL::COUNTERCLOCKWISE)}; }
    /// From the local system of an ellipse at @p center back to world coordinates
    Point transform_to_cartesian_coordinates(const Point& center, double cphi, double sphi) const;

    Vector operator+(const Vector& v) const { return Vector{_v + v._v}; }
    Vector operator-(const Vector& v) const { return Vector{_v - v._v}; }
    Vector operator-() const { return Vector{-_v}; }
    Vector operator*(double f) const { return Vector{_v * f}; }
    /// Returns the vector unchanged and logs a warning unless @p f > epsilon^2.
    Vector operator/(double f) const
    {
        static constexpr auto eps =
            std::numeric_limits<double>::epsilon() * std::numeric_limits<double>::epsilon();
        if(f > eps) {
            return Vector{_v / f};
        }
        warn_small_divisor(f);
        return *this;
    }
    Vector& operator+=(const Vector& v)
    {
        _v += v._v;
        return *this;
    }
    bool operator==(const Vector& v) const { return x() == v.x() && y() == v.y(); }
    bool operator!=(const Vector& v) const { return !(*this == v); }
};

/// Absolute location. Defaults to the origin.
class Point
{
    K::Point_2 _p{0.0, 0.0};

public:
    Point() = default;
    Point(double x, double y) : _p(x, y) {}
    explicit Point(const K::Point_2& p) : _p(p) {}

    double x() const { return _p.x(); }
    double y() const { return _p.y(); }
    const K::Point_2& cgal() const { return _p; }

    /// Into the local system of an ellipse at @p center, rotated by -phi
    Vector transform_to_ellipse_coordinates(const Point& center, double cphi, double sphi) const
    {
        return (*this - center).rotate(cphi, -sphi);
    }

    Vector operator-(const Point& p) const { return Vector{_p - p._p}; }
    Point operator+(const Vector& v) const { return Point{_p + v.cgal()}; }
    Point operator-(const Vector& v) const { return Point{_p - v.cgal()}; }
    bool operator==(const Point& p) const { return x() == p.x() && y() == p.y(); }
    bool operator!=(const Point& p) const { return !(*this == p); }
    /// Lexicographic: x, then y
    bool operator<(const Point& p) const { return x() < p.x() || (x() == p.x() && y() < p.y()); }
};

inline Point
Vector::transform_to_cartesian_coordinates(const Point& center, double cphi, double sphi) const
{
    return center + rotate(cphi, sphi);
}

/// The vector from the origin to @p p
inline Vector offset_from_origin(const Point& p)
{
    return {p.x(), p.y()};
}

/// The point reached from the origin by @p v
inline Point at_offset(const Vector& v)
{
    return {v.x(), v.y()};
}

inline Point midpoint(const Point& a, const Point& b)
{
    return Point{CGAL::midpoint(a.cgal(), b.cgal())};
}

inline double distance_squared(const Point& a, const Point& b)
{
    return (a - b).norm_square();
}

inline double distance(const Point& a, const Point& b)
{
    return (a - b).norm();
}

template <>
struct fmt::formatter<Vector> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Vector& v, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", v.x(), v.y());
    }
};

template <>
struct fmt::formatter<Point> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const Point& p, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "({}, {})", p.x(), p.y());
    }
};
