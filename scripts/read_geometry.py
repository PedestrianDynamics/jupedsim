import xml.etree.ElementTree as ET
from shapely.geometry import Polygon
from shapely.ops import unary_union
import matplotlib.pyplot as plt

if __name__ == "__main__":
    tree = ET.parse('correct_aknz_geo_arrival.xml')
    root = tree.getroot()
    subrooms = root.findall('.//subroom')

    # list with all polygons from this geometry
    geometry_polygons = []
    for subroom in subrooms:
        room_obstacles = []
        room_polygon = []
        polygons = subroom.findall('polygon')
        obstacles = subroom.findall('obstacle')
        for obstacle in obstacles:
            temp_obstacle = []
            obstacle_polygons = obstacle.findall('polygon')
            for polygon in obstacle_polygons:
                vertices = polygon.findall('vertex')
                for vertex in vertices:
                    point = (float(vertex.get('px')), float(vertex.get('py')))
                    if point not in temp_obstacle:
                        temp_obstacle.append(point)

            room_obstacles.append(temp_obstacle)

        for polygon in polygons:
            vertices = polygon.findall('vertex')
            for vertex in vertices:
                point = (float(vertex.get('px')), float(vertex.get('py')))
                if point not in room_polygon:
                    room_polygon.append(point)
        # subroom geometry is now parsed
        geometry_polygons.append(Polygon(room_polygon, room_obstacles))

    for polygon in geometry_polygons:
        plt.plot(*polygon.exterior.xy)

    plt.show()

    # merged_polygon = unary_union(geometry_polygons)
    # plt.plot(*merged_polygon.exterior.xy)
    # plt.show()
