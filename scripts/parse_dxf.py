import argparse
import logging
import pathlib
from argparse import RawTextHelpFormatter
from typing import List

import ezdxf
import geopandas as gpd
import matplotlib.pyplot
import matplotlib.pyplot as plt
import shapely
from shapely import (
    GeometryCollection,
    LineString,
    MultiPolygon,
    Point,
    Polygon,
    polygonize,
    to_wkt,
)

logging.basicConfig(format="%(levelname)s:%(message)s", level=logging.DEBUG)


class IncorrectDXFFileError(Exception):
    def __init__(self, message, geometries=None):
        self.message = message
        self.geometries = geometries
        super().__init__(self.message)


def save_as_wkt(geometry, out_file: pathlib.Path):
    """
    converts geometry into wkt and writes them in out_file
    @param geometry: shapely geometry
    @param out_file: The file to write the output to.
    """

    # create result dir
    out_file.parent.mkdir(parents=True, exist_ok=True)

    geometry_collection = GeometryCollection(geometry)
    with open(out_file, "w") as out:
        out.write(to_wkt(geometry_collection, rounding_precision=-1))

    logging.info(f"wkt was written to: {out_file.absolute()}")


def plot_polygon(polygon: shapely.Polygon):
    """plots a polygon with its interior in matplotlib"""
    poly = gpd.GeoSeries([polygon])
    poly.plot()
    plt.show()


def line_to_linestring(line):
    """converts an entity with dxftype line to a shapely Linestring"""
    start_point = line.dxf.start
    end_point = line.dxf.end
    return LineString(
        [(start_point[0], start_point[1]), (end_point[0], end_point[1])]
    )


def multipolygon_to_list(multipolygon: shapely.MultiPolygon):
    """converts a multipolygon to a list of polygons"""
    return [poly for poly in multipolygon.geoms]


def polyline_to_linestring(polyline):
    """converts an entity with dxftype polyline to a shapely Linestring"""
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])

    # if the polyline is closed this resembles a polygon
    # to create a closed shapely linestring the last point must correspond to
    # the first point
    if polyline.closed:
        points.append(points[0])
    return LineString(points)


def polyline_to_polygon(polyline):
    """converts an entity with dxftype polyline to a shapely Polygon"""
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])

    if len(points) < 3:
        logging.error(
            "a polyline has at most 2 points and can not be a Polygon"
        )
        raise IncorrectDXFFileError(
            f"a polyline could not be converted to a Polygon", [points]
        )
    return Polygon(points)


def dxf_circle_to_shply(dxf_circle, quad_segs):
    """converts an entity with dxftype circle to a shapely Point with buffer
    @param dxf_circle: an entity with dxf-type circle
    @param quad_segs: Sets the number of line segments used to approximate an
                      angle fillet
    """
    pt = dxf_circle.dxf.center
    radius = dxf_circle.dxf.radius
    return Point(pt).buffer(radius, quad_segs)


def parse_dxf_file(
    dxf_path: pathlib.Path,
    outer_line_layer: str,
    hole_layers: List[str],
    quad_segs: int,
):
    """parses a dxf-file and creates a shapely structure resembling the file
    @param dxf_path: Path to the DXF file
    @param outer_line_layer: the name of the layer in the dxf-file where the
                             outer polygon is defined
    @param hole_layers: a list with all layer names in the dxf-file where holes
                        are defined
    @param quad_segs: Specifies the number of linear segments in a quarter
                      circle in the approximation of circular arcs.

    @return: shapely polygon or multipolygon from dxf-file
    """
    holes = []
    outer_lines = []
    # Open the DXF file
    doc = ezdxf.readfile(dxf_path)
    # Access the model (modelspace)
    msp = doc.modelspace()

    # Iterate over all entities in the model
    for entity in msp:
        if entity.dxftype() == "LWPOLYLINE":
            if not entity.closed:
                logging.error(
                    f"There is a Polygon in layer {entity.dxf.layer} "
                    f"that is not closed. This may cause issues "
                    f"creating the polygon."
                )
            if entity.dxf.layer in hole_layers:
                holes.append(polyline_to_polygon(entity))
            elif entity.dxf.layer == outer_line_layer:
                outer_lines.append(polyline_to_linestring(entity))
        elif entity.dxftype() == "CIRCLE":
            if entity.dxf.layer in hole_layers:
                holes.append(dxf_circle_to_shply(entity, quad_segs))
            else:
                logging.warning(
                    f"there is a circle defined in Layer {entity.dxf.layer} "
                    f"at {entity.dxf.center}. This is not valid and will be "
                    f"ignored."
                )

        elif entity.dxftype() != "INSERT":
            logging.warning(
                f"there is an entity of type {entity.dxftype()} defined which "
                f"will not be parsed."
            )

    # create a polygon from all outer lines
    outer_polygons = []
    for line in outer_lines:
        outer_polygons.append(Polygon(line.coords))
    outer_polygon = polygonize(outer_lines)

    # separate simple and not simple holes
    simple_holes = []
    other_holes = []
    for hole in holes:
        simple_holes.append(hole) if hole.is_simple else other_holes.append(
            hole
        )

    if len(other_holes) > 0:
        logging.error(
            f"{len(other_holes)} not simple polygons were parsed. These are "
            f"not supported."
        )
        raise IncorrectDXFFileError(
            "The file contained at least one not simple Polygon.", other_holes
        )

    # create new Polygon with holes
    simple_holes = polygonize(simple_holes)
    logging.debug(f"the geometry was parsed")
    return outer_polygon.difference(simple_holes)


