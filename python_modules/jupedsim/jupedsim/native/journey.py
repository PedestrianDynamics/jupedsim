# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

from typing import Optional

import py_jupedsim as py_jps


class JourneyDescription:
    def __init__(self, stage_id: Optional[list[int]] = None):
        if stage_id is None:
            self._obj = py_jps.JourneyDescription()
        else:
            self._obj = py_jps.JourneyDescription(stage_id)

    def append(self, stages: int | list[int]) -> None:
        self._obj.append(stages)
