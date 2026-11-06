// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <concepts>

/// Conversion from @p From to @p To, used by into().
/// Specialise to enable a conversion, e.g.
/// ```
///     template <>
///     struct Convert<Point, epick::Point2D> {
///         static epick::Point2D from(const Point& p) { return p.cgal(); }
///     };
/// ```
/// @tparam From type to convert from
/// @tparam To type to convert to
template <typename From, typename To>
struct Convert;

template <typename T>
struct Convert<T, T> {
    static T from(const T& value) { return value; }
};

template <typename From, typename To>
concept ConvertibleInto = requires(const From& from) {
    { Convert<From, To>::from(from) } -> std::same_as<To>;
};

/// Convert @p from to @p To, if `Convert<From, To>` is specialised.
/// @tparam To type to convert to
/// @tparam From type to convert from, deduced from @p from
/// @param from value to convert
/// @return the converted value
template <typename To, typename From>
    requires ConvertibleInto<From, To>
To into(const From& from)
{
    return Convert<From, To>::from(from);
}
