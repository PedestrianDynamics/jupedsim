"""
convert jpscore's trajectories from xml to txt format:
id   fr   x   y   z
input: xml file
output: xml file
"""

from sys import argv, exit
import os
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET


def write_header(o, fps, geometry_file):
    o.write("#xml to txt converted simulation file\n")
    o.write("#framerate: %0.2f\n" % fps)
    o.write("#geometry: %s\n" % geometry_file)
    o.write("#ID: the agent ID\n");
    o.write("#FR: the current frame\n");
    o.write("#X,Y,Z: the agents coordinates (in meters)\n");
    o.write("\n");
    o.write("#ID\tFR\tX\tY\tZ\n");


    
if __name__ == "__main__":
    if len(argv) < 2:
        exit('usage: %s filename' % argv[0])

    filename = argv[1]
    filename1, file_extension = os.path.splitext(filename)
    if file_extension != ".xml":
        exit('not an xml file')
        
    output = filename1 +  ".txt"
    o = open(output, "w")
    #    o.write("# ID             frame              x             y             z\n\n")
    print (">> %s" % output)

    tree = ET.parse(filename)
    root = tree.getroot()

    for header in root.iter('header'):
        fps = header.find('frameRate').text

    try:
        fps = float(fps)
    except:
        print ("ERROR: could not read <fps>")
        exit()

    for header in root.iter('header'):
        N = header.find('agents').text

    try:
        N = int(N)
    except:
        print ("ERROR: could not read <agents>")
        exit()

        
    for g in root.iter('geometry'):
        file_location = g.find('file').attrib
        location = file_location['location']
        
    write_header(o, fps, location)
    for node in root.iter():
        tag = node.tag
        if tag == "frame":
            frame = node.attrib['ID']
            for agent in node.getchildren():
                x = agent.attrib['x']
                y = agent.attrib['y']
                z = agent.attrib['z']
                ID = agent.attrib['ID']
                o.write("%d\t %d\t %.5f\t %.5f\t %.5f\n" % (int(ID), int(frame), float(x), float(y), float(z)))

    o.close()
