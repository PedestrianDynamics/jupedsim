import xml.etree.ElementTree as ET
from shapely.geometry import Polygon
from shapely.geometry import LineString, Point
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
            ordered_lines = []
            room_polygon = []
            room_lines = []
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
                            last_point = point

                room_obstacles.append(temp_obstacle)

            for polygon in polygons:
                vertices = polygon.findall('vertex')
                point1 = (float(vertices[0].get('px')), float(vertices[0].get('py')))
                point2 = (float(vertices[1].get('px')), float(vertices[1].get('py')))
                room_lines.append([point1, point2])

            transitions = root.findall('.//transition')
            subroom_id = subroom.attrib['id']
            for transition in transitions:
                if transition.attrib['subroom1_id'] == subroom_id \
                        and room.attrib['id'] == transition.attrib['room1_id']\
                        or\
                        transition.attrib['subroom2_id'] == subroom_id \
                        and room.attrib['id'] == transition.attrib['room2_id']:
                    vertices = transition.findall('vertex')
                    point1 = (float(vertices[0].get('px')), float(vertices[0].get('py')))
                    point2 = (float(vertices[1].get('px')), float(vertices[1].get('py')))
                    room_lines.append([point1, point2])


            # remove double entries
            unique_lines = []
            for line in room_lines:
                if line not in unique_lines:
                    unique_lines.append(line)
            room_lines = unique_lines

            # sort unordered lines to ordered lines
            ending_pt = room_lines[0][0]
            curr_pt = room_lines[0][1]
            ordered_lines.append(room_lines[0])
            room_lines.pop(0)
            last_pt = curr_pt
            while len(room_lines) > 0:
                for i, line in enumerate(room_lines):
                    if line[0] == curr_pt:
                        curr_pt = line[1]
                        ordered_lines.append(room_lines[i])
                        room_lines.pop(i)
                        break
                    if line[1] == curr_pt:
                        curr_pt = line[0]
                        ordered_lines.append(room_lines[i])
                        room_lines.pop(i)
                        break
                if curr_pt == last_pt and len(room_lines) > 0:
                    print("something doesn´t add up")
                    break
                else:
                    last_pt = curr_pt

            for line in ordered_lines:
                if line[0] not in room_polygon:
                    room_polygon.append(line[0])
                if line[1] not in room_polygon:
                    room_polygon.append(line[1])

            # subroom geometry is now parsed
            geometry_polygons.append(Polygon(room_polygon, room_obstacles))

    #for polygon in geometry_polygons:
    #    plt.plot(*polygon.exterior.xy)
    #plt.show()

    merged_polygon = unary_union(geometry_polygons)
    plt.plot(*merged_polygon.exterior.xy)
    plt.show()
