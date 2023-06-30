import ezdxf
import matplotlib.pyplot
from shapely import LineString, Polygon, Point, polygonize, to_wkt, GeometryCollection, MultiPolygon
import matplotlib.pyplot as plt
import geopandas as gpd
import logging


class IncorrectDXFFile(Exception):
    def __init__(self, message, geometries=None):
        self.message = message
        self.geometries = geometries
        super().__init__(self.message)


def save_as_wkt(geometry, out_file):
    """
    converts geometry into wkt and writes them in out_file
    @param geometry_polygons: shapely geometry
    @param out_file: The file to write the output to.
    """
    geomery_collection = GeometryCollection(geometry)
    with open(out_file, 'w') as out:
        out.write(to_wkt(geomery_collection, rounding_precision=-1))
    logging.debug(f"wkt was written into {out_file}")


def plot_polygon(polygon):
    """plots a polygon with its interior in matplotlib"""
    poly = gpd.GeoSeries([polygon])
    poly.plot()
    plt.show()


def line_to_linestring(line):
    """converts an entity with dxftype line to a shapely Linestring"""
    start_point = line.dxf.start
    end_point = line.dxf.end
    return LineString([(start_point[0], start_point[1]), (end_point[0], end_point[1])])


def multipolygon_to_list(multipolygon):
    """converts a multipolygon to a list of polygons"""
    return [poly for poly in multipolygon.geoms]


def polyline_to_linestring(polyline):
    """converts an entity with dxftype polyline to a shapely Linestring"""
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])

    # if the polyline is closed this resembles a polygon
    # to create a closed shapely linestring the last point must correspond to the first point
    if polyline.closed:
        points.append(points[0])
    return LineString(points)


def polyline_to_polygon(polyline):
    """converts an entity with dxftype polyline to a shapely Polygon"""
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])

    if len(points) < 3:
        logging.error("a polyline has at most 2 points and can not be a Polygon")
        raise IncorrectDXFFile(f"a polyline could not be converted to a Polygon", [points])
    return Polygon(points)


def dxf_circle_to_shply(dxf_circle, quad_segs=4):
    """converts an entity with dxftype circle to a shapely Point with buffer
    @param dxf_circle: an entity with dxf-type circle
    @param quad_segs: Sets the number of line segments used to approximate an angle fillet"""
    pt = dxf_circle.dxf.center
    radius = dxf_circle.dxf.radius
    return Point(pt).buffer(radius, quad_segs)


def parse_dxf_file(dxf_path, outer_line_layer, hole_layers, circle_accuracy=4):
    """ parses a dxf-file and creates a shapely structure resembling the file
    @param dxf_path: Path to the DXF file
    @param outer_line_layer: the name of the layer in the dxf-file where the outer polygon is defined
    @param hole_layers: a list with all layer names in the dxf-file where holes are defined
    @param circle_accuracy: The accuracy of the circle, specified as the number of line segments
                         used to approximate a circle. By default, `circle_accuracy` is set to 8.
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
                logging.error(f"There is a Polygon in layer {entity.dxf.layer} that is not closed. "
                              f"This may cause issues creating the polygon")
            if entity.dxf.layer in hole_layers:
                holes.append(polyline_to_polygon(entity))
            elif entity.dxf.layer == outer_line_layer:

                outer_lines.append(polyline_to_linestring(entity))
        elif entity.dxftype() == "CIRCLE":
            if entity.dxf.layer in hole_layers:
                holes.append(dxf_circle_to_shply(entity, circle_accuracy))
            else:
                logging.warning(f"there is a circle defined in Layer {entity.dxf.layer} at {entity.dxf.center}."
                                f" This is not valid and will be ignored")

        elif entity.dxftype() != "INSERT":
            logging.warning(f"there is an entity of type {entity.dxftype()} defined which will not be parsed")

    # create a polygon from all outer lines
    outer_polygons = []
    for line in outer_lines:
        outer_polygons.append(Polygon(line.coords))
    outer_polygon = polygonize(outer_lines)

    # separate simple and not simple holes
    simple_holes = []
    other_holes = []
    for hole in holes:
        simple_holes.append(hole) if hole.is_simple else other_holes.append(hole)

    if len(other_holes) > 0:
        logging.error(f"{len(other_holes)} not simple polygons were parsed. These are not supported.")
        raise IncorrectDXFFile("The file contained at least one not simple Polygon.", other_holes)

    # create new Polygon with holes
    simple_holes = polygonize(simple_holes)
    logging.debug(f"the geometry was parsed")
    return outer_polygon.difference(simple_holes)


def shapely_to_dxf(geometry, dxf_path, walkable_layer="walkable_layer", hole_layer="hole_layer"):
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
    logging.debug(f"dxf was written into {dxf_path}")


def main():
    # set up logger
    logging.basicConfig(format="%(levelname)s:%(message)s", level=logging.DEBUG)
    file_name = "SiB2023_entrance_jupedsim.dxf"
    outer_layer = "jupedsim_walkable_area"
    inner_layers = ["jupedsim_holes", "entranceGates"]
    # parse polygon(s)
    merged_polygon = parse_dxf_file(file_name, outer_layer, inner_layers)
    # create a dxf file using the parsed geometry
    shapely_to_dxf(merged_polygon, "parsed_entrance.dxf")
    # plot final Polygon
    matplotlib.pyplot.set_loglevel("warning")
    logging.getLogger('PIL.PngImagePlugin').setLevel(logging.WARNING)

    if merged_polygon.geom_type == "Polygon":
        plot_polygon(merged_polygon)
    if merged_polygon.geom_type == 'MultiPolygon':
        merged_polygon = multipolygon_to_list(merged_polygon)
        for poly in merged_polygon:
            plot_polygon(poly)
    # now cast to wkt and write into a file
    out_file_name = "entrance_jupedsim.wkt"
    save_as_wkt(merged_polygon, out_file=out_file_name)


if __name__ == "__main__":
    main()
