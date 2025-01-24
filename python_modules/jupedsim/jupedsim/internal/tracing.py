// SPDX-License-Identifier: LGPL-3.0-or-later

import jupedsim.native as py_jps


class Trace:
    """
    .. important::

        This is indented for internal usage. We will not guarantee that this API will
        stable and available in any release. It might be changed on any update, regardless of
        a major/minor/patch update.
    """

    def __init__(self) -> None:
        self._obj = py_jps.Trace

    @property
    def iteration_duration(self) -> float:
        """Time for one simulation iteration in us.

        Returns:
             Time for one simulation iteration in us
        """
        return self._obj.iteration_duration

    @property
    def operational_level_duration(self) -> float:
        """Time for one simulation iteration in the operational level in us.

        Returns:
             Time for one simulation iteration in the operational level in us
        """

        return self._obj.operational_level_duration

    def __str__(self) -> str:
        return self._obj.__repr__()
