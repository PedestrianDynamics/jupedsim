#! /usr/bin/env python3

import argparse
import sys
from pathlib import Path

import jupedsim
import matplotlib.pyplot as plt
import py_jupedsim as jps
import py_jupedsim.experimental as jpex
import shapely


class LineDrawer:
    def __init__(self, line, navi):
        self.line = line
        self.navi = navi
        self.cids = [
            self.line.figure.canvas.mpl_connect(
                "button_press_event", self.on_press
            ),
            self.line.figure.canvas.mpl_connect(
                "button_release_event", self.on_release
            ),
            self.line.figure.canvas.mpl_connect(
                "motion_notify_event", self.on_motion
            ),
        ]
        self.is_pressed = False
        self.line_from = (0.0, 0.0)
        self.line_to = (0.0, 0.0)

    def on_press(self, evt):
        if evt.inaxes != self.line.axes:
            return
        self.is_pressed = True
        self.line_from = (evt.xdata, evt.ydata)
        self.line_to = (evt.xdata, evt.ydata)
        self._draw()

    def on_release(self, evt):
        self.is_pressed = False
        self.line_from = (0.0, 0.0)
        self.line_to = (0.0, 0.0)
        self._draw()

    def on_motion(self, evt):
        if not self.is_pressed:
            return
        self.line_to = (evt.xdata, evt.ydata)
        self._draw()

    def _draw(self):
        if self.navi.is_routable(self.line_from) and self.navi.is_routable(
            self.line_to
        ):
            route = self.navi.compute_waypoints(self.line_from, self.line_to)
            x_coords = []
            y_coords = []
            for x, y in route:
                x_coords.append(x)
                y_coords.append(y)
            self.line.set_data(x_coords, y_coords)
        else:
            self.line.set_data([0, 0])
        self.line.figure.canvas.draw_idle()


def get_colormap(nbr_elements, colormap="tab20"):
    return plt.cm.get_cmap(colormap, nbr_elements)


def plot(navi: jpex.RoutingEngine):
    fig = plt.figure()
    ax = fig.subplots()
    mesh = navi.mesh()
    colormap = get_colormap(len(mesh))
    for idx, triangle in enumerate(mesh):
        color = colormap(idx)
        x = [triangle[0][0], triangle[1][0], triangle[2][0]]
        y = [triangle[0][1], triangle[1][1], triangle[2][1]]
        ax.fill(
            x,
            y,
            fill=True,
            alpha=0.5,
            c=color,
            linewidth=1.0,
            zorder=1,
            label=str(idx),
        )
        for edge in navi.edges_for(idx):
            x = [edge[0][0], edge[1][0]]
            y = [edge[0][1], edge[1][1]]
            ax.plot(x, y, c="black")

    ax.legend()
    (line,) = ax.plot([0, 0])
    line_drawer = LineDrawer(line, navi)
    plt.show(block=True)


def parse_args() -> argparse.Namespace:
    argParser = argparse.ArgumentParser(
        description="Plots the supplied geometry and visualises the applied triangulation.\n"
        "Use Click and drag to show paths created by the routing system."
    )
    argParser.add_argument(
        "geometry", nargs=1, type=Path, help="path to geometry data of TYPE"
    )
    argParser.add_argument(
        "--type",
        choices=["dlr-ad-hoc", "WKT"],
        default="WKT",
        help="Specifies the geometry input format",
    )
    return argParser.parse_args()


def build_jps_geometry(geo: shapely.GeometryCollection):
    geo_builder = jps.GeometryBuilder()

    for obj in geo.geoms:
        if obj.geom_type != "Polygon":
            print(
                f"Unexpected geometry type found in GeometryCollection: {obj.type}",
                file=sys.stderr,
            )
            sys.exit(1)
        geo_builder.add_accessible_area(obj.exterior.coords[:-1])
        for hole in obj.interiors:
            geo_builder.exclude_from_accssible_area(hole.coords[:-1])
    return geo_builder.build()


def main() -> None:
    args = parse_args()
    geo_file_path = args.geometry[0]
    if not geo_file_path.exists():
        print(f"Input file {geo_file_path} not found", file=sys.stderr)
        sys.exit(1)
    input = geo_file_path.read_text()
    geo = None
    if args.type == "WKT":
        geo = jupedsim.serialization.parse_wkt(input)
    elif args.type == "dlr-ad-hoc":
        geo = jupedsim.serialization.parse_dlr_ad_hoc(input)
    else:
        # This should not happen :)
        print(f"Unknown input type provided: {args.type}", file=sys.stderr)
        sys.exit(1)
    jps_geo = build_jps_geometry(geo)
    jps_navi = jpex.RoutingEngine(jps_geo)
    plot(jps_navi)


if __name__ == "__main__":
    main()
