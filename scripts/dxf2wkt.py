# SPDX-License-Identifier: LGPL-3.0-or-later
import logging
import pathlib
from pathlib import Path
from typing import List, Optional
import json
import ezdxf
import geopandas as gpd
import matplotlib.patches as mpatches
from shapely.geometry.base import BaseGeometry
import matplotlib.pyplot as plt
import numpy as np
import shapely
import typer
from rich.console import Console
from rich.markdown import Markdown
from rich.panel import Panel
from rich.syntax import Syntax
from rich.table import Table
from shapely import (
    GeometryCollection,
    LineString,
    MultiPolygon,
    Point,
    Polygon,
    polygonize,
    to_wkt,
)
from shapely.ops import unary_union

logging.getLogger("markdown_it").setLevel(logging.WARNING)
logging.basicConfig(format="%(levelname)s:%(message)s", level=logging.DEBUG)
logging.getLogger("ezdxf").setLevel(logging.WARNING)

logging.getLogger("matplotlib").setLevel(logging.WARNING)


help_text = """
Converts a DXF file into WKT format and saves it as a new file.

Optionally,
the resulting geometry can be exported back to DXF or visualized as a plot.


## RECOMMENDED NAMING CONVENTIONS (auto-detection if arguments are omitted):

- Walkable Area Layer:        jps-walkablearea-* or walkablearea-*
- Obstacle/Hole Layers:       jps-obstacles-* or obstacles-*
- Exit Layers:                jps-exits-* or exits-*
- Distribution Zone Layers:   jps-distributions-* or distributions-*

Layer names must match one of these prefixes (case-insensitive). **Only visible** layers are considered.

If you omit --walkable, --obstacles, --exits, or --distributions, the program
will try to infer them from visible DXF layers based on these naming patterns.


## REQUIREMENTS FOR SUCCESSFUL CONVERSION:
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

## EXPECTED STRUCTURE IN DXF:
- No gaps in the outer polygon or holes.
- No duplicate lines.
- Sufficiently wide areas for agents.
- All polygons must be **simple** (i.e., not self-intersecting).

## STRUCTURE OF THE OUTPUT GEOMETRY:
The resulting geometry is structured as a **GeometryCollection** containing:
- The **walkable area** as a Polygon or MultiPolygon.
- **Exits** (optional) as Polygons or empty GeometryCollections.
- **Distribution zones** (optional) as Polygons or empty GeometryCollections.
"""

example_wkt = """
GEOMETRYCOLLECTION (
    GEOMETRYCOLLECTION (
        POLYGON ((...))  -- Walkable Area
    ),
    GEOMETRYCOLLECTION EMPTY,  -- Exits (if present)
    GEOMETRYCOLLECTION EMPTY   -- Distribution zones (if present)
)
""".strip()


def match_pattern(patterns, available_layers):
    """Return matching layer names based on pattern keywords."""
    return [
        layer
        for layer in available_layers
        if any(p in layer.lower() for p in patterns)
    ]


console = Console()
app = typer.Typer()


@app.command()
def rich_help():
    """Show more help."""
    console.print(
        Panel(
            Markdown(help_text),
            title="DXF to WKT Converter",
            border_style="blue",
            padding=(1, 2),
        )
    )
    console.print(
        Panel(
            Syntax(example_wkt, "text", theme="ansi_dark", line_numbers=False),
            title="Example Output Geometry",
            border_style="green",
        )
    )


LAYER_PATTERNS = {
    "walkable": ["jps-walkablearea", "walkablearea"],
    "obstacles": ["jps-obstacles", "obstacles"],
    "exits": ["jps-exits", "exits"],
    "distributions": ["jps-distributions", "distributions"],
    "waypoints": ["jps-waypoints", "waypoints"],
    "journeys": ["jps-journeys", "journeys"],
}


class IncorrectDXFFileError(Exception):
    """Exception raised when a DXF file is found to be incorrect or malformed."""

    def __init__(self, message, geometries=None):
        self.message = message
        self.geometries = geometries
        super().__init__(self.message)


def parse_waypoints_from_dxf(doc, waypoint_layer="jps-waypoints"):
    """Extract waypoints from DXF circles with center and radius."""
    waypoints = []
    msp = doc.modelspace()
    for entity in msp:
        if entity.dxftype() == "CIRCLE" and entity.dxf.layer == waypoint_layer:
            center = Point(entity.dxf.center[0], entity.dxf.center[1])
            radius = entity.dxf.radius
            waypoints.append((center, radius))
    return waypoints


def save_as_wkt(geometry, out_file: pathlib.Path):
    """
    Convert geometry into wkt and writes them in out_file.

    @param geometry: shapely geometry
    @param out_file: The file to write the output to.
    """
    # create result dir
    out_file.parent.mkdir(parents=True, exist_ok=True)

    geometry_collection = GeometryCollection(geometry)
    with open(out_file, "w") as out:
        out.write(to_wkt(geometry_collection, rounding_precision=-1))

    logging.info(f" WKT-file was written to: {out_file.absolute()}")


