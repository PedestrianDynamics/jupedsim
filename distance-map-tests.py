#! /usr/bin/env python

import dataclasses
import textwrap
from typing import List

import jinja2
import shapely


def cvt_poly(poly):
    coords = poly.exterior.coords[:]
    holes_coords = [hole.coords[:] for hole in poly.interiors]

    line_segments = []
    for start, end in zip(coords, coords[1:]):
        line_segments.append((start, end))

    for hole_coords in holes_coords:
        for start, end in zip(hole_coords, hole_coords[1:]):
            line_segments.append((start, end))

    return line_segments


class TestCase:
    def __init__(self, poly, exit):
        self.lines: list[(shapely.Point, shapely.Point)] = cvt_poly(poly)
        self.exit: shapely.Point = exit

    def __str__(self) -> str:
        repr = textwrap.dedent(
            f"""\
                {{
                {{ {",".join([f"{{{{{p1[0]}, {p1[1]}}}, {{{p2[0]}, {p2[1]}}}}}" for p1, p2 in self.lines])} }},
                {{ {self.exit.x}, {self.exit.y} }}
                }}        
            """
        )
        return repr


def generate_test_cases():
    return [
        TestCase(
            shapely.box(0, 0, 23 * 0.2, 23 * 0.2), shapely.Point([2.3, 2.3])
        ),
    ]


def generate_header(test_cases: List[TestCase]):
    environment = jinja2.Environment()
    template = environment.from_string(
        textwrap.dedent(
            """
    #pragma once

    #include <vector>
    #include <tuple>
    
    namespace help
    {
        using Point = std::tuple<float, float>;
        using Line = std::tuple<Point, Point>;
    
        using TestCase = std::vector<std::tuple<std::vector<Line>, Point>>;

        const TestCase cases = {
            {% for test_case in test_cases %}
                {{ test_case }}{{ ", " if not loop.last else "" }}
            {% endfor %}
        };
    }
    """
        )
    )
    return template.render(test_cases=test_cases)


if __name__ == "__main__":
    test_cases = generate_test_cases()
    print(generate_header(test_cases))
