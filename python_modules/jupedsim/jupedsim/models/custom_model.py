from typing import Any
import jupedsim.native as py_jps
import numpy as np

# alias - use the correct exposed class name
PythonModel = py_jps.OperationalModel


class CustomModelUpdate:
    """
    A simple data class to hold updates for the custom model.
    """
    def __init__(self, update_object: Any = dict()):
        self._obj = py_jps.PythonModelUpdate(update_object)

    def __getattr__(self, name):
        """Automatically get from the underlying object"""
        if name.startswith("_"):
            return object.__getattribute__(self, name)
        if hasattr(self._obj.py_object, "keys"):
            if name in self._obj.py_object.keys():
                return self._obj.py_object[name]
            else:
                raise AttributeError(
                    f"Key '{name}' not found in the underlying object"
                )
        else:
            return getattr(self._obj.py_object, name)

    def __setattr__(self, name, value):
        """Automatically set on the underlying object"""
        if name.startswith("_"):
            object.__setattr__(self, name, value)
        if hasattr(self._obj.py_object, "keys"):
            self._obj.py_object[name] = value
        else:
            setattr(self._obj.py_object, name, value)


class CustomModelParameters:
    """
    A simple data class to hold parameters for the custom model.
    """

    def __init__(self, param_object: Any = None):
        """
        Accept either:
        - a mapping/dict-like (has .items())
        - a dataclass or any other class
        """
        self._obj = py_jps.PythonModelState(param_object)

    def __getattr__(self, name):
        """Automatically get from the underlying object"""
        if name.startswith("_"):
            return object.__getattribute__(self, name)
        if hasattr(self._obj.py_object, "keys"):
            if name in self._obj.py_object.keys():
                return self._obj.py_object[name]
            else:
                raise AttributeError(
                    f"Key '{name}' not found in the underlying object"
                )
        else:
            return getattr(self._obj.py_object, name)

    def __setattr__(self, name, value):
        """Automatically set on the underlying object"""
        if name.startswith("_"):
            object.__setattr__(self, name, value)
        if hasattr(self._obj.py_object, "keys"):
            self._obj.py_object[name] = value
        else:
            setattr(self._obj.py_object, name, value)


class StraightAheadModel(PythonModel):
    """
    A simple custom model that moves agents straight ahead at a constant speed.
    """

    def __init__(self, speed: float = 1.0):
        PythonModel.__init__(self)
        self.speed = speed

    def ComputeNewPosition(self, dT: float, ped, geometry, neighborhood_search):
        from jupedsim.agent import Agent
        """
        Compute the new position for an agent.

        Args:
            dT: Time step
            ped: GenericAgent object
            geometry: CollisionGeometry object
            neighborhood_search: NeighborhoodSearch object

        Returns:
            OperationalModelUpdate object with position and velocity changes
        """
        # Move straight ahead at constant speed

        agents = neighborhood_search.get_neighboring_agents(
            ped.position, 2.0
        )  # Example of using neighborhood search

        #the ped is a GenericAgent, we want to interact with the Python native Agent class, so we wrap it
        ped = Agent(ped)
        model = ped.model

        direction = (
            ped.target[0] - ped.position[0],
            ped.target[1] - ped.position[1],
        )
        norm = np.linalg.norm(direction)

        new_position = (
            ped.position[0] + self.speed * direction[0] / norm * dT,
            ped.position[1] + self.speed * direction[1] / norm * dT,
        )

        update = CustomModelUpdate()
        update.position = new_position
        # update.set("orientation", new_velocity)

        return update._obj

    def CheckModelConstraint(self, ped, neighborhood_search, geometry):
        """
        Check if the agent satisfies model constraints.

        Args:
            agent: GenericAgent object to check
            neighborhood_search: NeighborhoodSearch object
            geometry: CollisionGeometry object

        Raises:
            SimulationError if constraints are violated
        """
        # For this simple model, we don't have additional constraints
        pass