def plot_polygon(polygon: shapely.Polygon):
    """Plot a polygon with its interior in matplotlib."""
    poly = gpd.GeoSeries([polygon])
    poly.plot()
    plt.show()


def line_to_linestring(line):
    """Convert an entity with dxftype line to a shapely Linestring."""
    start_point = line.dxf.start
    end_point = line.dxf.end
    return LineString(
        [(start_point[0], start_point[1]), (end_point[0], end_point[1])]
    )


def multipolygon_to_list(multipolygon: shapely.MultiPolygon):
    """Convert a multipolygon to a list of polygons."""
    return [poly for poly in multipolygon.geoms]


def polyline_to_linestring(polyline):
    """Convert an entity with dxftype polyline to a shapely Linestring."""
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
    """Convert an entity with dxftype polyline to a shapely Polygon."""
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])

    if len(points) < 3:
        logging.error(
            "A polyline with fewer than 3 points cannot form a valid polygon."
        )
        raise IncorrectDXFFileError(
            "Polyline conversion to a Polygon failed due to insufficient points.",
            [points],
        )

    return Polygon(points)


def dxf_circle_to_shply(dxf_circle, quad_segs):
    """Convert an entity with dxftype circle to a shapely Point with buffer.

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


def validate_exits_and_distributions(
    walkable_area: Polygon | MultiPolygon,
    exits: List[Polygon],
    distributions: List[Polygon],
) -> None:
    """Validate that exits and distribution zones.

    - Are fully contained within the walkable area.
    - Do not overlap the walkable area boundary.
    - Do not overlap with each other.

    Raises an IncorrectDXFFileError if any constraint is violated.

    Parameters:
    ----------
    walkable_area : Polygon | MultiPolygon
        The main walkable area in which exits and distributions must reside.
    exits : List[Polygon]
        A list of polygons representing exit areas.
    distributions : List[Polygon]
        A list of polygons representing distribution zones.
    """
    invalid_exits = [
        exit_poly
        for exit_poly in exits.geoms
        if not walkable_area.contains(exit_poly)
    ]
    if invalid_exits:
        logging.error("Some exits are outside the walkable area!")
        for i, exit_poly in enumerate(invalid_exits, start=1):
            logging.error(
                f"Exit {i} has points outside the walkable area: {list(exit_poly.exterior.coords)}"
            )
        raise IncorrectDXFFileError(
            "Exits must be fully contained within the walkable area."
        )

    invalid_distributions = [
        dist_poly
        for dist_poly in distributions.geoms
        if not walkable_area.contains(dist_poly)
    ]
    if invalid_distributions:
        logging.error("Some distribution zones are outside the walkable area!")
        for i, dist_poly in enumerate(invalid_distributions, start=1):
            logging.error(
                f"Distribution {i} has points outside the walkable area: {list(dist_poly.exterior.coords)}"
            )
        raise IncorrectDXFFileError(
            "Distribution zones must be fully contained within the walkable area."
        )

    overlapping_exits = [
        exit_poly
        for exit_poly in exits.geoms
        if walkable_area.intersects(exit_poly)
        and not walkable_area.contains(exit_poly)
    ]
    if overlapping_exits:
        logging.error(
            "Some exits are intersecting but not fully inside the walkable area."
        )
        for i, exit_poly in enumerate(overlapping_exits, start=1):
            logging.error(
                f"Exit {i} intersects the walkable area: {list(exit_poly.exterior.coords)}"
            )
        raise IncorrectDXFFileError(
            "Exits must not overlap the walkable area boundary."
        )

    overlapping_distributions = [
        dist_poly
        for dist_poly in distributions.geoms
        if walkable_area.intersects(dist_poly)
        and not walkable_area.contains(dist_poly)
    ]
    if overlapping_distributions:
        logging.error(
            "Some distribution zones are intersecting but not fully inside the walkable area."
        )
        for i, dist_poly in enumerate(overlapping_distributions, start=1):
            logging.error(
                f"Distribution {i} intersects the walkable area: {list(dist_poly.exterior.coords)}"
            )
        raise IncorrectDXFFileError(
            "Distribution zones must not overlap the walkable area boundary."
        )

    if unary_union(exits).intersects(unary_union(distributions)):
        logging.error("Exits and distribution zones are overlapping!")
        raise IncorrectDXFFileError(
            "Exits and distribution zones must not overlap each other."
        )


def parse_dxf_file(
    dxf_path: pathlib.Path,
    outer_line_layer: str,
    hole_layers: List[str],
    exit_layers: List[str],
    distribution_layers: List[str],
    waypoint_layers: List[str],
    journey_layers: List[str],
    quad_segs: int,
):
    """Parse a dxf-file and creates a shapely structure resembling the file.

    @param dxf_path: Path to the DXF file
    @param outer_line_layer: the name of the layer in the dxf-file where the
                             outer polygon is defined
    @param hole_layers: a list with all layer names in the dxf-file where holes
                        are defined
    @param exit_layers: a list with all layer names in the dxf-file where exits
                        are defined
    @param distribution_layers: a list with all layer names in the dxf-file where distributions
                        are defined
    @param waypoint_layers: a list with all layer names in the dxf-file where waypoints
                        are defined
    @param journey_layers: a list with all layer names in the dxf-file where journeys
                        are defined
    @param quad_segs: Specifies the number of linear segments in a quarter
                      circle in the approximation of circular arcs.

    @return: shapely polygon or multipolygon from dxf-file and additional components
    """
    doc = ezdxf.readfile(dxf_path)

    holes = []
    exits = []
    distributions = []
    outer_lines = []
    waypoints = []
    journeys = []

    # Open the DXF file
    # Filter only visible layers
    visible_layers = [
        layer.dxf.name
        for layer in doc.layers
        if not layer.is_off()
        and not layer.is_frozen()
        and layer.dxf.name.lower() != "defpoints"
    ]

    layers_in_dxf = visible_layers
    msp = doc.modelspace()
    logging.info(f"{outer_line_layer = }")

    if not layer_exists(outer_line_layer, layers_in_dxf):
        raise IncorrectDXFFileError(
            f"Layer '{outer_line_layer}' not found in DXF file. Ensure it exists, is visible, and matches naming conventions (e.g., 'jps-walkablearea-*').\n Available layers are: {layers_in_dxf}"
        )
    hole_layers = validate_layers(layers_in_dxf, hole_layers, "hole")
    exit_layers = validate_layers(layers_in_dxf, exit_layers, "exit")
    distribution_layers = validate_layers(
        layers_in_dxf, distribution_layers, "distribution"
    )
    waypoint_layers = validate_layers(
        layers_in_dxf, waypoint_layers, "waypoint"
    )
    journey_layers = validate_layers(layers_in_dxf, journey_layers, "journey")

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
            elif any(
                journey_layer in entity.dxf.layer
                for journey_layer in journey_layers
            ):
                coords = [(p[0], p[1]) for p in entity.get_points()]
                if len(coords) >= 2:
                    journeys.append(LineString(coords))

        elif entity.dxftype() == "CIRCLE":
            if entity.dxf.layer in hole_layers:
                holes.append(dxf_circle_to_shply(entity, quad_segs))
            elif any(
                waypoint_layer in entity.dxf.layer
                for waypoint_layer in waypoint_layers
            ):
                center = Point(entity.dxf.center[0], entity.dxf.center[1])
                radius = entity.dxf.radius
                waypoints.append((center, radius))
            else:
                logging.warning(
                    f"there is a circle defined in Layer {entity.dxf.layer} "
                    f"at {entity.dxf.center}. This is not valid and will be "
                    f"ignored."
                )
        elif entity.dxftype() == "LINE":
            if any(
                journey_layer in entity.dxf.layer
                for journey_layer in journey_layers
            ):
                start = entity.dxf.start
                end = entity.dxf.end
                coords = [(start[0], start[1]), (end[0], end[1])]
                journeys.append(LineString(coords))
        elif entity.dxftype() != "INSERT":
            coords = None
            try:
                # Try accessing common geometric data
                if hasattr(entity, "dxf") and hasattr(entity.dxf, "insert"):
                    coords = entity.dxf.insert  # For INSERT-like entities
                elif hasattr(entity, "dxf") and hasattr(entity.dxf, "start"):
                    coords = entity.dxf.start  # For LINE, etc.
                elif hasattr(entity, "dxf") and hasattr(entity.dxf, "center"):
                    coords = entity.dxf.center  # For CIRCLE, ARC, etc.
                elif hasattr(entity, "dxf") and hasattr(entity.dxf, "vertices"):
                    coords = list(entity.dxf.vertices)  # For some polylines
                elif hasattr(
                    entity, "vertices"
                ):  # Sometimes vertices is a method or property
                    coords = list(entity.vertices())

            except Exception as e:
                coords = f"Could not extract coordinates: {e}"

            logging.warning(
                f"Skipped {entity.dxftype()} at ({coords[0]:.2f}, {coords[1]:.2f})"
            )

    # create a polygon from all outer lines
    outer_polygons = []
    for line in outer_lines:
        outer_polygons.append(Polygon(line.coords))

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

    if other_holes:
        logging.error(
            f"Detected {len(other_holes)} non-simple polygons in the hole layer. "
            f"These polygons are not supported."
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

    if other_exits:
        logging.error(
            f"Detected {len(other_exits)} non-simple polygons in the exit layer(s). "
            f"These polygons are not supported."
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

    if other_distributions:
        logging.error(
            f"Detected {len(other_distributions)} non-simple polygons in the distribution layer(s). "
            f"These polygons are not supported."
        )
        raise IncorrectDXFFileError(
            "The file contained at least one not simple distribution polygon.",
            other_distributions,
        )

    # create new Polygon with exits
    simple_distributions = polygonize(simple_distributions)
    validate_exits_and_distributions(
        outer_polygon, simple_exits, simple_distributions
    )

    number_obstacles = len(list(simple_holes.geoms))
    if not outer_polygon or outer_polygon.is_empty:
        logging.error(
            "The outer polygon is empty. Returning an empty geometry."
        )
        return number_obstacles, GeometryCollection()

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
        waypoint_geoms = [pt.buffer(r) for pt, r in waypoints]
        waypoints_collection = GeometryCollection(waypoint_geoms)
        journeys_collection = GeometryCollection(journeys)

        return (
            number_obstacles,
            GeometryCollection(
                [
                    walkable_area_collection,
                    exits_collection,
                    distributions_collection,
                    waypoints_collection,
                    journeys_collection,
                ]
            ),
        )
    except Exception as e:
        logging.error(f"Error while computing difference: {e}")
        return outer_polygon


@app.command()
def convert(
    input: Path = typer.Option(..., "-i", help="Input DXF file"),
    output: Optional[Path] = typer.Option(
        None, "-o", help="Output WKT file (defaults to input.wkt)"
    ),
    dxf_output: Optional[Path] = typer.Option(
        None, "-d", help="Output DXF file"
    ),
    walkable: Optional[str] = typer.Option(
        None, "-w", help="Walkable area layer name"
    ),
    obstacles: list[str] = typer.Option(
        [], "-x", help="Obstacle (hole) layer names"
    ),
    exits: list[str] = typer.Option([], "-t", help="Exit layer names"),
    distributions: list[str] = typer.Option(
        [], "-D", help="Distribution layer names"
    ),
    waypoints: list[str] = typer.Option([], "-p", help="Waypoint layer names"),
    journeys: list[str] = typer.Option([], "-j", help="Journey layer names"),
    quad_segments: int = typer.Option(
        4, "-q", help="Segments for approximating circles"
    ),
):
    """convert DXF to WKT"""
    console.print(
        "\n[dim] 💬 For a better overview, run:[/] [green]dxf2wkt rich-help[/green]"
    )
    # Validate input
    if not input.exists():
        console.print(f"[red]❌ Input file not found:[/] {input}")
        raise typer.Exit(1)

    if not output:
        output = input.with_suffix(".wkt")

    with console.status(f"Reading DXF file: {input}", spinner="dots"):
        try:
            doc = ezdxf.readfile(input)
        except Exception as err:
            console.print(f"[red]❌ Error reading DXF file:[/] {err}")
            raise typer.Exit(1)

    # Show visible and hidden layers
    visible_layers = [
        layer.dxf.name
        for layer in doc.layers
        if not layer.is_off() and not layer.is_frozen()
    ]

    table = Table(title="DXF Layer Overview")
    table.add_column("Layer Name", style="cyan", no_wrap=True)
    table.add_column("Visibility", style="green")

    for layer in doc.layers:
        if layer.dxf.name.lower() == "defpoints":
            continue
        status = (
            "Visible"
            if not layer.is_off() and not layer.is_frozen()
            else "Hidden"
        )
        style = "green" if status == "Visible" else "red"
        table.add_row(layer.dxf.name, status, style=style)

    console.print(table)

    if not walkable:
        matches = match_pattern(LAYER_PATTERNS["walkable"], visible_layers)
        if matches:
            walkable = matches[0]
            console.print(
                f"[bold green]✔ Inferred walkable layer:[/] {walkable}"
            )
        else:
            console.print(
                "[red]❌ Could not infer walkable layer. Use --walkable or rename your layer to follow naming convention.[/red]"
            )
            raise typer.Exit(1)

    if not obstacles:
        obstacles.extend(
            match_pattern(LAYER_PATTERNS["obstacles"], visible_layers)
        )
    if not exits:
        exits.extend(match_pattern(LAYER_PATTERNS["exits"], visible_layers))
    if not distributions:
        distributions.extend(
            match_pattern(LAYER_PATTERNS["distributions"], visible_layers)
        )
    if not waypoints:
        waypoints.extend(
            match_pattern(LAYER_PATTERNS["waypoints"], visible_layers)
        )
    if not journeys:
        journeys.extend(
            match_pattern(LAYER_PATTERNS["journeys"], visible_layers)
        )

    try:
        num_obstacles, result = parse_dxf_file(
            input,
            walkable,
            obstacles,
            exits,
            distributions,
            waypoints,
            journeys,
            quad_segments,
        )
    except IncorrectDXFFileError as e:
        console.print(f"[red]DXF parsing failed:[/red] {e.message}")
        raise typer.Exit(1)

    if dxf_output:
        shapely_to_dxf(result, dxf_output)

    out_file = output if output else input.with_suffix(".wkt")
    save_as_wkt(result, out_file)

    if not result.is_empty:
        parts = list(result.geoms)

        table = Table(title="", show_lines=True)
        table.add_column("Category", style="cyan", no_wrap=True)
        table.add_column("Count", style="magenta")

        if len(parts) > 0:
            table.add_row("Walkable areas", str(len(list(parts[0].geoms))))
        table.add_row("Obstacles", str(num_obstacles))
        if len(parts) > 1:
            table.add_row("Exits", str(len(list(parts[1].geoms))))
        if len(parts) > 2:
            table.add_row("Distributions", str(len(list(parts[2].geoms))))
        if len(parts) > 3:
            table.add_row("Waypoints", str(len(list(parts[3].geoms))))
        if len(parts) > 4:
            table.add_row("Journeys", str(len(list(parts[4].geoms))))

        console.print("\n[bold green]✅ Geometry summary:[/bold green]")
        console.print(table)


def plot_interactive(geometry, title="Interactive Geometry Viewer"):
    """Create an interactive plot with Matplotlib"""
    from matplotlib.patches import Polygon as MplPolygon
    from matplotlib.widgets import CheckButtons

    fig, ax = plt.subplots(figsize=(10, 8))
    fig.canvas.manager.set_window_title(title)

    # Extract geometry components
    parts = list(geometry.geoms)
    walkable_areas = list(parts[0].geoms) if len(parts) > 0 else []
    exits = list(parts[1].geoms) if len(parts) > 1 else []
    distributions = list(parts[2].geoms) if len(parts) > 2 else []
    waypoints = list(parts[3].geoms) if len(parts) > 3 else []
    journeys = list(parts[4].geoms) if len(parts) > 4 else []

    walkable_patches = []
    exit_patches = []
    distribution_patches = []
    waypoint_patches = []
    journey_lines = []

    # Plot walkable areas
    for area in walkable_areas:
        x, y = area.exterior.xy
        patch = MplPolygon(
            np.column_stack([x, y]),
            alpha=0.1,
            color="gray",
            label="Walkable Area",
        )
        ax.add_patch(patch)
        walkable_patches.append(patch)

        for hole in area.interiors:
            hx, hy = hole.xy
            hole_patch = MplPolygon(
                np.column_stack([hx, hy]),
                alpha=1,
                edgecolor="gray",
                facecolor="white",
                lw=0.3,
            )
            ax.add_patch(hole_patch)
            walkable_patches.append(hole_patch)

    # Plot exits
    for exit_area in exits:
        x, y = exit_area.exterior.xy
        patch = MplPolygon(
            np.column_stack([x, y]), alpha=0.3, color="red", label="Exit"
        )
        ax.add_patch(patch)
        exit_patches.append(patch)

    # Plot distributions
    for dist in distributions:
        x, y = dist.exterior.xy
        patch = MplPolygon(
            np.column_stack([x, y]),
            alpha=0.3,
            color="green",
            label="Distribution",
        )
        ax.add_patch(patch)
        distribution_patches.append(patch)

    # Plot waypoints
    for w in waypoints:
        x, y = w.centroid.x, w.centroid.y
        r = w.centroid.distance(Point(w.exterior.coords[0]))
        patch = plt.Circle((x, y), r, color="blue", alpha=0.7)
        ax.add_patch(patch)
        waypoint_patches.append(patch)

    # Plot journeys
    for journey in journeys:
        x, y = journey.xy
        line = plt.Line2D(x, y, color="magenta", linewidth=2, linestyle="-")
        ax.add_line(line)
        journey_lines.append(line)

    ax.autoscale_view()
    ax.set_aspect("equal")
    ax.set_xlabel("X [m]")
    ax.set_ylabel("Y [m]")

    # Add check buttons for visibility toggles
    check_labels = [
        "Walkable",
        "Exits",
        "Distributions",
        "Waypoints",
        "Journeys",
    ]
    check_defaults = [True, True, True, True, True]

    # Filter out empty categories
    active_labels = []
    active_defaults = []

    if walkable_patches:
        active_labels.append("Walkable")
        active_defaults.append(True)
    if exit_patches:
        active_labels.append("Exits")
        active_defaults.append(True)
    if distribution_patches:
        active_labels.append("Distributions")
        active_defaults.append(True)
    if waypoint_patches:
        active_labels.append("Waypoints")
        active_defaults.append(True)
    if journey_lines:
        active_labels.append("Journeys")
        active_defaults.append(True)

    ax_check = plt.axes([0.05, 0.05, 0.15, 0.15])
    check = CheckButtons(ax_check, active_labels, active_defaults)

    def toggle_visibility(label):
        label = label.lower()
        if label == "walkable":
            for patch in walkable_patches:
                patch.set_visible(not patch.get_visible())
        elif label == "exits":
            for patch in exit_patches:
                patch.set_visible(not patch.get_visible())
        elif label == "distributions":
            for patch in distribution_patches:
                patch.set_visible(not patch.get_visible())
        elif label == "waypoints":
            for patch in waypoint_patches:
                patch.set_visible(not patch.get_visible())
        elif label == "journeys":
            for line in journey_lines:
                line.set_visible(not line.get_visible())
        fig.canvas.draw_idle()

    check.on_clicked(toggle_visibility)

    legend_elements = []
    if walkable_areas:
        legend_elements.append(
            mpatches.Patch(color="gray", label="Walkable Area")
        )
    if exits:
        legend_elements.append(mpatches.Patch(color="red", label="Exits"))
    if distributions:
        legend_elements.append(
            mpatches.Patch(color="green", label="Distribution Zones")
        )
    if waypoints:
        legend_elements.append(mpatches.Patch(color="blue", label="Waypoints"))
    if journeys:
        legend_elements.append(
            mpatches.Patch(color="magenta", label="Journeys")
        )

    ax.legend(
        handles=legend_elements,
        loc="upper center",
        bbox_to_anchor=(0.5, 1.05),
        ncol=len(legend_elements),
    )

    plt.show()


def shapely_to_dxf(
    geometry,
    dxf_path,
    walkable_layer="walkable_layer",
    hole_layer="hole_layer",
):
    """Create a dxf file according to the geometry.

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


