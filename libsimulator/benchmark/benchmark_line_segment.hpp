// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "line_segment.hpp"
#include "point.hpp"

#include <benchmark/benchmark.h>

template <class... Args>
void bm_dist_to_on_line(benchmark::State& state, Args&&... args)
{
    auto args_tuple = std::make_tuple(std::move(args)...);
    auto line_segment = std::get<0>(args_tuple);

    auto factor = state.range(0) / 100.;
    Point point(line_segment.p1 + (line_segment.p2 - line_segment.p1) * factor);

    for(auto _ : state) {
        line_segment.dist_to(point);
    }
}

template <class... Args>
void bm_dist_to(benchmark::State& state, Args&&... args)
{
    auto args_tuple = std::make_tuple(std::move(args)...);
    auto line_segment = std::get<LineSegment>(args_tuple);

    auto edge_point = line_segment.p2 + line_segment.normal_vec() * 3.;

    auto direction = edge_point - line_segment.p1;

    auto factor = state.range(0) / 100.;

    Point point(line_segment.p1 + direction * factor);

    for(auto _ : state) {
        line_segment.dist_to(point);
    }
}

// CGAL implementation
BENCHMARK_CAPTURE(
    bm_dist_to_on_line,
    dist_to_point_on_line,
    LineSegment(Point(-1.3, 2.1), Point(3.6, -4.4)))
    ->DenseRange(0, 100, 20);

BENCHMARK_CAPTURE(bm_dist_to, dist_to_point, LineSegment(Point(-1.3, 2.1), Point(3.6, -4.4)))
    ->DenseRange(-50, 150, 10)
    ->Arg(-100000)
    ->Arg(100000);
