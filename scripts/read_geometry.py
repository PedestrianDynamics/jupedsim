import xml.etree.ElementTree as ET

if __name__ == "__main__":
    tree = ET.parse('correct_aknz_geo_arrival.xml')
    root = tree.getroot()
    subrooms = root.findall('.//subroom')

    
    for subroom in subrooms:
        print('Subroom:', subroom.get('id'))
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

        print(room_polygon)
        print(room_obstacles)