@app.command()
def analyze(
    input: Path = typer.Option(..., "-i", help="Input WKT or DXF file"),
):
    """Analyze geometry and provide metrics."""

    # Load geometry
    if input.suffix.lower() == ".wkt":
        try:
            with open(input) as f:
                from shapely import wkt

                geometry = wkt.loads(f.read())
        except Exception as e:
            console.print(f"[red]❌ Failed to load WKT file:[/] {e}")
            raise typer.Exit(1)
    elif input.suffix.lower() == ".dxf":
        try:
            doc = ezdxf.readfile(input)
            visible_layers = [
                layer.dxf.name
                for layer in doc.layers
                if not layer.is_off()
                and not layer.is_frozen()
                and layer.dxf.name.lower() != "defpoints"
            ]

            walkable = match_pattern(LAYER_PATTERNS["walkable"], visible_layers)
            walkable_layer = walkable[0] if walkable else None

            if not walkable_layer:
                console.print(
                    "[red]❌ Could not auto-detect walkable area in DXF.[/red]"
                )
                raise typer.Exit(1)

            _, geometry = parse_dxf_file(
                input,
                walkable_layer,
                match_pattern(LAYER_PATTERNS["obstacles"], visible_layers),
                match_pattern(LAYER_PATTERNS["exits"], visible_layers),
                match_pattern(LAYER_PATTERNS["distributions"], visible_layers),
                match_pattern(LAYER_PATTERNS["waypoints"], visible_layers),
                match_pattern(LAYER_PATTERNS["journeys"], visible_layers),
                quad_segs=4,
            )
        except Exception as e:
            console.print(f"[red]❌ Failed to load DXF file:[/] {e}")
            raise typer.Exit(1)
    else:
        console.print(
            "[red]Unsupported file format. Please provide a .wkt or .dxf file.[/red]"
        )
        raise typer.Exit(1)

    # Extract components

    walkable_areas = list(geometry.geoms[0].geoms)
    exits = list(geometry.geoms[1].geoms) if len(geometry.geoms) > 1 else []
    distributions = (
        list(geometry.geoms[2].geoms) if len(geometry.geoms) > 2 else []
    )
    waypoints = list(geometry.geoms[3].geoms) if len(geometry.geoms) > 3 else []
    journeys = list(geometry.geoms[4].geoms) if len(geometry.geoms) > 4 else []

    # Create results table
    table = Table(title=f"📊 Geometry Analysis: {input.name}")
    table.add_column("Metric", style="cyan")
    table.add_column("Value", style="green")

    # Walkable area metrics
    total_area = sum(area.area for area in walkable_areas)
    table.add_row("Total walkable area", f"{total_area:.2f} m²")

    total_perimeter = sum(area.length for area in walkable_areas)
    table.add_row("Total perimeter", f"{total_perimeter:.2f} m")

    # Exit metrics
    if exits:
        table.add_row("Exit count", str(len(exits)))

    if waypoints:
        table.add_row("Waypoint count", str(len(waypoints)))

    if journeys:
        table.add_row("Journey count", str(len(journeys)))

    # Distribution zones
    if distributions:
        total_dist_area = sum(d.area for d in distributions)
        table.add_row("Total distribution area", f"{total_dist_area:.2f} m²")
        table.add_row("Distribution count", str(len(distributions)))

    # Obstacles
    obstacle_count = sum(len(area.interiors) for area in walkable_areas)
    table.add_row("Obstacle count", str(obstacle_count))

    # Complexity
    total_vertices = sum(len(area.exterior.coords) for area in walkable_areas)
    total_vertices += sum(
        len(interior.coords)
        for area in walkable_areas
        for interior in area.interiors
    )
    table.add_row("Total vertices", str(total_vertices))

    console.print(table)


