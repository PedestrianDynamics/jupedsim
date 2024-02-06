// Copyright © 2012-2024 Forschungszentrum Jülich GmbH
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <benchmark/benchmark.h>

#include "LineSegment.hpp"

template <class... Args>
void bmDistToOnLine(benchmark::State& state, Args&&... args)
{
    auto args_tuple = std::make_tuple(std::move(args)...);
    auto lineSegment = std::get<0>(args_tuple);

    auto factor = state.range(0) / 100.;
    Point point(lineSegment.p1 + (lineSegment.p2 - lineSegment.p1) * factor);

    for(auto _ : state) {
        lineSegment.DistTo(point);
    }
}

template <class... Args>
void bmDistTo(benchmark::State& state, Args&&... args)
{
    auto args_tuple = std::make_tuple(std::move(args)...);
    auto lineSegment = std::get<LineSegment>(args_tuple);

    auto edgePoint = lineSegment.p2 + lineSegment.NormalVec() * 3.;

    auto direction = edgePoint - lineSegment.p1;

    auto factor = state.range(0) / 100.;

    Point point(lineSegment.p1 + direction * factor);

    for(auto _ : state) {
        lineSegment.DistTo(point);
    }
}

// CGAL implementation
BENCHMARK_CAPTURE(
    bmDistToOnLine,
    dist_to_point_on_line,
    LineSegment(Point(-1.3, 2.1), Point(3.6, -4.4)))
    ->DenseRange(0, 100, 20);

BENCHMARK_CAPTURE(bmDistTo, dist_to_point, LineSegment(Point(-1.3, 2.1), Point(3.6, -4.4)))
    ->DenseRange(-50, 150, 10)
    ->Arg(-100000)
    ->Arg(100000);
