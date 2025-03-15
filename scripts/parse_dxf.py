# SPDX-License-Identifier: LGPL-3.0-or-later
import argparse
import logging
import pathlib
from argparse import RawTextHelpFormatter
from typing import List

import ezdxf
import geopandas as gpd
import matplotlib.patches as mpatches
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
    # return list(multipolygon.geoms)


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
            "a polyline could not be converted to a Polygon", [points]
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


def layer_exists(layer_name: str, layers_in_dxf: List[str]) -> bool:
    """Check if a given layer exists in the DXF file, allowing for sublayers."""
    return any(layer_name in layer for layer in layers_in_dxf)


def validate_layers(
    layers_in_dxf: List[str], layer_list: List[str], category: str
) -> List[str]:
    """Log missing layers and returns only existing layers."""
    matched_layers = [
        layer for layer in layer_list if layer_exists(layer, layers_in_dxf)
    ]
    missing_layers = [
        layer for layer in layer_list if not layer_exists(layer, layers_in_dxf)
    ]

    if missing_layers:
        logging.warning(
            f"These {category} layers were not found in the DXF file: {missing_layers}."
        )
        logging.warning(f"Available layers are: {layers_in_dxf}.")

    return matched_layers


def parse_dxf_file(
    dxf_path: pathlib.Path,
    outer_line_layer: str,
    hole_layers: List[str],
    exit_layers: List[str],
    distribution_layers: List[str],
    quad_segs: int,
):
    """parses a dxf-file and creates a shapely structure resembling the file
    @param dxf_path: Path to the DXF file
    @param outer_line_layer: the name of the layer in the dxf-file where the
                             outer polygon is defined
    @param hole_layers: a list with all layer names in the dxf-file where holes
                        are defined
    @param exit_layers: a list with all layer names in the dxf-file where exits
                        are defined
    @param distribution_layers: a list with all layer names in the dxf-file where distributions
                        are defined

    @param quad_segs: Specifies the number of linear segments in a quarter
                      circle in the approximation of circular arcs.

    @return: shapely polygon or multipolygon from dxf-file
    """
    doc = ezdxf.readfile(dxf_path)

    holes = []
    exits = []
    distributions = []
    outer_lines = []
    # Open the DXF file

    # Access the model (modelspace)
    msp = doc.modelspace()
    layers_in_dxf = [layer.dxf.name for layer in doc.layers]
    logging.info(f"{outer_line_layer = }")

    if not layer_exists(outer_line_layer, layers_in_dxf):
        raise IncorrectDXFFileError(
            f"Layer '{outer_line_layer}' not found in DXF file.\n Available layers are: {layers_in_dxf}"
        )
    hole_layers = validate_layers(layers_in_dxf, hole_layers, "hole")
    exit_layers = validate_layers(layers_in_dxf, exit_layers, "exit")
    distribution_layers = validate_layers(
        layers_in_dxf, distribution_layers, "distribution"
    )
    # Iterate over all entities in the model
    for entity in msp:
        if entity.dxftype() == "LWPOLYLINE":
            if not entity.closed and (
                entity.dxf.layer in hole_layers
                or entity.dxf.layer in exit_layers
                or entity.dxf.layer in distribution_layers
                or outer_line_layer in entity.dxf.layer
            ):
                logging.error(
                    f"There is a Polygon in layer <{entity.dxf.layer}> "
                    f"that is not closed. This may cause issues "
                    f"creating the polygon."
                )
                points = [(p[0], p[1]) for p in entity.get_points()]
                shape = LineString(points)
                logging.error(shape)
                logging.error("-" * 50)
                raise IncorrectDXFFileError(
                    f"Unclosed polyline in layer <{entity.dxf.layer}>"
                )
            if any(
                hole_layer in entity.dxf.layer for hole_layer in hole_layers
            ):
                holes.append(polyline_to_polygon(entity))
            elif any(
                exit_layer in entity.dxf.layer for exit_layer in exit_layers
            ):
                exits.append(polyline_to_polygon(entity))
            elif outer_line_layer in entity.dxf.layer:
                outer_lines.append(polyline_to_linestring(entity))
            elif any(
                distribution_layer in entity.dxf.layer
                for distribution_layer in distribution_layers
            ):
                distributions.append(polyline_to_polygon(entity))
            elif outer_line_layer in entity.dxf.layer:
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

    logging.debug(f"Found {len(outer_lines)} outer lines.")
    outer_polygon = polygonize(outer_lines)
    if not outer_polygon:
        raise IncorrectDXFFileError("Could not create an outer polygon.")

    # separate simple and not simple holes
    simple_holes = []
    other_holes = []
    for hole in holes:
        (
            simple_holes.append(hole)
            if hole.is_simple
            else other_holes.append(hole)
        )

    if len(other_holes) > 0:
        logging.error(
            f"{len(other_holes)} not simple polygons were parsed. These are "
            f"not supported."
        )

        raise IncorrectDXFFileError(
            "The file contained at least one not simple hole polygon.",
            other_holes,
        )

    # create new Polygon with holes
    simple_holes = polygonize(simple_holes)

    # separate simple and not simple exits
    simple_exits = []
    other_exits = []
    for _exit in exits:
        (
            simple_exits.append(_exit)
            if _exit.is_simple
            else other_exits.append(_exit)
        )

    if len(other_exits) > 0:
        logging.error(
            f"{len(other_exits)} not simple polygons were parsed. These are "
            f"not supported."
        )
        raise IncorrectDXFFileError(
            "The file contained at least one not simple exit polygon.",
            other_exits,
        )

    # create new Polygon with exits
    simple_exits = polygonize(simple_exits)

    # separate simple and not simple distribution
    simple_distributions = []
    other_distributions = []
    for distribution in distributions:
        (
            simple_distributions.append(distribution)
            if distribution.is_simple
            else other_distributions.append(distribution)
        )

    if len(other_distributions) > 0:
        logging.error(
            f"{len(other_distributions)} not simple polygons were parsed. These are "
            f"not supported."
        )
        raise IncorrectDXFFileError(
            "The file contained at least one not simple distribution polygon.",
            other_distributions,
        )

    # create new Polygon with exits
    simple_distributions = polygonize(simple_distributions)

    logging.info("The geometry was parsed:")
    logging.info(f">> Got {len(list(simple_holes.geoms))} holes.")
    logging.info(f">> Got {len(list(simple_exits.geoms))} exits.")
    logging.info(
        f">> Got {len(list(simple_distributions.geoms))} distributions."
    )
    logging.info(f">> Got {len(list(outer_polygon.geoms))} outer polygons.")
    if not outer_polygon or outer_polygon.is_empty:
        logging.error(
            "The outer polygon is empty. Returning an empty geometry."
        )
        return GeometryCollection()

    try:
        result = outer_polygon.difference(simple_holes)
        if result.is_empty:
            logging.warning(
                "The difference operation resulted in an empty geometry."
            )
            return outer_polygon  # return the outer polygon without holes
        walkable_area_collection = GeometryCollection(result)
        exits_collection = GeometryCollection(simple_exits)
        distributions_collection = GeometryCollection(simple_distributions)
        return GeometryCollection(
            [
                walkable_area_collection,
                exits_collection,
                distributions_collection,
            ]
        )

    except Exception as e:
        logging.error(f"Error while computing difference: {e}")
        return outer_polygon


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
Converts a DXF file into WKT format and saves it as a new file.

        Optionally,
