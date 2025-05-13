# SPDX-License-Identifier: LGPL-3.0-or-later
import typer
from typing import Optional
from rich.console import Console
from rich.panel import Panel
from rich.table import Table

from rich.markdown import Markdown
from rich.syntax import Syntax

from typing import List

import ezdxf
import geopandas as gpd
import matplotlib.patches as mpatches
import matplotlib.pyplot
import matplotlib.pyplot as plt
import shapely
import logging
from pathlib import Path
import pathlib

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
        l for l in available_layers if any(p in l.lower() for p in patterns)
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
}


class IncorrectDXFFileError(Exception):
    """Exception raised when a DXF file is found to be incorrect or malformed."""

    def __init__(self, message, geometries=None):
        self.message = message
        self.geometries = geometries
        super().__init__(self.message)


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
        return number_obstacles, GeometryCollection(
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

    existing_elements = ["area"]
    if exits.geoms:
        existing_elements.append("exits")
    if distributions.geoms:
        existing_elements.append("distribution")

    for a in area.geoms:
        x, y = a.exterior.xy
        plt.fill(x, y, alpha=0.1, color="gray")
        for hole in a.interiors:
            hx, hy = hole.xy
            ax.fill(
                hx, hy, alpha=1, edgecolor="gray", facecolor="white", lw=0.3
            )

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
        for key in existing_elements
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
    quad_segments: int = typer.Option(
        4, "-q", help="Segments for approximating circles"
    ),
    plot: bool = typer.Option(False, "-p", help="Display parsed geometry plot"),
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

    doc = ezdxf.readfile(input)
    visible_layers = [
        layer.dxf.name
        for layer in doc.layers
        if not layer.is_off() and not layer.is_frozen()
    ]

    if not walkable:
        matches = match_pattern(LAYER_PATTERNS["walkable"], visible_layers)
        if matches:
            walkable = matches[0]
            console.log(
                f"[bold green]Inferred walkable layer:[/bold green] {walkable}"
            )
        else:
            console.print(
                "[red]❌ Could not infer walkable layer. Please specify with --walkable (-w).[/red]"
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

    try:
        num_obstacles, result = parse_dxf_file(
            input, walkable, obstacles, exits, distributions, quad_segments
        )
    except IncorrectDXFFileError as e:
        console.print(f"[red]DXF parsing failed:[/red] {e.message}")
        raise typer.Exit(1)

    if dxf_output:
        shapely_to_dxf(result, dxf_output)

    if plot:
        matplotlib.pyplot.set_loglevel("warning")
        plot_geometry(result)

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

        console.print("\n[bold green]✅ Geometry summary:[/bold green]")
        console.print(table)


if __name__ == "__main__":
    app()
