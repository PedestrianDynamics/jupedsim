import ezdxf
from shapely import LineString, Polygon, polygonize
import matplotlib.pyplot as plt
import geopandas as gpd


def plot_polygon(polygon):
    """plots a polygon with its interior in matplotlib"""
    poly = gpd.GeoSeries([polygon])
    poly.plot()
    plt.show()


def line_to_linestring(line):
    start_point = entity.dxf.start
    end_point = entity.dxf.end
    return [(start_point[0], start_point[1]), (end_point[0], end_point[1])]

def polyline_to_linestring(polyline):
    points = []
    for point in polyline.get_points():
        print(point)
        points.append([point[0], point[1]])
    return points


def put_line_together(lines):
    polygon = lines[0]
    curr_pt = lines[0][-1]
    last_len = len(lines)
    lines.pop(0)
    while len(lines) > 0:
        if last_len == len(lines):
            print(curr_pt, "here we end", last_len)
            return polygon
        else:
            last_len = len(lines)
        for line in lines:
            if resembles(line[0], curr_pt):
                # first point of line matches
                for i in range(1, len(line)):
                    # add all points except matching one
                    polygon.append(line[i])
                curr_pt = line[-1]
                lines.remove(line)
                break
            elif resembles(line[-1], curr_pt):
                # first point of line matches
                for i in reversed(range(0, len(line)-1)):
                    # add all points except matching one
                    polygon.append(line[i])
                curr_pt = line[0]
                lines.remove(line)
                break
            else:
                # this line does not match
                continue
    return polygon


def resembles(pt1, pt2, epsilon=0.01):
    diff_x = abs(pt1[0] - pt2[0])
    diff_y = abs(pt1[1] - pt2[1])
    return  diff_x < epsilon and diff_y < epsilon


def plot_line_strings(lines):
    # Create a new figure and axis object
    fig, ax = plt.subplots()

    # Iterate over the LineString objects
    for line in lines:
        x_coords, y_coords = zip(*line)
        plt.plot(x_coords, y_coords)

    # Adjust the axis limits
    ax.autoscale()
    plt.show()


if __name__ == "__main__":
    # Path to the DXF file
    file_path = "./SiB2023_entrance_jupedsim.dxf"
    hole_lines = []
    outer_lines = []
    # Open the DXF file
    doc = ezdxf.readfile(file_path)

    # Access the model (modelspace)
    msp = doc.modelspace()

    # Iterate over all entities in the model
    for entity in msp:
        if entity.dxftype() == "LINE":
            if entity.dxf.layer == "jupedsim_holes":
                hole_lines.append(line_to_linestring(entity))
            elif entity.dxf.layer == "jupedsim_walkable_area":
                outer_lines.append(line_to_linestring(entity))
        elif entity.dxftype() == "LWPOLYLINE":
            if entity.dxf.layer == "jupedsim_holes":
                hole_lines.append(polyline_to_linestring(entity))
            elif entity.dxf.layer == "jupedsim_walkable_area":
                outer_lines.append(polyline_to_linestring(entity))
        else:
            print("entity type:", entity.dxftype())
            print()
    plot_line_strings(hole_lines)
    plot_line_strings(outer_lines)
    for line in outer_lines:
        print(line)
    outer_polygon = put_line_together(outer_lines)
    print(outer_polygon)
    plot_line_strings([outer_polygon])
