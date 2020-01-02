#!/usr/bin/env python3
import os
import sys
import logging
import numpy as np
from dataclasses import dataclass, field
from typing import List
utestdir = os.path.abspath(os.path.dirname(os.path.dirname(sys.path[0])))
from sys import *
sys.path.append(utestdir)
from JPSRunTest import JPSRunTestDriver
from utils import equals, contains, SUCCESS, FAILURE


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

        if s.time:
            # in sources.xml time of sources is equidistant (step=5)
            on_time = equals(d[source_id][0], s.time*(j+1), err=time_err)

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

    msg = ("Got: "+", ".join(map(str, times)))

    logging.info(msg)
    times_first = []
    # 111 222 333 ncreate*percent = 3
    # 1 2 3: ncreate*percent = 1
    for i in range(len(times)):
        sub_array = times[i * s.N_create * s.percent:(i+1) * s.N_create * s.percent]
        if not sub_array:
            continue

        times_first.append(sub_array[0])
        if not np.all(np.equal(np.diff(sub_array), 0)):
            err_msg = ("frequency mismatch. Got subtimes: "+", ".join(map(str, sub_array))
                    )
            logging.error(err_msg)

    if not np.all(np.less_equal(np.abs(np.diff(times_first) - s.frequency), time_err)):
        err_msg = ("frequency mismatch. Got times_first: "+", ".join(map(str, times_first))
                  )
        logging.error(err_msg)
        return False

    return True


def get_data(_trajfile):
    """
    Read data from file
    @param:
    _trajfile: trajectory file(type: str)
    @return
    time_err: defined as 3/fps
    d[ped_id] = [time, x, y] (type: dic)
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
        time = frame/fps

        if pid not in _d:
            _d[pid] = [time, x, y]

    return _time_err, _d

def runtest(inifile, trajfile):
    pos_err = 0.15
    time_err, d = get_data(trajfile)

    # start testing sources
    #----------------
    source_ids = range(1, 5)
    source = Source(ids = source_ids, startX = 4, startY = 4, time = 5)
    success = test_source(d, source, time_err, pos_err)
    if not success:
        exit(FAILURE)
    #----------------
    source_ids = range(5, 9)
    source = Source(ids = source_ids, time = 5,
                    xmin = 0, xmax = 1,
                    ymin = 0, ymax =1,
                    )
    success = test_source(d, source, time_err, pos_err)
    if not success:
        exit(FAILURE)
    #----------------        
    source_ids = [9, 10, 11]
    source = Source(ids = source_ids,
                    agents_max = 3,
                    xmin = 3, xmax = 5,
                    ymin = 2, ymax = 4,
                    time_min = 10, time_max = 15)
    success = test_source(d, source, time_err, pos_err)
    if not success:
        exit(FAILURE)
    #----------------
    source_ids = range(12, 12+5)
    source = Source(ids = source_ids,
                    agents_max = 5,
                    xmin = 4, xmax = 6.2,
                    ymin = 3, ymax = 3.5,
                    time_min = 15, time_max = 35,
                    frequency = 5)
    success = test_source(d, source, time_err, pos_err)
    if not success:
        exit(FAILURE)
    #----------------
    source_ids = range(17, 24)
    source = Source(ids = source_ids,
                    N_create = 1,
                    xmin = 4, xmax = 6.2,                   
                    ymin = 3, ymax = 6.1,
                    time_min = 10, time_max = 50,
                    frequency = 6)
    success = test_source(d, source, time_err, pos_err)
    if not success:
        exit(FAILURE) 
    #----------------
    source_ids = range(29, 59)
    source = Source(ids = source_ids,
                    N_create = 10,
                    rate = 4,
                    percent = 1,
                    xmin = 1, xmax = 6.2,                   
                    ymin = 1, ymax = 6.1,
                    time_min = 2, time_max = 30,
                    frequency = 10)
    success = test_source(d, source, time_err, pos_err)
    if not success:
        exit(FAILURE) 
    #----------------

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=sys.path[0], utestdir=utestdir)
    test.run_test(testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
