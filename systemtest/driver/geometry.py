
from driver.utils import pairwise
from numpy import ndarray
from sympy.geometry import Point, Segment


def get_intersetions_path_segment(path: ndarray, crossing_segment: Segment):
    """
    checks, weather the trajetory path of an agent crosses a line (for example: a door).

    :param path (numpy.ndarray): Path of a single agent
    :param crossing_segment (sympy.geometry.segment): Segment which should be checked against the path
    :return (bool) weather agent crosses line

    """
    for p1, p2 in pairwise(path):
        point1, point2 = Point(p1[2], p1[3]), Point(p2[2], p2[3])
        segment = Segment(point1, point2)
        intersections = segment.intersection(crossing_segment)
        if intersections:
            return {"frame": p1[1], "intersections": intersections}

    return None