the resulting geometry can be exported back to DXF or visualized as a plot.

### Requirements for Successful Conversion:
- The outer and inner polygons must be defined in different layers.
- The outer polygon must be in a single designated layer.
- Inner polygons (holes) can be distributed across multiple layers.
- A hole must be defined by a **single polyline** or **single circle**:
  - Holes **cannot** be composed of multiple disconnected lines/polylines.
  - Circles are converted to polygons with many corners, which may impact triangulation accuracy.
  - If circles touch other elements, they might be approximated differently than expected.
- Holes may:
  - **Overlap with each other**.
  - **Be located outside the outer polygon** (this does not affect the final structure).
- The outer polygon **must be a closed polyline**.
- If multiple polygons exist in the outer polygon layer, they will be parsed as a **MultiPolygon**.

### Expected Structure in DXF:
- No gaps in the outer polygon or holes.
- No duplicate lines.
- Sufficiently wide areas for agents.
- All polygons must be **simple** (i.e., not self-intersecting).

### Structure of the Output Geometry:
The resulting geometry is structured as a **GeometryCollection** containing:
- The **walkable area** as a Polygon or MultiPolygon.
- **Exits** (optional) as Polygons or empty GeometryCollections.
- **Distribution zones** (optional) as Polygons or empty GeometryCollections.