def shapely_to_dxf(
    geometry,
    dxf_path,
    walkable_layer="walkable_layer",
    hole_layer="hole_layer",
):
    """creates a dxf file according to the geometry
    @param geometry: shapely Polygon or Multipolygon
    @param dxf_path: path to where the created dxf file should be saved to
    @param walkable_layer: name of the layer containing the walkable area
    @param hole_layer: name of the layer containing the holes of the geometry
    """

    def add_polyline(polyline, layername):
        points = list(polyline.coords)
        modelspace.add_lwpolyline(points, dxfattribs={"layer": layername})

    doc = ezdxf.new("R2010")
    modelspace = doc.modelspace()

    if isinstance(geometry, Polygon):
        geometry = [geometry]
    elif isinstance(geometry, MultiPolygon):
        geometry = geometry.geoms

    for polygon in geometry:
        if not polygon.is_empty:
            add_polyline(polygon.exterior, walkable_layer)
            for hole in polygon.interiors:
                add_polyline(hole, hole_layer)

    doc.saveas(dxf_path)
    logging.info(f"dxf was written to: {dxf_path.absolute()}")


def parse_args():
    parser = argparse.ArgumentParser(
        description="""
Converts the given dxf file to wkt and saves it in a new file. Can also save \
the resulting geometry in a dxf file and plot the geometry.

Following constraints need to be fulfilled that the conversion can succeed:

    * outer and inner polygon are defined on different layers
    * the definition of the inner polygon (holes) can be distributed on \
several layers
    * the definition of the outer polygon must be on one layer
    * a hole is defined with ONE polyline or ONE circle
      * there are no holes, which are composed of multiple lines/polylines
      * keep in mind that a circle is considered a polygon with many corners. \
This might make the triangulation too accurate and slow down the \
simulation unnecessarily.
      * circles touching other elements might be approximated different than \
expected
    * Holes may overlap
    * Holes may be defined outside the outer polygon
      * this does not change the structure of the polygon
    * the outer polygon must be defined with one closed polyline
    * if there are multiple polygons on the outer polygon a MultiPolygon will \
be parsed

The polygon should end up looking like the structure in the dxf file:

    * There must be no gaps in the outer polygon or in a hole
    * There must be no double lines
    * all areas must be wide enough for the agents
    * all polygons must be simple""",
        formatter_class=RawTextHelpFormatter,
    )
    parser.add_argument(
        "-i",
        "--input",
        help="dxf file to parse",
        required=True,
        type=pathlib.Path,
    )
    parser.add_argument(
        "-o",
        "--output",
        help="result file containing the wkt (if none given: INPUT.wkt)",
        type=pathlib.Path,
    )
    parser.add_argument(
        "-d",
        "--dxf-output",
        help="result file containing the wkt",
        type=pathlib.Path,
        required=False,
    )

    parser.add_argument(
        "-w",
        "--walkable",
        help="layer containing the walkable area",
        required=True,
    )
    parser.add_argument(
        "-x", "--obstacles", help="layer(s) containing obstacles", nargs="+"
    )

    parser.add_argument(
        "-q",
        "--quad-segments",
        help="specifies the number of linear segments in a quarter circle in "
        "the approximation of circular arcs (default: 4)",
        default=4,
    )
    parser.add_argument(
        "-p",
        "--plot",
        help="plot the parsed polygon in an interactive window",
        action="store_true",
    )
    return parser.parse_args()


def main():
    parsed_args = parse_args()

    # parse polygon(s)
    merged_polygon = parse_dxf_file(
        parsed_args.input,
        parsed_args.walkable,
        parsed_args.obstacles,
        parsed_args.quad_segments,
    )

    # create a dxf file using the parsed geometry
    if parsed_args.dxf_output:
        shapely_to_dxf(merged_polygon, parsed_args.dxf_output)

    # plot final Polygon
    if parsed_args.plot:
        matplotlib.pyplot.set_loglevel("warning")
        logging.getLogger("PIL.PngImagePlugin").setLevel(logging.WARNING)

        if merged_polygon.geom_type == "Polygon":
            plot_polygon(merged_polygon)
        if merged_polygon.geom_type == "MultiPolygon":
            merged_polygon = multipolygon_to_list(merged_polygon)
            for poly in merged_polygon:
                plot_polygon(poly)

    # now cast to wkt and write into a file
    out_file = (
        parsed_args.output
        if parsed_args.output is not None
        else parsed_args.input.with_suffix(".wkt")
    )
    save_as_wkt(merged_polygon, out_file=out_file)


if __name__ == "__main__":
    main()
