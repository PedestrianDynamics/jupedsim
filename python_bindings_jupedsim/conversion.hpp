// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <Point.hpp>

#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

std::tuple<double, double> into_tuple(const Point& p);

// Works for container of Point type, but also e.g. for container of glm::vec<2>
std::vector<std::tuple<double, double>> into_tuples(const auto& in)
{
    std::vector<std::tuple<double, double>> tuples{};
    tuples.reserve(in.size());
    for(const auto& pt : in) {
        tuples.emplace_back(pt.x, pt.y);
    }
    return tuples;
}

Point into_point(const std::tuple<double, double>& p);

std::vector<Point> into_points(const std::vector<std::tuple<double, double>>& in);

/// Normalizes the whitespace of a docstring written as an indented raw string literal.
///
/// Behaves like Python's `inspect.cleandoc()` and additionally strips trailing whitespace:
/// - removes leading whitespace from the first line,
/// - removes the indentation common to all non-blank lines after the first,
/// - strips trailing whitespace (spaces, tabs, `\r`) from every line,
/// - drops blank lines at the start and the end.
///
/// Relative indentation (code blocks, `Args:` sections) is preserved; text is not changed
/// otherwise. Tabs count as one column of indentation.
///
/// Typical use when binding, where pybind11 copies the docstring before the temporary is
/// destroyed:
/// @code
/// .def("name", &Fn, cleanDoc(R"(
///     Summary line.
///
///     Details.
/// )").c_str())
/// @endcode
///
/// @param raw Docstring as written in the source; not copied.
/// @return The cleaned docstring, lines joined with `\n`; empty if @p raw is blank.
std::string clean_doc(std::string_view raw);

template <typename Range>
auto into_vec(Range&& range)
{
    using Value = std::remove_cvref_t<decltype(*std::begin(range))>;

    std::vector<Value> result{};
    if constexpr(std::ranges::sized_range<Range>) {
        result.reserve(std::ranges::size(range));
    }
    for(auto&& value : range) {
        result.emplace_back(value);
    }
    return result;
}

template <typename T, typename U>
std::vector<T> into_vec_t(const std::vector<U>& vec)
{
    auto result = std::vector<T>();
    result.reserve(vec.size());
    for(const auto& v : vec) {
        result.emplace_back(v);
    }
    return result;
}
