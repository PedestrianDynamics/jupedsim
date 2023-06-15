import xml.etree.ElementTree as ET
from shapely import to_wkt, GeometryCollection
from shapely.geometry import Polygon
from shapely.ops import polygonize
from shapely.ops import unary_union
import matplotlib.pyplot as plt
import geopandas as gpd

"""This script takes a XML file containing a geometry as input and 
returns a list of Shapely polygons corresponding to the geometry.
The script also includes functions for converting the polygons to WKT format and
plotting the polygons using the Matplotlib and Geopandas libraries."""


def parse_geo_file(geo_file):
    """
    Parses the geo_file returns all rooms as shapely polygon.
    :param geo_file: The file containing the geometry data.
    :returns: geo_file as list of rooms as polygon
    :rtype: list of shapely polygons corresponding to the geometry
    """
    tree = ET.parse(geo_file)
    root = tree.getroot()
    rooms = root.findall('.//room')
    # list with all polygons from this geometry
    geometry_polygons = []
    for room in rooms:
        subrooms = room.findall('.//subroom')
        for subroom in subrooms:
            room_obstacles = []
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

                room_obstacles.append(temp_obstacle)
            # parse all edges of polygon into room_lines list
            polygons = subroom.findall('polygon')
            for polygon in polygons:
                vertices = polygon.findall('vertex')
                room_lines.append([(float(vertex.get('px')), float(vertex.get('py'))) for vertex in vertices])

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
                    # the transition matches the current subroom
                    vertices = transition.findall('vertex')
                    point1 = (float(vertices[0].get('px')), float(vertices[0].get('py')))
                    point2 = (float(vertices[1].get('px')), float(vertices[1].get('py')))
                    room_lines.append([point1, point2])
            # add all crossings from the room to room_lines list
            crossings = room.findall('crossings')
            if len(crossings) > 0:
                # at least one crossing for this room found
                crossings = crossings[0]
                for crossing in crossings:
                    sub1_id = crossing.attrib['subroom1_id']
                    sub2_id = crossing.attrib['subroom2_id']
                    if sub1_id == subroom_id or sub2_id == subroom_id:
                        # the transition matches the current subroom
                        vertices = crossing.findall('vertex')
                        point1 = (float(vertices[0].get('px')), float(vertices[0].get('py')))
                        point2 = (float(vertices[1].get('px')), float(vertices[1].get('py')))
                        room_lines.append([point1, point2])
            # put lines together to form a polygon
            try:
                # create polygon from lines parsed from this room
                room_polygon = polygonize(room_lines)[0]
                # create polygon from this rooms polygon and parsed obstacles and add to list of geometries polygons
                geometry_polygons.append(Polygon(room_polygon.exterior, room_obstacles))
            except Exception:
                message = f"there has been an error parsing subroom {subroom_id} in room {room_id}"
                raise RuntimeError(message)

    return geometry_polygons


def convert_to_wkt(geometry_polygons, out_file, all_rooms=False):
    """
    converts geometry_polygons into wkt and writes them in out_file

    :param geometry_polygons: list of shapely polygons corresponding to the geometry

    :param out_file: The file to write the output to.

    :param all_rooms: (optional) If True, writes each room to the output file separately.
                      If False, writes only the entire geometry to the output file.
    :type all_rooms: bool
    """
    if all_rooms:
        geomery_collection = GeometryCollection(geometry_polygons)
        with open(out_file, 'a') as out:
            out.write(to_wkt(geomery_collection))
    else:
        merged_polygon = unary_union(geometry_polygons)
        geomery_collection = GeometryCollection(merged_polygon)
        with open(out_file, 'a') as out:
            out.write(to_wkt(geomery_collection))


def remove_existing_rooms(existing_polygons, new_polygons):
    """returns all polygons that do not already exist"""
    exists = [False] * len(new_polygons)
    for i, poly in enumerate(new_polygons):
        for existing_polygon in existing_polygons:
            if poly.equals(existing_polygon):
                exists[i] = True
                break
    remaining_polygons = []
    for i in range(len(exists)):
        if not exists[i]:
            remaining_polygons.append(new_polygons[i])
    return remaining_polygons


def plot_polygon(polygon):
    """plots a polygon with its interior in matplotlib"""
    poly = gpd.GeoSeries([polygon])
    poly.plot()
    plt.show()
