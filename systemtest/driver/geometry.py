from sympy.geometry import Point, Segment
from numpy import ndarray
import itertools

def check_traj_path_cross_line(path: ndarray, crossing_segment: Segment):
    """
    checks, weather the trajetory path of an agent crosses a line (for example: a door).

    :param path (numpy.ndarray): Path of a single agent
    :param crossing_segment (sympy.geometry.segment): Segment which should be checked against the path
    :return (bool) weather agent crosses line

    """
    for p1, p2 in itertools.pairwise(path):
        point1, point2 = Point(p1[2], p1[3]), Point(p2[2], p2[3])
        segment = Segment(point1, point2)
        intersections = segment.intersection(crossing_segment)
        if intersections:
            return True

    return False
