
# help: python makeini.py -h
import os, sys, glob
from shutil import copy2, rmtree, move
import logging, types, argparse
import errno, time
from numpy import *
from itertools import product, izip
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

SUCCESS = 0
FAILURE = 1
ego = os.path.splitext(sys.argv[0])[0] + ".txt"

logfile = "log_%s"%ego
logging.basicConfig(filename=logfile, level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
# ============= some directories =============
#HOME = os.path.expanduser("~")
#TRUNK =  HOME + "/Workspace/peddynamics/JuPedSim/jpscore/"
#JPSCORE = TRUNK + "bin/jpscore"
#CURDIR = os.getcwd()
# ============= some default dictionaries =============
default_value = {'tmax':1000, 'seed':1111, 'geometry':'', 'number':1, 'numCPU':1, 'file':'', 'model_id':1, 'exitCrossingStrategy':3, 'cell_size':2.2, 'operational_model_id':1}
tags = ['tmax', 'seed', 'geometry', 'exitCrossingStrategy', 'numCPU']     # only these tags can be multiplied
attributes = ['number', 'operational_model_id', 'cell_size', 'router_id'] # these attributes too, but
tags2attributes = ['group', 'agents', 'linkedcells', 'router' ]           # only these corresponding to these tags
input_tags = {}
# =======================================================
def getParserArgs():
    parser = argparse.ArgumentParser(description='Generate inifiles for jpscore simulations. ')
    parser.add_argument("-f", "--file", metavar='in-file', required=True, help='Master inifile')
    args = parser.parse_args()
    return args

# =======================================================
def make_dir(path):
    if os.path.exists(path):
        rmtree(path)
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise
# =======================================================
def get_tag(node):
    # geometry
    if node.tag == "geometry":
        return  glob.glob("%s/*.xml"%node.text)
    else:
        text = node.text
        
    if text:
        value = eval(text)
    else:
        value = default_value[node.tag]
    return value
# =======================================================
def get_attribute(node):
    for atr in attributes: # todo suppose that attributes are distinct
        if node.attrib.has_key(atr):
            text = node.attrib[atr]
            break

    if text:
        value = eval(text)
    else:
        value = default_value[node.tag]
    return value, atr
# =======================================================
def get_product(root):
    """
    read values, which may be lists or arrays of len>1 and  return a list of
    dics composed of the cartesian product of these lists.
    example:
    we read from the file (xml --> root) the following
    {'numCPU': [5, 1, 2], 'tmax': [1, 2]}
    return is:
    [
    {'numCPU': 5, 'tmax': 1}, {'numCPU': 5, 'tmax': 2},
    {'numCPU': 1, 'tmax': 1}, {'numCPU': 1, 'tmax': 2},
    {'numCPU': 2, 'tmax': 1}, {'numCPU': 2, 'tmax': 2}
    ]
    """
    for node in root.iter():
        tag = node.tag        
        if tag in tags:   # ignore tags that are not of interest
            d = get_tag(node)
        elif bool( set(node.attrib.keys() ) & set(attributes) ): # check our list of attributes
            d, attr = get_attribute(node)
        else:
            continue

        
        if isinstance(d, list) or isinstance(d, ndarray): # in case some tags have multiple values
            if not input_tags.has_key(tag) and len(d)>1: # ignore lists with one element (equiv to scalars)
                if tag in tags:
                    input_tags[tag] = d
                else:
                    input_tags[attr] = d
            # else: # should not happen
            #     sys.exit("Tag %s already exists! What the hell?"%tag)
    result = [dict(izip(input_tags, x)) for x in product(*input_tags.itervalues())]
    return result
# =======================================================
def make_filename(directory, d):
    name = "%s/inifiles/ini"%directory
    traj = "../trajectories/traj" #%directory
    for key, value in d.iteritems():
        if key == "geometry":
            value = os.path.basename(value)
        name += "_" + key + "_" + str(value)
        traj += "_" + key + "_" + str(value)
    name += ".xml"
    traj += ".xml"
    print (name)
    return name, traj
# =======================================================
def update_tag_value(root, tag, value):
    for rank in root.iter(tag):
        rank.text = str(value)
# =======================================================
def update_attrib_value(root, attr, value):
    
    indexes = [i for i, j in enumerate(attributes) if j == attr]
    if len(indexes) == 0:  # e.g. location
        for r in root.iter():
            if r.attrib.has_key(attr):
                r.attrib[attr] = str(value)
        return
    
    index = indexes[0]
    cor_tag = tags2attributes[ index ]
    for r in root.iter(cor_tag):
        if r.attrib.has_key(attr):
            r.attrib[attr] = str(value)
# =======================================================
def make_file(masterfile, tree, result):
    """
    Given a list of dictionaries produce an xml file for each dic.
    The file is first copied from masterfile.
    """
    directory = os.path.dirname(masterfile)    #args.directory
    root = tree.getroot()
    for item in result:
        newfile, trajfile = make_filename(directory, item)
        copy2(masterfile, newfile)
        #update trajectory file
        update_attrib_value(root, "location", trajfile)
        if not os.path.isfile (newfile):
            logging.error("make_file: could not create file %s"%newfile)
            sys.exit(FAILURE)
        for tag, value in item.iteritems():
            if tag in attributes:
                update_attrib_value(root, tag, value)
            else:
                update_tag_value(root, tag, value)

        logging.info('>> %s'%newfile)
        tree.write(newfile)
# =======================================================

if __name__ == "__main__":
    time1 = time.clock()    
    args = getParserArgs()

    masterfile = args.file
    if not os.path.isfile(masterfile):
        logging.error("ERROR: file %s does not exist."%masterfile)
        sys.exit(FAILURE)
    
    directory = os.path.dirname(masterfile)    #args.directory
    logging.info('working directory = <%s>'%directory)
    logging.info('master inifile = <%s>'%masterfile)
    make_dir( "%s/trajectories"%directory)
    make_dir( "%s/inifiles"%directory)
    
    tree = ET.parse(masterfile)
    root = tree.getroot()
    result = get_product(root)
    make_file(masterfile, tree, result)
    # executable = "%s/bin/jpscore"%TRUNK
    # if not path.exists(executable):
    #     logging.critical("executable <%s> does not exist yet."%executable)
    #     sys.exit(FAILURE)
    # for inifile in inifiles:
    #     logging.info('start simulating with inifile=<%s>'%(cmd, inifile))
    #     subprocess.call([executable, "--inifile=%s"%inifile])
#          
    time2 = time.clock()
    if not  os.path.isfile("%s/%s"%(directory, logfile)):
        move(logfile, directory)
    logging.info('time elapsed: %.2f to generate %d files'%(time2 - time1, len(result)))
    if 0:
        sys.exit(FAILURE)
    else:
        sys.exit(SUCCESS)

    
