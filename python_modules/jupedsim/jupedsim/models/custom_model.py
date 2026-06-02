import jupedsim.native as py_jps
import numpy as np

# alias - use the correct exposed class name
PythonModel = py_jps.OperationalModel


class CustomModuleUpdate:
    """
    A simple data class to hold updates for the custom model.
    """

    def __init__(self, dict: dict = None):
        self._obj = py_jps.PythonModelUpdate()
        if dict is not None:
            self.set_dict(dict)

    def set_dict(self, dict):
        for key, value in dict.items():
            self._obj.set(key, value)

    def __getattr__(self, name):
        """Automatically get from the underlying object"""
        if name.startswith("_"):
            return object.__getattribute__(self, name)
        return self._obj.get(name)

    def __setattr__(self, name, value):
        """Automatically set on the underlying object"""
        if name.startswith("_"):
            object.__setattr__(self, name, value)
        else:
            self._obj.set(name, value)


class CustomModelParameters:
    """
    A simple data class to hold parameters for the custom model.
    """

    def __init__(self, dict: dict = None):
        if dict is not None:
            for key, value in dict.items():
                setattr(self, key, value)


class CustomModelState:
    """
    A simple data class to hold state for the custom model.
    """

    def __init__(self, dict: dict = None):
        if dict is not None:
            for key, value in dict.items():
                setattr(self, key, value)


class StraightAheadModel(PythonModel):
    """
    A simple custom model that moves agents straight ahead at a constant speed.
    """

    def __init__(self, speed: float = 1.0):
        PythonModel.__init__(self)
        self.speed = speed

    def ComputeNewPosition(self, dT: float, ped, geometry, neighborhood_search):
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

        for agent in agents:
            print(
                f"Neighboring agent id: {agent.id}, position: {agent.position}"
            )

        direction = (
            ped.target[0] - ped.position[0],
            ped.target[1] - ped.position[1],
        )
        norm = np.linalg.norm(direction)

        new_position = (
            ped.position[0] + self.speed * direction[0] / norm * dT,
            ped.position[1] + self.speed * direction[1] / norm * dT,
        )

        update = py_jps.PythonModelUpdate()
        update.set("position", new_position)
        # update.set("orientation", new_velocity)

        return update

    def ApplyUpdate(self, update, agent):
        """
        Apply the computed update to the agent.

        Args:
            update: OperationalModelUpdate object containing position and velocity changes
            agent: GenericAgent object to update
        """
        pos = update.get("position")
        vel = update.get("velocity")

        if pos is not None:
            agent.position = pos

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