@app.command()
def view(
    input: Path = typer.Option(..., "-i", help="Input WKT or DXF file"),
):
    """View geometry in an interactive viewer"""

    # Load geometry
    if input.suffix.lower() == ".wkt":
        try:
            with open(input) as f:
                from shapely import wkt

                geometry = wkt.loads(f.read())
        except Exception as e:
            console.print(f"[red]❌ Failed to load WKT file:[/] {e}")
            raise typer.Exit(1)
    elif input.suffix.lower() == ".dxf":
        try:
            doc = ezdxf.readfile(input)
            visible_layers = [
                layer.dxf.name
                for layer in doc.layers
                if not layer.is_off()
                and not layer.is_frozen()
                and layer.dxf.name.lower() != "defpoints"
            ]

            walkable = match_pattern(LAYER_PATTERNS["walkable"], visible_layers)
            walkable_layer = walkable[0] if walkable else None

            if not walkable_layer:
                console.print(
                    "[red]❌ Could not auto-detect walkable area in DXF.[/red]"
                )
                raise typer.Exit(1)

            waypoints = match_pattern(
                LAYER_PATTERNS["waypoints"], visible_layers
            )
            journeys = match_pattern(LAYER_PATTERNS["journeys"], visible_layers)

            _, geometry = parse_dxf_file(
                input,
                walkable_layer,
                match_pattern(LAYER_PATTERNS["obstacles"], visible_layers),
                match_pattern(LAYER_PATTERNS["exits"], visible_layers),
                match_pattern(LAYER_PATTERNS["distributions"], visible_layers),
                waypoints,
                journeys,
                quad_segs=4,
            )
        except Exception as e:
            console.print(f"[red]❌ Failed to load DXF file:[/] {e}")
            raise typer.Exit(1)
    else:
        console.print(
            "[red]Unsupported file format. Please provide a .wkt or .dxf file.[/red]"
        )
        raise typer.Exit(1)

    plot_interactive(geometry, title=f"Geometry Viewer: {input.name}")


