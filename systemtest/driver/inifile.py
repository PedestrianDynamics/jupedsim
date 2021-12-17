import pathlib
import sympy.geometry
import jinja2
from typing import Dict

from xml.dom.minidom import parse


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
