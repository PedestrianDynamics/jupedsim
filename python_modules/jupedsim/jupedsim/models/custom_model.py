from typing import Any, TYPE_CHECKING, Tuple
import jupedsim.native as py_jps
import numpy as np
from abc import ABCMeta, abstractmethod

if TYPE_CHECKING:
    from jupedsim.agent import Agent
    from jupedsim.geometry import Geometry
    from jupedsim.neighborhood import NeighborhoodSearch
    from jupedsim.linesegment import LineSegment

# alias - use the correct exposed class name
PythonModel = py_jps.OperationalModel


class CustomModelAgentUpdate:
    """
    A simple data class to hold updates for the custom model.
    """

    def __init__(self, update_object: Any = None):
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
    A simple data class to hold parameters for the custom model.
    """

    def __init__(self, param_object: Any = None):
        """
        Accept either:
        - a mapping/dict-like (has .items())
        - a dataclass or any other class
        """
        if param_object is None:
            param_object = dict()
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
    def __init__(self):
        PythonModel.__init__(self)

    @abstractmethod
    def compute_new_position(
        self,
        dT: float,
        ped: Agent,
        geometry: Geometry,
        neighborhoodsearch: NeighborhoodSearch,
    ) -> CustomModelAgentUpdate:
        pass

    def ComputeNewPosition(
        self,
        dT: float,
        ped: py_jps.Agent,
        geometry: py_jps.Geometry,
        neighborhoodsearch: py_jps.NeighborhoodSearch,
    ) -> py_jps.PythonModelUpdate:
        from jupedsim import Agent, Geometry, NeighborhoodSearch

        p = Agent(ped)
        geom = Geometry(geometry)
        neighbor = NeighborhoodSearch(neighborhoodsearch)
        upd = self.compute_new_position(dT, p, geom, neighbor)
        return upd._obj

    @abstractmethod
    def check_model_constraint(
        self,
        ped: Agent,
        neighborhood_search: NeighborhoodSearch,
        geometry: Geometry,
    ):
        pass

    def CheckModelConstraint(
        self,
        ped: py_jps.Agent,
        neighborhood_search: py_jps.NeighborhoodSearch,
        geometry: py_jps.Geometry,
    ):
        from jupedsim import Agent, Geometry, NeighborhoodSearch

        p = Agent(ped)
        geom = Geometry(geometry)
        neighbor = NeighborhoodSearch(neighborhood_search)
        self.check_model_constraint(p, neighbor, geom)


class PythonSocialForceModel(CustomOperationalModel):
    """
    Implementation of the Helbing Social Force Model as described in:
    "Social Force Model for Pedestrian Dynamics" by Helbing & Molnár (1995)

    The model computes the acceleration of each agent as a superposition of:
    1. Desired force: drives agents toward their goal at desired speed
    2. Social force: repulsive forces between agents
    3. Obstacle force: repulsive forces from obstacles

    Parameters:
        desired_speed: desired walking speed [m/s], default 1.34
        reaction_time: time to adapt to new accelerations [s], default 0.5
        agent_scale: strength of repulsive force between agents [N], default 2000
        obstacle_scale: strength of repulsive force from obstacles [N], default 2000
        force_distance: range of social/obstacle forces [m], default 0.08
        mass: mass of agents [kg], default 80
        body_force: body contact force [kg/s²], default 120000
        friction: friction during contact [kg/m/s], default 240000
    """

    def __init__(
        self,
    ):
        PythonModel.__init__(self)

    def _normalize(self, vector: Tuple[float, float]) -> Tuple[float, float]:
        """Normalize a 2D vector."""
        norm = np.sqrt(vector[0] ** 2 + vector[1] ** 2)
        if norm < 1e-10:
            return (0.0, 0.0)
        return (vector[0] / norm, vector[1] / norm)

    def _distance(
        self, p1: Tuple[float, float], p2: Tuple[float, float]
    ) -> float:
        """Compute Euclidean distance between two points."""
        dx = p1[0] - p2[0]
        dy = p1[1] - p2[1]
        return np.sqrt(dx**2 + dy**2)

    def _desired_force(
        self,
        velocity: Tuple[float, float],
        target_direction: Tuple[float, float],
    ) -> Tuple[float, float]:
        """
        Compute the desired force that drives agents toward their goal.

        F_desired = (v_desired - v_actual) / tau
        where v_desired = v0 * e_target and tau is reaction time
        eq2 in paper
        """
        v_desired_x = self.desired_speed * target_direction[0]
        v_desired_y = self.desired_speed * target_direction[1]

        fx = (v_desired_x - velocity[0]) / self.reaction_time
        fy = (v_desired_y - velocity[1]) / self.reaction_time

        return (fx, fy)

    def _set_parameters(self, params):
        """Set model parameters from CustomModelParameters."""
        self.desired_speed = getattr(params, "desired_speed", 0.8)
        self.reaction_time = getattr(params, "reaction_time", 0.5)
        self.agent_scale = getattr(params, "agent_scale", 2000)
        self.obstacle_scale = getattr(params, "obstacle_scale", 2000)
        self.force_distance = getattr(params, "force_distance", 0.08)
        self.mass = getattr(params, "mass", 80)
        self.body_force = getattr(params, "body_force", 120000)
        self.friction = getattr(params, "friction", 240000)
        self.radius = getattr(
            params, "radius", 0.3
        )  # typical pedestrian radius [m]

    def _social_force(
        self,
        agent_pos: Tuple[float, float],
        agent_velocity: Tuple[float, float],
        other_pos: Tuple[float, float],
        other_velocity: Tuple[float, float],
        agent_radius: float = 0.3,
        other_radius: float = 0.3,
    ) -> Tuple[float, float]:
        """
        Compute repulsive social force between two agents.

        Based on Helbing's model with psychological and body contact forces.
        """
        dx = agent_pos[0] - other_pos[0]
        dy = agent_pos[1] - other_pos[1]
        dist = np.sqrt(dx**2 + dy**2)

        # Minimum distance (sum of radii)
        min_dist = agent_radius + other_radius

        if dist < 1e-3:  # Avoid division by zero
            return (0.0, 0.0)

        # Normal direction (from other to agent)
        n_x = dx / dist
        n_y = dy / dist

        # Tangential direction
        t_x = -n_y
        t_y = n_x

        # Relative velocity
        dvx = agent_velocity[0] - other_velocity[0]
        dvy = agent_velocity[1] - other_velocity[1]
        dv_t = dvx * t_x + dvy * t_y  # tangential component

        # Distance-dependent factor
        exp_factor = np.exp(-(dist - min_dist) / self.force_distance)

        # Normal force (repulsive)
        f_n = self.obstacle_scale * exp_factor

        # Body contact force
        if dist < min_dist:
            f_body = self.body_force * (min_dist - dist)
            f_n = f_n + f_body

        # Friction (tangential)
        f_t = self.friction * exp_factor * dv_t if dist < min_dist else 0

        # Total force components
        fx = (f_n + f_t * t_x) * n_x - f_t * t_x
        fy = (f_n + f_t * t_y) * n_y - f_t * t_y

        return (fx, fy)

    def _obstacle_force(
        self,
        agent: Agent,
        obstacle: LineSegment,
    ) -> Tuple[float, float]:
        """
        Compute repulsive force from an obstacle (line segment).

        Based on Helbing's model with psychological and body contact forces.
        """
        # Get closest point on obstacle to agent
        closest_point = obstacle.closest_point(agent.position)
        dist = self._distance(agent.position, closest_point)

        if dist < 1e-3:  # Avoid division by zero
            return (0.0, 0.0)

        # Normal direction (from obstacle to agent)
        n_x = (agent.position[0] - closest_point[0]) / dist
        n_y = (agent.position[1] - closest_point[1]) / dist

        # Distance-dependent factor
        exp_factor = np.exp(-dist / self.force_distance)

        # Normal force (repulsive)
        f_n = self.obstacle_scale * exp_factor

        # Body contact force
        if dist < self.radius:
            f_body = self.body_force * (self.radius - dist)
            f_n = f_n + f_body

        fx = f_n * n_x
        fy = f_n * n_y

        return (fx, fy)

    def compute_new_position(
        self, dT: float, ped, geometry, neighborhood_search
    ):
        """
        Compute new position using Social Force Model.

        Args:
            dT: time step [s]
            ped: GenericAgent (current agent)
            geometry: CollisionGeometry
            neighborhood_search: NeighborhoodSearch for neighbor queries

        Returns:
            Update object with new position and velocity
        """

        # Wrap agent to access Python interface
        agent = ped
        pos = agent.position

        # Get target direction (normalized)
        target_diff = (agent.target[0] - pos[0], agent.target[1] - pos[1])
        # eq 1 in paper
        target_dir = self._normalize(target_diff)
        # Get current velocity
        model = agent.model
        self._set_parameters(model)
        velocity = getattr(model, "velocity")
        # Initialize acceleration from desired force
        acc_x, acc_y = self._desired_force(velocity, target_dir)

        ## Add social forces from neighboring agents
        neighboring_agents = neighborhood_search.get_neighboring_agents(
            pos, 2.0
        )

        for neighbor in neighboring_agents:
            try:
                # neighbor = Agent(neighbor_id)  # Note: this might need adjustment
                neighbor_pos = neighbor.position
                neighbor_velocity = getattr(
                    neighbor.model, "velocity", (0.0, 0.0)
                )

                fx, fy = self._social_force(
                    pos, velocity, neighbor_pos, neighbor_velocity
                )
                acc_x += fx / self.mass
                acc_y += fy / self.mass
            except:
                print("Neighbor can not be accessed")
                pass  # Skip if neighbor cannot be accessed

        # Add obstacle forces (from geometry)
        for wall in geometry.get_walls_in_distance_to(pos, 5.0):
            fx, fy = self._obstacle_force(agent, wall)
            acc_x += fx / self.mass
            acc_y += fy / self.mass

        # Update velocity: v_new = v_old + a * dt
        new_velocity = (
            velocity[0] + acc_x * dT,
            velocity[1] + acc_y * dT,
        )

        # Update position: x_new = x_old + v_new * dt
        new_position = (
            pos[0] + new_velocity[0] * dT,
            pos[1] + new_velocity[1] * dT,
        )

        # Create update
        update = CustomModelAgentUpdate()
        update.position = new_position
        update.velocity = new_velocity

        return update

    def check_model_constraint(self, ped, neighborhood_search, geometry):
        """Check model constraints (optional)."""
        pass
