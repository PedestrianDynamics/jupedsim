import numpy as np
import matplotlib.pyplot as plt
from sys import argv

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET


def get_polygon(poly):
    X = []
    Y = []
    # for poly in node.getchildren():
    #     # print "polygon tag: ", poly.tag
    #     # if poly.tag == "obstacle":
    #     #     # print poly.getchildren()
    #     #     # for pobst in poly.getchildren():
    #     #     #     # print pobst.tag
    #     #     #     for q in pobst.getchildren(): # vertex
    #     #     #         X.append( q.attrib['px'] )
    #     #     #         Y.append( q.attrib['py'] )
    #     #     pass
    #     # else:
    for p in poly.getchildren(): # vertex
        X.append(p.attrib['px'])
        Y.append(p.attrib['py'])

    return X, Y

def plot_geometry(filename):
    tree = ET.parse(geometry)
    root = tree.getroot()
    for node in root.iter():
        tag = node.tag
        # print "subroom tag", tag
        if tag == "polygon":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "k", lw=2)
        elif tag == "obstacle":
            # print "obstacle tag",tag
            for n in node.getchildren():
                # print "N: ", n
                X, Y = get_polygon(n)
                # print "obstacle", X, Y
                plt.plot(X, Y, "g", lw=2)
        elif tag == "crossing":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--b", lw=0.9, alpha=0.2)
        elif tag == "HLine":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--b", lw=0.9, alpha=0.2)
        elif tag == "transition":
            X, Y = get_polygon(node)
            plt.plot(X, Y, "--r", lw=1.6, alpha=0.2)



geometry = argv[1]
plot_geometry(geometry)

plt.show()
