import ezdxf
import matplotlib.pyplot
from shapely import LineString, Polygon, polygonize, to_wkt, GeometryCollection
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
    with open(out_file_name, 'w') as out:
        out.write(to_wkt(geomery_collection, rounding_precision=-1))


def plot_polygon(polygon):
    """plots a polygon with its interior in matplotlib"""
    poly = gpd.GeoSeries([polygon])
    poly.plot()
    plt.show()


def polyline_to_linestring(polyline):
    """converts a entity with dxftype polyline to a shapely Linestring"""
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])
    return LineString(points)


def polyline_to_polygon(polyline):
    """converts a entity with dxftype polyline to a shapely Polygon"""
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])
    if len(points) < 3:
        logging.error("a polyline has at most 2 points and can not be a Polygon")
        raise IncorrectDXFFile(f"a polyline could not be converted to a Polygon", [points])
    return Polygon(points)


def parse_dxf_file(dxf_path, outer_line_layer, hole_layers):
    """ parses a dxf-file and creates a Polygon with the structure of the file
    @param dxf_path: Path to the DXF file
    @param outer_line_layer: the name of the layer in the dxf-file where the outer polygon is defined
    @param hole_layers: a list with all layer names in the dxf-file where holes are defined
    @return: shapely polygon containing polygon from dxf-file
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
            if entity.dxf.layer in hole_layers:
                holes.append(polyline_to_polygon(entity))
            elif entity.dxf.layer == outer_line_layer:
                outer_lines.append(polyline_to_linestring(entity))
        elif entity.dxftype() != "INSERT":
            logging.warning(f"there is an entity of type {entity.dxftype()} defined which will not be parsed")
            print("there is a entity of unknow type")

    # create a polygon from all outer lines (only works if outer_lines is one polyline)
    outer_polygon = Polygon(outer_lines[0].coords)

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
    return outer_polygon.difference(simple_holes)


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    file_name = "SiB2023_entrance_jupedsim.dxf"
    outer_layer = "jupedsim_walkable_area"
    inner_layers = ["jupedsim_holes", "entranceGates"]
    merged_polygon = parse_dxf_file(file_name, outer_layer, inner_layers)
    # plot final Polygon
    logging.basicConfig(level=logging.WARNING)
    matplotlib.pyplot.set_loglevel("warning")
    plot_polygon(merged_polygon)
    # now cast to wkt and write into a file
    out_file_name = "entrance_jupedsim.wkt"
    save_as_wkt(merged_polygon, out_file=out_file_name)
