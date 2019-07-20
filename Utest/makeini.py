#!/usr/bin/env python3
import argparse
import errno
import glob
import logging
import os
import sys
import itertools
from shutil import copy2, rmtree, move
import numpy as np


try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

SUCCESS = 0
FAILURE = 1

ego = os.path.basename(sys.argv[0]).split(".")[0] + ".txt"
print("ego: ", ego)
logfile = "log_%s"%ego
logging.basicConfig(filename=logfile, level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
# ============= some directories =============
#HOME = os.path.expanduser("~")
#TRUNK =  HOME + "/Workspace/ped|ynamics/JuPedSim/jpscore/"
#JPSCORE = TRUNK + "bin/jpscore"
#CURDIR = os.getcwd()
# ============= some default dictionaries =============
default_value = {'tmax':1000, 'seed':1111, 'geometry':'', 'number':1, 'num_threads':1,
                 'file':'', 'model_id':1, 'exit_crossing_strategy':3, 'cell_size':2.2,
                 'operational_model_id':1}

# only these tags can be multiplied
tags = ['tmax',
        'seed',
        'geometry',
        'exit_crossing_strategy',
        'num_threads',
        'stepsize']

# format tag-attribute
attributes_tags = ['group-pre_movement_mean',
                   'group-number',
                   'group-router_id',
                   'group-agent_parameter_id',
                   'group-premovement_sigma',
                   'agents-operational_model_id',
                   'linkedcells-cell_size',
                   'v0-mu', 'v0-sigma',
                   'v0_upstairs-mu', 'v0_upstairs-sigma',
                   'v0_downstairs-mu', 'v0_downstairs-sigma',
                   'bmax-mu', 'bmin-mu',
                   'amin-mu', 'tau-mu',
                   'atau-mu',
                   'force_ped-dist_max',
                   'force_ped-disteff_max',
                   'force_ped-interpolation_width',
                   'force_ped-nu',
                   'force_ped-b',
                   'force_ped-c',
                   'force_wall-dist_max',
                   'force_wall-disteff_max',
                   'force_wall-interpolation_width',
                   'force_wall-nu',
                   'force_wall-b',
                   'force_wall-c',
                   'source-frequency',
                   'source-agents_max',
               ]


# cor_tags = np.unique([att.split("_")[0] for att in attributes_tags]).astype(str)a
cor_tags = [att.split("-")[0] for att in attributes_tags]
attributes = [att.split(tag+"-")[1] for (att, tag) in zip(attributes_tags, cor_tags)]
cor_tags = np.unique(cor_tags)
attributes = np.unique(attributes)
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
        geometries = []
        geom = glob.glob("%s/*.xml"%node.text)
        for g in geom:
            geometries.append('../geometries' + g.split(".xml")[0].split("geometries")[1] + ".xml")
        # the geometries are relative to the inifiles directory
        #print geometries
        return  geometries
    else:
        text = node.text

    if text:
        value = eval(text)
    else:
        value = default_value[node.tag]
    return value
# =======================================================
def get_attribute(node):
    text = ''
    values = []

    for node_attrib in list(node.attrib.keys()):
        if node_attrib in attributes:
            text = node.attrib[node_attrib]
            if text:
                value = eval(text)
            else:
                value = 0
            if isinstance(value, list) or isinstance(value, np.ndarray):
                if len(value) > 1:
                    values.append([value, str(node.tag)+"-"+str(node_attrib), node_attrib])

    return values
# =======================================================
def get_product(root):
    """
    read values, which may be lists or arrays of len>1 and  return a list of
    dics composed of the cartesian product of these lists.
    example:
    we read from the file (xml --> root) the following
    {'num_threads': [5, 1, 2], 'tmax': [1, 2]}
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
            if isinstance(d, list)  or isinstance(d, np.ndarray):
                # in case some tags have multiple values
                if tag not in input_tags and len(d) > 1:
            # ignore lists with one element (equiv to scalars)
            # if tag in tags:
                    input_tags[tag] = d
        elif bool(set(node.attrib.keys()) & set(attributes)): # check our list of attributes
            values = get_attribute(node) # d, atr_tag, attr
            # value, atr_tag, atr
            for value in values:
                d = value[0]
                atr_tag = value[1]
                input_tags[atr_tag] = d
        else:
            continue

    result_prod = [dict(zip(input_tags, x)) for x in itertools.product(*iter(input_tags.values()))]
    return result_prod
# =======================================================
def make_filename(directory, d):
    name = "%s/inifiles/ini"%directory
    traj = "../trajectories/traj" #%directory
    for key, value in d.items():
        if key == "geometry":
            value = os.path.basename(value)
        # if key == "num_threads":
            # value = "numCPU"
        name += "_" + key + "_" + str(value)
        traj += "_" + key + "_" + str(value)

    if not name.endswith("xml"):
        name += ".xml"
    if not traj.endswith("xml"):
        traj += ".xml"
    #print "name", (name)
    return name, traj
# =======================================================
def update_tag_value(root, tag, value):
    for rank in root.iter(tag):
        rank.text = str(value)
# =======================================================
def update_attrib_value(root, attr_tag, value):
    # location
    print ("update_attrib_value: ", attr_tag, value)
    # raw_input()
    if attr_tag == "location":  # e.g. location
        for r in root.iter():
            if attr_tag in r.attrib:
                r.attrib[attr_tag] = str(value)
        return

    attr = attr_tag.split("-")[1]
    cor_tag = attr_tag.split("-")[0]

    for r in root.iter(cor_tag):
        if attr in r.attrib:
            r.attrib[attr] = str(value)
# =======================================================
def make_file(masterfile, tree, result):
    """
    Given a list of dictionaries produce an xml file for each dic.
    The file is first copied from masterfile.
    """
    directory = os.path.dirname(os.path.abspath(masterfile))    #args.directory
    root = tree.getroot()
    for item in result:
        newfile, trajfile = make_filename(directory, item)
        copy2(masterfile, newfile)
        #update trajectory file
        update_attrib_value(root, "location", trajfile)
        if not os.path.isfile(newfile):
            logging.error("make_file: could not create file %s"%newfile)
            sys.exit(FAILURE)
        for tag, value in item.items():
            # print "tag: ", tag, "value:", value
            # raw_input()
            if tag in attributes_tags:
                update_attrib_value(root, tag, value)
            else:
                update_tag_value(root, tag, value)

        logging.info('>> %s'%newfile)
        tree.write(newfile)
# =======================================================

if __name__ == "__main__":
    args = getParserArgs()

    masterfile = args.file
    if not os.path.isfile(masterfile):
        logging.error("ERROR: file %s does not exist."%masterfile)
        sys.exit(FAILURE)

    directory = os.path.dirname(os.path.abspath(masterfile))    #args.directory
    logging.info('working directory = <%s>'%directory)
    logging.info('master inifile = <%s>'%masterfile)
    make_dir("%s/trajectories"%directory)
    make_dir("%s/inifiles"%directory)

    tree = ET.parse(masterfile)
    root = tree.getroot()
    result = get_product(root)
    make_file(masterfile, tree, result)

    if not  os.path.isfile("%s/%s"%(directory, logfile)):
        move(logfile, directory)
