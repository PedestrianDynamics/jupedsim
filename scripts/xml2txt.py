"""
convert jpscore's trajectories from xml to txt format:
id   fr   x   y   z
input: xml file
output: xml file
"""

import os
from sys import argv, exit

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET


def write_header(o, fps, geometry_file):
    o.write("#xml to txt converted simulation file\n")
    o.write("#framerate: %0.2f\n" % fps)
    o.write("#geometry: %s\n" % geometry_file)
    o.write("#ID: the agent ID\n")
    o.write("#FR: the current frame\n")
    o.write("#X,Y,Z: the agents coordinates (in meters)\n")
    o.write("#A, B: semi-axes of the ellipse\n")
    o.write("#ANGLE: orientation of the ellipse\n")
    o.write("#COLOR: color of the ellipse\n")
    o.write("\n")
    o.write("#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\n")


if __name__ == "__main__":
    if len(argv) < 2:
        exit("usage: %s filename" % argv[0])

    filename = argv[1]
    filename1, file_extension = os.path.splitext(filename)
    if file_extension != ".xml":
        exit("ERROR: not an xml file")

    output = filename1 + ".txt"
    print(">> %s" % output)

    tree = ET.parse(filename)
    root = tree.getroot()

    for header in root.iter("header"):
        fps = header.find("frameRate").text

    try:
        fps = float(fps)
    except ValueError:
        exit("ERROR: could not read <fps>")

    for header in root.iter("header"):
        N = header.find("agents").text

    try:
        N = int(N)
    except ValueError:
        exit("ERROR: could not read <agents>")

    for g in root.iter("geometry"):
        file_location = g.find("file").attrib
        location = file_location["location"]

    with open(output, "w") as o:
        write_header(o, fps, location)
        for node in root.iter():
            tag = node.tag
            if tag == "frame":
                frame = node.attrib["ID"]
                for agent in list(node):
                    x = agent.attrib["x"]
                    y = agent.attrib["y"]
                    z = agent.attrib["z"]
                    rA = agent.attrib["rA"]
                    rB = agent.attrib["rB"]
                    eO = agent.attrib["eO"]
                    eC = agent.attrib["eC"]
                    ID = agent.attrib["ID"]
                    o.write(
                        "%d\t%d\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%.5f\t%d\n"
                        % (
                            int(ID),
                            int(frame),
                            float(x),
                            float(y),
                            float(z),
                            float(rA),
                            float(rB),
                            float(eO),
                            float(eC),
                        )
                    )
