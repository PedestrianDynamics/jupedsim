#!/usr/bin/env python3
# helper functions for testing sources
import os
from sys import path
from math import ceil
from typing import List
from dataclasses import dataclass, field
import logging
from utils import equals, contains
import numpy as np

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET


@dataclass
class Source:
    ids: List[int] = field(default_factory=list)
    caption: str = None
    group_id: int = None
    agent_id: int = None
    time_min: int = None
    time_max: int = None
    xmin: float = None
    xmax: float = None
    ymin: float = None
    ymax: float = None
    startX: float = None
    startY: float = None
    frequency: int = None
    rate: int = None
    percent: float = 1
    N_create: int = 1
    agents_max: int = None
    time: int = None
    greedy: bool = None

    def possible_creations_with_np(self) -> int:
        """
        How many times can we create <np> agents in a subcycle?
        """
        Np = int(self.N_create*self.percent)
        return int(self.N_create%Np != 0) + self.N_create//Np
    def num_possible_creations(self) -> int:
        if self.rate == None:
            return 1

        max_creations_in_subcycle = ceil(self.frequency/self.rate)
        return min(self.possible_creations_with_np(), max_creations_in_subcycle)

    def num_cycles(self) -> int:
        return int((self.time_max - self.time_min)//self.frequency)

    def sub_cycle(self) -> List[int]:
        if self.rate == None:
            rate = self.frequency
        else:
            rate = self.rate

        subcycle = []
        Np = int(self.N_create*self.percent)
        rest = Np
        for i in range(self.num_possible_creations()):
            if rest > Np:
                L = [self.time_min+i*rate]*Np
            else:
                L = [self.time_min+i*rate]*rest

            rest = self.N_create - Np
            subcycle += L

        return subcycle

    def cycle(self) -> List[int]:
        subcycle = self.sub_cycle()
        cycle = subcycle
        for i in range(1, self.num_cycles()+2):
            cycle = np.hstack((cycle, subcycle + i*self.frequency*np.ones(len(subcycle))))

        return cycle[cycle <= self.time_max]


def get_data(_trajfile):
    """
    Read data from file
    @param:
    _trajfile: trajectory file(type: str)
    @return
    time_err: defined as 3/fps, if framerate is defined
    d[ped_id] = [time, x, y, group] (type: dic)
    """
    _d = {}
    _time_err = 0.01
    f = open(_trajfile)
    lines = f.readlines()
    f.close()
    for line in lines:
        line = line.strip()
        if line.startswith("#framerate"):
            fps = float(line.split(":")[1])
            _time_err = 3.0/fps

        if not line or line.startswith("#"):
            continue

        line = line.split()
        pid = int(line[0])
        frame = int(line[1])
        x = float(line[2])
        y = float(line[3])
        group_id = int(line[9])
        time = frame/fps

        if pid not in _d:
            _d[pid] = [time, x, y, group_id]

    return _time_err, _d

def test_source(d, s, time_err, pos_err):
    """
    @param
    s: source (type: Source)
    d: data from trajectory file (type: dic)
    time_err: time threshold (type: float)
    time_pos: space threshold (type: float)
    @return:
    False if someting went wrong. Otherwise, return True
    """
    logging.info("testing source ids: %s", ", ".join(map(str, s.ids)))
    debug_msg = ("source properties: \n\t\t\t\t time_min="+str(s.time_min)+" time_max="+str(s.time_max)+"\n"
                 "\t\t\t\t xmin="+str(s.xmin)+" xmax="+str(s.xmax)+"\n"
                 "\t\t\t\t ymin="+str(s.ymin)+" ymax="+str(s.ymax)+"\n"
                 "\t\t\t\t startX="+str(s.startX)+" startY="+str(s.startY)+"\n"
                 "\t\t\t\t frequency="+str(s.frequency)+"\n"
                 "\t\t\t\t percent="+str(s.percent)+"\n"
                 "\t\t\t\t rate="+str(s.rate)+"\n"
                 "\t\t\t\t n_create="+str(s.N_create)+"\n"
                 "\t\t\t\t agents_max="+str(s.agents_max)+"\n"
                 "\t\t\t\t time="+str(s.time))
    logging.info(debug_msg)
    times = []
    # TODO
    # if s.N_create:
    #     correct_length = s.N_create == len(s.ids)
    #     if no correct_length:
    #         logging.error("expected "+ str(s.N_create)
    #                       ", but got "+str(len(s.ids))
    #                       )
    #         return False

    for j, source_id in enumerate(s.ids):
        in_time_interval = True
        in_box_x = True
        in_box_y = True
        on_time = True
        if not source_id in d.keys():
            logging.error("source_id: %d is not in data", source_id)
            msg = ("Data keys : "+", ".join(map(str, sorted(d.keys()))))
            logging.error(msg)
            return False

        if s.time:
            on_time = equals(d[source_id][0], s.time, err=time_err)

        if s.time_min and s.time_max:
            in_time_interval = contains(d[source_id][0], s.time_min, s.time_max, time_err)

        if s.xmin and s.xmax:
            in_box_x = contains(d[source_id][1], s.xmin, s.xmax, pos_err)

        if s.ymin and s.ymax:
            in_box_y = contains(d[source_id][2], s.ymin, s.ymax, pos_err)

        # frequency
        if s.frequency:
            times.append(d[source_id][0])

        if not in_time_interval or \
        not in_box_x or \
        not in_box_y or \
        not on_time:
            err_msg = ("source id "+str(source_id)+" "
                       "creation time at "+str(d[source_id][0])+
                       " in position ("+str(d[source_id][1])+", "+ str(d[source_id][2])+")")

            logging.error(err_msg)
            return False

    if not times:
        return True

    msg = ("Got times : "+", ".join(map(str, times)))

    logging.info(msg)
    should_be = s.cycle()
    # test ids
    if len(s.ids) != len(should_be):
        logging.error("length of ids is not correct")
        msg = ("ids : "+", ".join(map(str, s.ids)))
        logging.error(msg)
        msg = ("should be : "+", ".join(map(str, should_be)))
        logging.error(msg)
        return False
    else:
        logging.info("length of ids is fine")
    if not np.all(np.less_equal(np.abs(times - should_be), time_err)):
        err_msg = ("frequency mismatch!\n"
                   "Should be: "+", ".join(map(str, should_be)))
        logging.error(err_msg)
        return False

    return True


def get_starting_position(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return float(source.attrib["startX"]), float(source.attrib["startY"])

    logging.error("could not get_staring_position for group id %d"%group_id)

def get_N_create(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return int(source.attrib["N_create"])

    logging.error("could not N_create for group id %d"%group_id)

def get_bounding_box(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return float(source.attrib["x_min"]), \
              float(source.attrib["x_max"]), \
              float(source.attrib["y_min"]), \
              float(source.attrib["y_max"])

    logging.error("could not get_bounding_box for group id %d"%group_id)

def get_time_limits(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return float(source.attrib["time_min"]), \
              float(source.attrib["time_max"])

    logging.error("could not get time limits for group id %d"%group_id)


def get_starting_time(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return float(source.attrib["time"])

    logging.error("could not get time for group id %d"%group_id)

def get_frequency(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return int(source.attrib["frequency"])

    logging.error("could not get frequency for group id %d"%group_id)


def get_rate(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return int(source.attrib["rate"])

    logging.error("could not get rate for group id %d"%group_id)


def get_percent(group_id, filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("source"):
        if source.attrib["group_id"] == str(group_id):
            return float(source.attrib["percent"])

    logging.error("could not get percent for group id %d"%group_id)

    
def get_source_file(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    for source in root.iter("agents_sources"):
        f = source.find("file")
        return f.text
    
