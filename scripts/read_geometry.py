import xml.etree.ElementTree as ET


if __name__ == "__main__":
    tree = ET.parse('correct_aknz_geo_arrival.xml')
    root = tree.getroot()
    subrooms = root.findall('.//subroom')

    for subroom in subrooms:
        print('Subroom:', subroom.get('id'))
        polygons = subroom.findall('polygon')
        obstacles = subroom.findall('obstacle')
        for obstacle in obstacles:
            print('Obstacle:', obstacle.get('caption'))
            polygons = subroom.findall('polygon')
            for polygon in polygons:
                print('Obstacle-Polygon:', polygon.get('caption'))
                vertices = polygon.findall('vertex')
                for vertex in vertices:
                    x = vertex.get('px')
                    y = vertex.get('py')
                    print('  Vertex:', x, y)

        for polygon in polygons:
            print('Polygon:', polygon.get('caption'))
            vertices = polygon.findall('vertex')
            for vertex in vertices:
                x = vertex.get('px')
                y = vertex.get('py')
                print('  Vertex:', x, y)
