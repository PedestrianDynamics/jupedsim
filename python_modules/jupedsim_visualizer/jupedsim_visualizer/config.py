# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
def web_color(color_as_hex_string: str) -> list[float]:
    value = int(color_as_hex_string, 16)
    red = (value & 0xFF0000) >> 16
    green = (value & 0x00FF00) >> 8
    blue = value & 0x0000FF
    return [red / 255, green / 255, blue / 255]


class Colors:
    a = web_color("c7522a")
    b = web_color("e5c185")
    c = web_color("fbf2c4")
    d = web_color("74a892")
    e = web_color("008585")
    agent = web_color("ffb366")


class ZLayers:
    geo = 0
    graph_edges = 5
    grid = 10
    agents = 90
    nav_line = 91
