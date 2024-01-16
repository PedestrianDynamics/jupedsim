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

@dataclass(kw_only=True)
class SocialForceModelAgentParameters:
    """
    Parameters required to create an Agent in the Social Force Model.

    Attributes:
        test_value: test value without meaning.
    """

    test_value: float = 2.5

    def as_native(
        self,
    ) -> py_jps.SocialForceModelAgentParameters:
        return py_jps.SocialForceModelAgentParameters(
            test_value
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