@app.command("clearance-heatmap")
def clearance_heatmap(
    input: Path = typer.Option(..., "-i", help="Input WKT file only"),
    spacing: float = typer.Option(
        0.2, "--spacing", "-s", help="Grid spacing in meters"
    ),
    max_clearance: float = typer.Option(
        2.0, "--max", help="Max clearance to scale colormap (in meters)"
    ),
):
    """Visualize minimum clearance in walkable areas using a color-coded heatmap."""
    import numpy as np
    from shapely.geometry import Point
    from shapely import wkt
    import matplotlib.pyplot as plt
    import matplotlib.cm as cm

    if input.suffix.lower() != ".wkt":
        console.print(
            "[red]Only WKT input is supported for clearance heatmap.[/red]"
        )
        raise typer.Exit(1)

    try:
        with open(input) as f:
            geometry = wkt.loads(f.read())
    except Exception as e:
        console.print(f"[red]❌ Failed to load WKT file:[/] {e}")
        raise typer.Exit(1)

    walkable_area = (
        geometry.geoms[0] if len(geometry.geoms) > 0 else GeometryCollection()
    )

    sample_points = []
    sample_values = []

    for polygon in walkable_area.geoms:
        minx, miny, maxx, maxy = polygon.bounds
        x_vals = np.arange(minx, maxx, spacing)
        y_vals = np.arange(miny, maxy, spacing)

        for x in x_vals:
            for y in y_vals:
                point = Point(x, y)
                if polygon.contains(point) and all(
                    not hole.contains(point) for hole in polygon.interiors
                ):
                    distances = [polygon.exterior.distance(point)] + [
                        hole.distance(point) for hole in polygon.interiors
                    ]
                    clearance = min(distances)
                    sample_points.append((x, y))
                    sample_values.append(clearance)

    # Normalize values to colormap
    norm = plt.Normalize(vmin=0.0, vmax=max_clearance)
    colors = cm.inferno(norm(sample_values))

    fig, ax = plt.subplots(figsize=(10, 8))
    for polygon in walkable_area.geoms:
        x, y = polygon.exterior.xy
        ax.fill(x, y, alpha=0.1, color="gray")
        for hole in polygon.interiors:
            hx, hy = hole.xy
            ax.fill(
                hx, hy, alpha=1, facecolor="white", edgecolor="gray", lw=0.5
            )

    for (x, y), color in zip(sample_points, colors):
        circle = plt.Circle(
            (x, y), radius=spacing * 0.3, color=color, alpha=0.8
        )
        ax.add_patch(circle)

    sm = plt.cm.ScalarMappable(cmap="inferno", norm=norm)
    sm.set_array([])
    cbar = plt.colorbar(sm, ax=ax)
    cbar.set_label("Minimum Clearance [m]")

    ax.set_aspect("equal")
    ax.set_title(f"Clearance Heatmap: {input.name}")
    ax.set_xlabel("X [m]")
    ax.set_ylabel("Y [m]")
    plt.show()