Example output:
```
GEOMETRYCOLLECTION (
    GEOMETRYCOLLECTION (
        POLYGON ((...))  -- Walkable Area
    ),
    GEOMETRYCOLLECTION EMPTY,  -- Exits (if present)
    GEOMETRYCOLLECTION EMPTY   -- Distribution zones (if present)
)
```
This ensures that the structure is flexible and can handle cases where exits or distributions are missing.
""",
        formatter_class=RawTextHelpFormatter,
    )
    parser.add_argument(
        "-i",
        "--input",
        help="Path to the DXF file to parse.",
        required=True,
        type=pathlib.Path,
    )
    parser.add_argument(
        "-o",
        "--output",
        help="Output file to save the result in WKT format (default: INPUT.wkt).",
        type=pathlib.Path,
    )
    parser.add_argument(
        "-d",
        "--dxf-output",
        help="Output file to save the result in DXF format.",
        type=pathlib.Path,
        required=False,
    )

    parser.add_argument(
        "-w",
        "--walkable",
        help="Layer containing the walkable area (outer polygon).",
        required=True,
    )
    parser.add_argument(
        "-x",
        "--obstacles",
        help="One or more layers defining obstacles (holes). Can accept multiple layers.",
        nargs="+",
        default=[],
    )
    parser.add_argument(
        "-t",
        "--exits",
        help="One or more layers defining exits. Can accept multiple layers.",
        nargs="+",
        default=[],
    )
    parser.add_argument(
        "-D",
        "--distributions",
        help="One or more layers defining distributions. Can accept multiple layers.",
        nargs="+",
        default=[],
    )

    parser.add_argument(
        "-q",
        "--quad-segments",
        help="Number of linear segments used to approximate a quarter-circle for curved shapes (default: 4).",
        default=4,
    )
    parser.add_argument(
        "-p",
        "--plot",
        help="Display an interactive plot of the parsed polygon.",
        action="store_true",
    )
    return parser.parse_args()


def plot_geometry(geometry_collection):
    """Plot the geometry collection with different colors for each geometry type."""
    fig, ax = plt.subplots(figsize=(8, 8))
    if geometry_collection.is_empty:
        logging.info("Skipping empty geometry.")
        return
    polygons = list(geometry_collection.geoms)  # Unpack main-level geometries
    area = polygons[0] if len(polygons) > 0 else None
    # Extract exits (second collection)
    if len(polygons) > 1:
        exits = polygons[1]

    # Extract distributions (third collection)
    if len(polygons) > 2:
        distributions = polygons[2]

    for a in area.geoms:
        x, y = a.exterior.xy
        plt.fill(x, y, alpha=0.1, color="gray")

    for e in exits.geoms:
        x, y = e.exterior.xy
        plt.fill(x, y, alpha=0.3, color="red")

    for d in distributions.geoms:
        x, y = d.exterior.xy
        plt.fill(x, y, alpha=0.3, color="green")

    legend_elements = {
        "area": mpatches.Patch(color="gray", label="Walkable Area"),
        "exits": mpatches.Patch(color="red", label="Exits"),
        "distribution": mpatches.Patch(
            color="green", label="Distribution Zones"
        ),
    }
    handles = [
        legend_elements[key]
        for key in ["area", "exits", "distribution"]
        if key in legend_elements
    ]
    ax.legend(
        handles=handles,
        loc="upper center",
        bbox_to_anchor=(0.5, 1.05),
        ncol=len(handles),
        frameon=False,
    )
    ax.set_xlabel("X [m]")
    ax.set_ylabel("Y [m]")
    plt.show()


def main():
    parsed_args = parse_args()
    print(parsed_args)
    try:
        merged_polygon = parse_dxf_file(
            parsed_args.input,
            parsed_args.walkable,
            parsed_args.obstacles,
            parsed_args.exits,
            parsed_args.distributions,
            parsed_args.quad_segments,
        )
    except IncorrectDXFFileError as e:
        logging.error(f"Failed to parse DXF: {e.message}")
        return

    # create a dxf file using the parsed geometry
    if parsed_args.dxf_output:
        shapely_to_dxf(merged_polygon, parsed_args.dxf_output)

    # plot final Polygon
    if parsed_args.plot:
        matplotlib.pyplot.set_loglevel("warning")
        logging.getLogger("PIL.PngImagePlugin").setLevel(logging.WARNING)
        plot_geometry(merged_polygon)
    # now cast to wkt and write into a file
    out_file = (
        parsed_args.output
        if parsed_args.output is not None
        else parsed_args.input.with_suffix(".wkt")
    )
    save_as_wkt(merged_polygon, out_file=out_file)


if __name__ == "__main__":
    main()
