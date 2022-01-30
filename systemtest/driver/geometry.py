from sympy.geometry import Point, Segment
from numpy import ndarray
from itertools import tee

def pairwise(iterable):
    "s -> (s0,s1), (s1,s2), (s2, s3), ..."
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)


def check_traj_path_cross_line(path: ndarray, crossing_segment: Segment):
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
            return True

    return False
