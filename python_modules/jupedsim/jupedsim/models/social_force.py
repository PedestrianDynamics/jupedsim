from dataclasses import dataclass

import jupedsim.native as py_jps

@dataclass(kw_only=True)
class SocialForceModel:
    """Parameters for Social Force Model

    All attributes are initialized with reasonably good defaults.

    Attributes:
        test_value: test value without meaning
    """

    test_value: float = 2.5
py_jps
@dataclass(kw_only=True)
class SocialForceModelAgentParameters:
    """
    Parameters required to create an Agent in the Social Force Model.

    Attributes:
        test_value: test value without meaning.
        position: Position of the agent.
        orientation: Orientation of the agent.
        journey_id: Id of the journey the agent follows.
        stage_id: Id of the stage the agent targets.
    """

    test_value: float = 2.5
    position: tuple[float, float] = (0.0, 0.0)
    orientation: tuple[float, float] = (0.0, 0.0)
    journey_id: int = -1
    stage_id: int = -1

    def as_native(
        self,
    ) -> py_jps.SocialForceModelAgentParameters:
        return py_jps.SocialForceModelAgentParameters(
            test_value=self.test_value,
            position=self.position,
            orientation=self.orientation,
            journey_id=self.journey_id,
            stage_id=self.stage_id,
        )
    
class SocialForceModelState:
    def __init__(self, backing) -> None:
        self._obj = backing

    @property
    def test_value(self) -> float:
        """test value of this agent."""
        return self._obj.test_value
    
    @test_value.setter
    def test_value(self, test_value):
        self._obj.test_value = test_value
