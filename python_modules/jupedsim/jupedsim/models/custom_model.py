from __future__ import annotations

from abc import abstractmethod
from typing import TYPE_CHECKING, Any

try:
    from typing import override
except ImportError:

    def override(f):  # type: ignore[misc]
        return f

import jupedsim.native as py_jps
from jupedsim.geometry import Geometry
from jupedsim.neighborhood import NeighborhoodSearch

if TYPE_CHECKING:
    from jupedsim.agent import Agent

# alias - use the correct exposed class name
PythonModel = py_jps.OperationalModel


class CustomModelAgentUpdate:
    """
    A simple class that wraps a Python object (dict, dataclass, or any other class) to be used as the return type of compute_new_position.
    All attributes in the underlying object will be used for an update on the CustomModelAgentParameters instance.
    """

    def __init__(self, update_object: Any = None):
        """
        Accept either:
        - a mapping/dict-like (has .items())
        - a dataclass or any other class
        """
        if update_object is None:
            update_object = dict()
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
            return
        if hasattr(self._obj.py_object, "keys"):
            self._obj.py_object[name] = value
        else:
            setattr(self._obj.py_object, name, value)


class CustomModelAgentParameters:
    """
    A simple data class to hold parameters for the custom model that are set on a per agent basis.
    All attributes in the underlying object will be accessible as parameters on the agent's model instance.
    """

    def __init__(self, param_object: Any = None):
        """
        Accept either:
        - a mapping/dict-like (has .items())
        - a dataclass or any other class
        """
        if param_object is None:
            self._obj = py_jps.PythonModelState(dict())
        elif isinstance(param_object, py_jps.PythonModelState):
            self._obj = param_object
        else:
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
            return
        if hasattr(self._obj.py_object, "keys"):
            self._obj.py_object[name] = value
        else:
            setattr(self._obj.py_object, name, value)

    def __str__(self):
        str_repr = f"CustomModelParameters({self._obj.py_object})"
        for key in getattr(self._obj.py_object, "keys", lambda: [])():
            value = self._obj.py_object.get(key)
            str_repr += f"\n  {key}: {value}"
        return str_repr


class CustomOperationalModel(PythonModel):
    """
    Abstract base class for custom operational models implemented in Python.
    To create a custom model, subclass this and implement the compute_new_position and check_model_constraint methods.
    i.e.:

    .. code:: python
        class MyCustomModel(CustomOperationalModel):
            def compute_new_position(self, dT, ped, geometry, neighborhoodsearch):
                # compute new position and return as CustomModelAgentUpdate
                return CustomModelAgentUpdate(...)

            def check_model_constraint(self, ped, neighborhood_search, geometry):
                # check if the model constraints are satisfied for this agent
                return True  # or False if constraints are violated

    """

    def __init__(self):
        PythonModel.__init__(self)

    @abstractmethod
    def compute_new_position(
        self,
        dt: float,
        ped: Agent,
        geometry: Geometry,
        neighborhoodsearch: NeighborhoodSearch,
    ) -> CustomModelAgentUpdate:
        """
        Abstract method to compute the new position and other updates for an agent.
         - dT: time step
         - ped: the agent for which to compute the update
         - geometry: the geometry of the environment for collision queries
         - neighborhoodsearch: for querying neighboring agents
        Return a CustomModelAgentUpdate with the desired updates (e.g. position, velocity, etc.)
        """
        pass

    @override
    def ComputeNewPosition(
        self,
        dt: float,
        ped: py_jps.Agent,
        geometry: py_jps.Geometry,
        neighborhoodsearch: py_jps.NeighborhoodSearch,
    ) -> py_jps.PythonModelUpdate:
        """
        Wrapper for ComputeNewPosition to convert to PythonModelUpdate.
        This method is called by the simulation and should not be overridden.
        Instead, override compute_new_position which uses the more convenient Python wrappers.
        """
        # imported here to avoid circular imports, since Agent also imports CustomOperationalModel
        from jupedsim.agent import Agent

        p = Agent(ped)
        geom = Geometry(geometry)
        neighbor = NeighborhoodSearch(neighborhoodsearch)
        upd = self.compute_new_position(dt, p, geom, neighbor)

        return py_jps.CustomModelUpdate(upd._obj)

    @abstractmethod
    def check_model_constraint(
        self,
        ped: Agent,
        neighborhood_search: NeighborhoodSearch,
        geometry: Geometry,
    ):
        """
        Abstract method to check if the model constraints are satisfied for this agent.
         - ped: the agent for which to check constraints
         - neighborhood_search: for querying neighboring agents
         - geometry: the geometry of the environment for collision queries
         Return True if constraints are satisfied, False if violated.
        """
        pass

    @override
    def CheckModelConstraint(
        self,
        ped: py_jps.Agent,
        neighborhood_search: py_jps.NeighborhoodSearch,
        geometry: py_jps.Geometry,
    ):
        """
        Wrapper for CheckModelConstraint to convert to wrap binding classes to pure Python classes.
        This method is called by the simulation and should not be overridden.
        Instead, override check_model_constraint which uses the more convenient Python wrappers.
        """
        # imported here to avoid circular imports, since Agent also imports CustomOperationalModel
        from jupedsim.agent import Agent

        p = Agent(ped)
        geom = Geometry(geometry)
        neighbor = NeighborhoodSearch(neighborhood_search)
        self.check_model_constraint(p, neighbor, geom)
