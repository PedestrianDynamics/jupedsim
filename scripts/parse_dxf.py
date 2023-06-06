import ezdxf
import shapely
from shapely import LineString, Polygon, polygonize, MultiPolygon, to_wkt, unary_union, GeometryCollection
import matplotlib.pyplot as plt
import geopandas as gpd


def plot_polygon(polygon):
    """plots a polygon with its interior in matplotlib"""
    poly = gpd.GeoSeries([polygon])
    poly.plot()
    plt.show()


def polyline_to_linestring(polyline):
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])
    return LineString(points)


def polyline_to_polygon(polyline):
    points = []
    for point in polyline.get_points():
        points.append([point[0], point[1]])
    if len(points) < 3:
        print(points)
        return []
    return Polygon(points)


def plot_lines(lines):
    fig, ax = plt.subplots()
    for line in lines:
        x_coords, y_coords = zip(*line)
        plt.plot(x_coords, y_coords)
    ax.autoscale()
    plt.show()

def plot_line_strings(lines):
    # Create a new figure and axis object
    fig, ax = plt.subplots()

    for line_string in lines:
        # Extrahieren der Koordinaten des LineString-Objekts
        x, y = line_string.xy
        ax.plot(x, y)

    # Adjust the axis limits
    ax.autoscale()
    plt.show()


if __name__ == "__main__":
    # Path to the DXF file
    file_path = "./SiB2023_entrance_jupedsim.dxf"
    hole_lines = []
    holes = []
    outer_lines = []
    # Open the DXF file
    doc = ezdxf.readfile(file_path)

    # Access the model (modelspace)
    msp = doc.modelspace()

    # Iterate over all entities in the model
    for entity in msp:
        if entity.dxftype() == "LWPOLYLINE":
            if entity.dxf.layer == "jupedsim_holes":
                holes.append(polyline_to_polygon(entity))
            elif entity.dxf.layer == "jupedsim_walkable_area":
                outer_lines.append(polyline_to_linestring(entity))
        elif entity.dxftype() != "INSERT":
            print("entity type:", entity.dxftype())
    # plot the parsed lines from the dxf file
    plot_line_strings(outer_lines)

    # create a polygon from all outer lines (only works if outer_lines is one polyline)
    outer_polygon = Polygon(outer_lines[0].coords)

    # create new Polygon with holes
    holes = polygonize(holes)
    merged_polygon = outer_polygon.difference(holes)
    plot_polygon(merged_polygon)
    # now cast to wkt and write into a file
    out_file = "entrance_jupedsim.wkt"
    geomery_collection = GeometryCollection(merged_polygon)
    with open(out_file, 'a') as out:
        out.write(to_wkt(geomery_collection, rounding_precision=-1))
