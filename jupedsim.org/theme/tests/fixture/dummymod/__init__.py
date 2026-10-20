# SPDX-License-Identifier: LGPL-3.0-or-later
"""Dummy module documented by sphinx-autoapi in the theme fixture."""


class Walker:
    """A pedestrian walking at constant speed.

    Parameters
    ----------
    speed : float
        Desired speed in m/s.
    """

    def __init__(self, speed: float = 1.3):
        self.speed = speed

    def distance(self, duration: float) -> float:
        """Distance covered in a given time.

        Parameters
        ----------
        duration : float
            Walking time in s.

        Returns
        -------
        float
            Distance in m.
        """
        return self.speed * duration


def density(count: int, area: float) -> float:
    """Classic density.

    Parameters
    ----------
    count : int
        Number of agents.
    area : float
        Area in m².

    Returns
    -------
    float
        Density in 1/m².
    """
    return count / area