def identify_stage(point: Point, stage_dict: dict) -> Optional[str]:
    for sid, geom in stage_dict.items():
        if geom.contains(point):
            return sid
    return None


@app.command("makejourneys")
def make_journeys(
    input: Path = typer.Option(
        ...,
        "-i",
        help="DXF file containing journeys, waypoints, exits, and distributions",
    ),
    output: Path = typer.Option(
        None,
        "-o",
        help="JSON file to write journey definitions to (defaults input.json)",
    ),
):
    """
    Construct journey definitions from a DXF file:
    - Reads jps-journeys (polylines)
    - Resolves each endpoint by containment in stage geometries (distribution, waypoint, exit)
    - Outputs journey stage sequences and transitions
    - Writes results to JSON file if output path is provided
    """

    def parse_polygons(doc, layers: list[str]) -> dict[str, BaseGeometry]:
        msp = doc.modelspace()
        geometries = {}
        index = 0
        for entity in msp:
            if entity.dxftype() == "LWPOLYLINE" and any(
                layer in entity.dxf.layer for layer in layers
            ):
                coords = [(p[0], p[1]) for p in entity.get_points()]
                if len(coords) >= 3:
                    poly = Polygon(coords)
                    if poly.is_valid:
                        geometries[f"{layers[0]}_{index}"] = poly
                        index += 1
        return geometries

    def parse_circles(doc, layer: str) -> dict[str, BaseGeometry]:
        msp = doc.modelspace()
        circles = {}
        index = 0
        for entity in msp:
            if entity.dxftype() == "CIRCLE" and entity.dxf.layer == layer:
                center = Point(entity.dxf.center)
                radius = entity.dxf.radius
                circles[f"{layer}_{index}"] = center.buffer(radius)
                index += 1
        return circles

    def identify_stage(point: Point, stage_dict: dict) -> str | None:
        for sid, geom in stage_dict.items():
            if geom.contains(point):
                return sid
        return None

    def parse_journey_paths_from_dxf(doc, visible_layers):
        """Extract journey paths from DXF polylines as LineStrings."""
        msp = doc.modelspace()
        journey_paths = []
        for entity in msp:
            if (
                (entity.dxftype() == "LWPOLYLINE" or entity.dxftype() == "LINE")
                and "jps-journeys" in entity.dxf.layer
                and entity.dxf.layer in visible_layers
            ):
                coords = []
                if entity.dxftype() == "LWPOLYLINE":
                    coords = [(p[0], p[1]) for p in entity.get_points()]
                elif entity.dxftype() == "LINE":
                    start = entity.dxf.start
                    end = entity.dxf.end
                    coords = [(start[0], start[1]), (end[0], end[1])]

                if len(coords) >= 2:
                    journey_paths.append(LineString(coords))
        return journey_paths

    doc = ezdxf.readfile(input)
    visible_layers = [
        layer.dxf.name
        for layer in doc.layers
        if not layer.is_off() and not layer.is_frozen()
    ]
    distributions = parse_polygons(
        doc, match_pattern(["jps-distributions"], visible_layers)
    )
    exits = parse_polygons(doc, match_pattern(["jps-exits"], visible_layers))
    waypoints = parse_circles(doc, "jps-waypoints")
    journeys = parse_journey_paths_from_dxf(doc, visible_layers)
    if not journeys:
        logging.info("No visible journey layer!")
        return {}
    # Structure to store journey data for JSON output
    journey_data = {
        "exits": {},
        "distributions": {},
        "waypoints": {},
        "journeys": [],
        "transitions": [],
    }

    # Add geometry information to the data structure
    for exit_id, exit_geom in exits.items():
        # For polygons, store the coordinates
        coords = list(exit_geom.exterior.coords)
        # Convert to list of lists for JSON serialization
        coords_list = [[x, y] for x, y in coords]
        journey_data["exits"][exit_id] = {
            "type": "polygon",
            "coordinates": coords_list,
        }

    for dist_id, dist_geom in distributions.items():
        coords = list(dist_geom.exterior.coords)
        # Convert to list of lists for JSON serialization
        coords_list = [[x, y] for x, y in coords]
        journey_data["distributions"][dist_id] = {
            "type": "polygon",
            "coordinates": coords_list,
        }

    for waypoint_id, waypoint_geom in waypoints.items():
        # For circles, store center and radius
        center = waypoint_geom.centroid
        radius = Point(center).distance(Point(waypoint_geom.exterior.coords[0]))
        journey_data["waypoints"][waypoint_id] = {
            "type": "circle",
            "center": [center.x, center.y],
            "radius": radius,
        }

    for j, line in enumerate(journeys):
        coords = list(line.coords)
        stage_ids = []
        start = identify_stage(Point(coords[0]), distributions)
        if start:
            stage_ids.append(start)
        for pt in coords[1:-1]:
            sid = identify_stage(Point(pt), waypoints)
            if sid:
                stage_ids.append(sid)
        end = identify_stage(Point(coords[-1]), exits)
        if end:
            stage_ids.append(end)

        if len(stage_ids) >= 2:
            journey_id = f"journey_{j}"
            console.print(
                f"[green]Journey {j}[/green]: " + " ➜ ".join(stage_ids)
            )

            # Add all transitions for this journey
            journey_transitions = []
            for a, b in zip(stage_ids[:-1], stage_ids[1:]):
                console.print(f"  • transition: {a} → {b}")
                transition = {"from": a, "to": b, "journey_id": journey_id}
                journey_data["transitions"].append(transition)
                journey_transitions.append(transition)

            # Add journey to data structure
            journey_data["journeys"].append(
                {
                    "id": journey_id,
                    "stages": stage_ids,
                    "transitions": journey_transitions,  # Add transitions directly to journey
                }
            )
        else:
            console.print(
                f"[yellow]Skipping journey {j}: could not resolve enough stages[/yellow]"
            )

    if not output:
        output = input.with_suffix(".json")

    output_path = Path(output)
    with open(output_path, "w") as f:
        json.dump(journey_data, f, indent=2)

    console.print(f"[green]Journey data written to {output_path}[/green]")
    return journey_data


if __name__ == "__main__":
    app()
