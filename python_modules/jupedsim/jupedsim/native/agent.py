# Copyright © 2012-2023 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

try:
    import py_jupedsim as py_jps
except ImportError:
    from .. import py_jupedsim as py_jps


from jupedsim.native.models import (
    GeneralizedCentrifugalForceModelState,
    VelocityModelState,
)


class Agent:
    def __init__(self, backing):
        self._obj = backing

    @property
    def id(self):
        return self._obj.id

    @property
    def journey_id(self):
        return self._obj.journey_id

    @journey_id.setter
    def journey_id(self, id):
        self._obj.journey_id = id

    @property
    def stage_id(self):
        return self._obj.stage_id

    @stage_id.setter
    def stage_id(self, id):
        self._obj.stage_id = id

    @property
    def position(self):
        return self._obj.position

    @position.setter
    def position(self, position: tuple[float, float]):
        self._obj.position = position

    @property
    def orientation(self):
        return self._obj.orientation

    @orientation.setter
    def orientation(self, orientation: tuple[float, float]):
        self._obj.orientation = orientation

    @property
    def model(self):
        model = self._obj.model
        if isinstance(model, py_jps.GeneralizedCentrifugalForceModelState):
            return GeneralizedCentrifugalForceModelState(model)
        elif isinstance(model, py_jps.VelocityModelState):
            return VelocityModelState(model)
        else:
            raise Exception("Internal error")
