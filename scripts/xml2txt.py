"""
convert jpscore's trajectories from xml to txt format:
id   fr   x   y
input: xml file
output: xml file
"""

from sys import argv, exit

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

if __name__ == "__main__":
    if len(argv) < 2:
        exit('usage: %s filename' % argv[0])

    filename = argv[1]
    output = filename.split(".")[0] + ".txt"
    o = open(output, "w")
    o.write("# ID             frame              x             y")
    print (">> %s" % output)

    tree = ET.parse(filename)
    root = tree.getroot()
    for node in root.iter():
        tag = node.tag
        if tag == "frame":
            frame = node.attrib['ID']
            for agent in node.getchildren():
                x = agent.attrib['x']
                y = agent.attrib['y']
                ID = agent.attrib['ID']
                o.write("%d\t %d\t %.3f\t %.3f\n" % (int(ID), int(frame), float(x), float(y)))

    o.close()
