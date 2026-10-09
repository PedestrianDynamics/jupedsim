// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"

#include "Point.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

std::tuple<double, double> into_tuple(const Point& p)
{
    return std::make_tuple(p.x, p.y);
}

Point into_point(const std::tuple<double, double>& p)
{
    return Point{std::get<0>(p), std::get<1>(p)};
}

std::vector<Point> into_points(const std::vector<std::tuple<double, double>>& in)
{
    std::vector<Point> points{};
    points.reserve(in.size());
    for(const auto& [x, y] : in) {
        points.emplace_back(x, y);
    }
    return points;
}

std::string clean_doc(std::string_view raw)
{
    // Tabs count as one column; docstrings in the bindings are indented with spaces.
    constexpr std::string_view whitespace = " \t\r";
    constexpr auto npos = std::string_view::npos;

    std::vector<std::string_view> lines{};
    for(auto rest = raw;;) {
        const auto newline = rest.find('\n');
        auto line = rest.substr(0, newline);
        const auto last = line.find_last_not_of(whitespace);
        lines.push_back(last == npos ? std::string_view{} : line.substr(0, last + 1));
        if(newline == npos) {
            break;
        }
        rest.remove_prefix(newline + 1);
    }

    // Lines are right-stripped, so every non-empty one has a non-whitespace character.
    auto margin = npos;
    for(size_t i = 1; i < lines.size(); ++i) {
        if(!lines[i].empty()) {
            margin = std::min(margin, lines[i].find_first_not_of(whitespace));
        }
    }
    for(size_t i = 1; i < lines.size(); ++i) {
        if(!lines[i].empty()) {
            lines[i].remove_prefix(margin);
        }
    }
    if(!lines[0].empty()) {
        lines[0].remove_prefix(lines[0].find_first_not_of(whitespace));
    }

    const auto first = std::ranges::find_if(lines, [](auto l) { return !l.empty(); });
    const auto last = std::ranges::find_if(lines.rbegin(), lines.rend(), [](auto l) {
                          return !l.empty();
                      }).base();
    std::string doc{};
    for(auto it = first; it < last; ++it) {
        if(it != first) {
            doc += '\n';
        }
        doc += *it;
    }
    return doc;
}
