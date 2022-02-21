import pathlib
import xml.etree.cElementTree as ET
from dataclasses import dataclass
from typing import Dict, NamedTuple, Optional
from xml.dom.minidom import parse

import jinja2
import sympy.geometry


def parse_waiting_areas(filename: pathlib.Path):
    with filename.open() as file:
        root = parse(file)

    waitingAreas = []

    for _, s_elem in enumerate(root.getElementsByTagName("waiting_area")):
        points = []
        for _, p_elem in enumerate(s_elem.getElementsByTagName("polygon")):
            for v_num, _ in enumerate(p_elem.getElementsByTagName("vertex")):
                x = (
                    p_elem.getElementsByTagName("vertex")[v_num]
                    .attributes["px"]
                    .value
                )
                y = (
                    p_elem.getElementsByTagName("vertex")[v_num]
                    .attributes["py"]
                    .value
                )
                points.append(sympy.geometry.Point(x, y))

        waitingAreas.append(sympy.geometry.Polygon(*points))

    return waitingAreas


def instanciate_tempalte(*, src: pathlib.Path, args: Dict, dest: pathlib.Path):
    tpl = jinja2.Template(src.read_text())
    content = tpl.render(args)
    dest.write_text(content)


@dataclass
class TrafficConstraint:
    door_id: int
    outflow: Optional[int]
    max_agents: Optional[int]


def parse_traffic_constraints(inifile: pathlib.Path):
    tree = ET.parse(inifile)
    root = tree.getroot()
    traffic_constraints = {}
    for tc in root.iter("traffic_constraints"):
        for door in tc.iter("door"):
            traffic_constraints[
                int(door.attrib["trans_id"])
            ] = TrafficConstraint(
                int(door.attrib["trans_id"]),
                int(door.get("outflow", None)),
                int(door.get("max_agents", None)),
            )

    if not traffic_constraints:
        raise RuntimeError("Could not parse traffic constraints")

    return traffic_constraints
