import numpy as np
import matplotlib.pyplot as plt
from sys import argv, exit
import os, time

from pylab import Rectangle

if len(argv)<=2:
    print("usage: %s, filename geometryfile"%argv[0])
    exit()

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

# tree = ET.parse(argv[2])
# root = tree.getroot()
# for node in root.iter():
#     tag = node.tag        
#     if tag == "polygon":
        

def get_polygon(node):
    X = []
    Y = []
    for p in node.getchildren():
        X.append( p.attrib['px'] )
        Y.append( p.attrib['py'] )
    return X, Y
def plot_geometry(filename):
    tree = ET.parse(geometry)
    root = tree.getroot()
    for node in root.iter():
        tag = node.tag        
        if tag == "polygon":
            X, Y = get_polygon(node) 
            plt.plot(X,Y, "k", lw=2)
        elif tag == "obstacle":
           for n in node.getchildren():
            X, Y = get_polygon(n)
           # print "obstacle", X, Y
            plt.plot(X,Y, "m", lw=2)
        elif tag == "crossing":
            X, Y = get_polygon(node) 
            plt.plot(X,Y, "--b", lw=2, alpha=0.5)
        elif tag == "HLine":
            X, Y = get_polygon(node) 
            plt.plot(X,Y, "--b", lw=2, alpha=0.5)
        elif tag == "transition":
            X, Y = get_polygon(node) 
            plt.plot(X,Y, "--b", lw=2, alpha=0.5)
def plotFrame(i, t, data, geometry):

    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    for line in data:
        p1x = line[0]
        p1y = line[1]
        p2x = line[2]
        p2y = line[3]
        connected = line[4]
        id0 = line[5]
        id1 = line[6]
        ms = 10
        r = 2.2
        ped = 8#10

            
            
        if id0 == ped:
            # print "t=%f  p1 %d p2 %d. connet %d"%(t, id0, id1, connected)
            circ = plt.Circle((p1x, p1y), radius=r, alpha =.2, fc='grey')
            rect = Rectangle((p1x-r,p1y-r),2*r,2*r, alpha =.1, fc='grey')
            rect0 = Rectangle((p1x-3*r,p1y-3*r),6*r,6*r, alpha =.1, fc='grey')
            rect1 = Rectangle((p1x+r,p1y-r),2*r,2*r, alpha =.1, fc='grey')
            rect2 = Rectangle((p1x-3*r,p1y-r),2*r,2*r, alpha =.1, fc='grey')
            rect3 = Rectangle((p1x-r,p1y+r),2*r,2*r, alpha =.1, fc='grey')
            rect4 = Rectangle((p1x-r,p1y-3*r),2*r,2*r, alpha =.1, fc='grey')

            ax.add_patch(rect)
            ax.add_patch(rect1)
            ax.add_patch(rect0)
            ax.add_patch(rect2)
            ax.add_patch(rect3)
            ax.add_patch(rect4)
            ax.add_patch(circ)
        dx = 0.5
        annotate=1
        if connected == 1:
            plt.plot((p2x), (p2y), 'or')
            if annotate:
                plt.annotate('%d'%id1, xy=(p2x, p2y), xytext=(p2x-dx, p2y-dx))
       
            plt.plot((p1x), (p1y), 'ok')
            #plt.annotate('%d'%id0, xy=(p1x, p1y), xytext=(p1x-dx, p1y-dx))
            plt.plot((p1x, p2x), (p1y, p2y), '-g', lw=1)
        # elif connected == -1:  # same peds
        #     #plt.plot((p2x), (p2y), 'or')
        #     #plt.plot((p1x), (p1y), 'ok', ms=ms)
        #     plt.plot((p1x), (p1y), 'ob', alpha=0.7)
        #     # plt.annotate('%d'%id1, xy=(p2x, p2y), xytext=(p2x-0.3, p2y-0.3))
        #     # plt.annotate('%d'%id0, xy=(p1x, p1y), xytext=(p1x-0.3, p1y-0.3))
            
        else:
            plt.plot((p2x), (p2y), 'or')
            #plt.plot((p1x), (p1y), 'ok', ms=ms)
            if connected == 0:
                plt.plot((p1x, p2x), (p1y, p2y), '--k', alpha=0.5)
                plt.annotate('%d'%id1, xy=(p2x, p2y), xytext=(p2x-dx, p2y-dx))
                #plt.annotate('%d'%id0, xy=(p1x, p1y), xytext=(p1x-dx, p1y-dx))

       
        # plot dummy peds
        # plt.plot((45), (108), '.k')
        # plt.plot((45), (94), '.k')
        # plt.plot((75), (94), '.k')
        # plt.plot((75), (108), '.k')


        #geometry
#         plt.plot([50, 65, 62, 62, 60, 60, 56], [104, 104, 104, 102.45, 102.45, 104, 104], 'k', lw=2)
#         plt.plot([50, 65, 62, 62, 60, 60, 56], [100, 100, 100, 101.4, 101.4, 100, 100], 'k', lw=2)
#         plt.plot([56,56], [100,104], "--b", alpha=0.3)
# #        plt.plot([54,54], [100,104], "--b", alpha=0.3)
#         plt.plot([62,62], [101.4,102.45], "--b", alpha=0.3)
        plot_geometry(geometry)
        # plt.xlim((48,66))
        # plt.ylim((99,105))
        #plt.xlim((-30,50))
        #plt.ylim((126,140))
        plt.axis('scaled')
        
    plt.title("t=%.3f"%t)
    fig.savefig("figs_bot/%.4d.png"%i)
    plt.clf()
    print("---> figs_bot/%.4d.png"%i) 


if __name__ == "__main__":
    Plot = 1
    Movie = 0
    if Plot:
        geometry = argv[2]
        filename = argv[1]
        d = np.loadtxt(filename)
        times = np.unique(d[:,0])
        i=0
        for t in times[::]:
            if t <= 1.9 or t>2: continue
            data = d[ d[:,0] == t ][:,1:]
            plotFrame(i, t, data, geometry)
            i += 1
    if Movie:
        cmd = "\"mf://figs_bot/*.png\" -mf w=800:h=600:fps=25:type=png -ovc lavc -lavcopts vcodec=mpeg4 -oac copy -o output.avi"
        print("Mencoder ---- ")
        os.system("mencoder %s" %cmd)
        MPlayer= "mplayer output.avi"
        os.system("%s" %MPlayer)

