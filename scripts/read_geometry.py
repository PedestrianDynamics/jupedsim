import xml.etree.ElementTree as ET
from shapely.geometry import Polygon
from shapely.ops import polygonize
from shapely.ops import unary_union
import matplotlib.pyplot as plt

if __name__ == "__main__":
    tree = ET.parse('correct_aknz_geo_arrival.xml')
    root = tree.getroot()
    rooms = root.findall('.//room')
    # list with all polygons from this geometry
    geometry_polygons = []
    for room in rooms:
        subrooms = room.findall('.//subroom')
        for subroom in subrooms:
            room_obstacles = []
            room_polygon = []
            room_lines = []
            # parse all obstacles into room obstacles list
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
                            last_point = point

                room_obstacles.append(temp_obstacle)
            # parse all edges of polygon into room_lines list
            polygons = subroom.findall('polygon')
            for polygon in polygons:
                vertices = polygon.findall('vertex')
                point1 = (float(vertices[0].get('px')), float(vertices[0].get('py')))
                point2 = (float(vertices[1].get('px')), float(vertices[1].get('py')))
                room_lines.append([point1, point2])

            # add all transitions with according room id and subroom id to room_lines list
            transitions = root.findall('.//transition')
            subroom_id = subroom.attrib['id']
            room_id = room.attrib['id']
            for transition in transitions:
                room1_id = transition.attrib['room1_id']
                room2_id = transition.attrib['room2_id']
                sub1_id = transition.attrib['subroom1_id']
                sub2_id = transition.attrib['subroom2_id']
                if sub1_id == subroom_id and room_id == room1_id \
                        or sub2_id == subroom_id and room2_id == room_id:
                    vertices = transition.findall('vertex')
                    point1 = (float(vertices[0].get('px')), float(vertices[0].get('py')))
                    point2 = (float(vertices[1].get('px')), float(vertices[1].get('py')))
                    room_lines.append([point1, point2])

            # put lines together to form a polygon
            room_polygon = polygonize(room_lines)[0]

            # create polygon from room and obstacles and add to list of geometries polygons
            geometry_polygons.append(Polygon(room_polygon.exterior, room_obstacles))

    merged_polygon = unary_union(geometry_polygons)
    # -- only plotting from here --

    # for polygon in geometry_polygons:
    #    plt.plot(*polygon.exterior.xy)
    # plt.show()

    plt.plot(*merged_polygon.exterior.xy)
    plt.show()
