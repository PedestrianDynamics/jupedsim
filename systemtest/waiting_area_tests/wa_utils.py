#!/usr/bin/env python3
# helper functions for testing waiting areas
from dataclasses import dataclass
from typing import Dict, Set
import numpy as np

from xml.dom.minidom import parse


@dataclass
class WaitingArea:
    id: int = None
    roomID: int = None
    subroomID: int = None
    boundingBox: np.ndarray((4,), int) = None
    minNumPed: int = None
    maxNumPed: int = None
    waitingTime: float = None
    startTime: float = None
    transitionID: int = None
    globalTimer: bool = None
    open: bool = None
    nextGoals: Dict[int, float] = None
    pedInside: Set[int] = None


def parse_waiting_areas(filename):
    root = parse(filename)

    n_wall = 0
    waitingAreas = []

    for s_num, s_elem in enumerate(root.getElementsByTagName('waiting_area')):
        waitingArea = WaitingArea()
        # read id

        # read polygon
        for p_num, p_elem in enumerate(s_elem.getElementsByTagName('polygon')):
            n_wall = n_wall + 1
            n_vertex = len(p_elem.getElementsByTagName('vertex'))
            vertex_array = np.zeros((n_vertex, 2))

            for v_num, v_elem in enumerate(p_elem.getElementsByTagName('vertex')):
                vertex_array[v_num, 0] = p_elem.getElementsByTagName('vertex')[v_num].attributes['px'].value
                vertex_array[v_num, 1] = p_elem.getElementsByTagName('vertex')[v_num].attributes['py'].value

            minX = np.min(vertex_array[:, 0])
            maxX = np.max(vertex_array[:, 0])
            minY = np.min(vertex_array[:, 1])
            maxY = np.max(vertex_array[:, 1])
            boundingBox = [minX, maxX, minY, maxY]
            waitingArea.boundingBox = boundingBox
        waitingAreas.append(waitingArea)

    return waitingAreas
