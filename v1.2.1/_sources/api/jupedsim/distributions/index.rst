:orphan:

:py:mod:`jupedsim.distributions`
================================

.. py:module:: jupedsim.distributions


Module Contents
---------------

.. py:exception:: AgentNumberError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:exception:: IncorrectParameterError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:exception:: OverlappingCirclesError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:exception:: NegativeValueError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.

   Initialize self.  See help(type(self)) for accurate signature.


.. py:function:: distribute_by_number(*, polygon: shapely.Polygon, number_of_agents: int, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000) -> list[tuple[float, float]]

   Generates specified number of randomized 2D coordinates.

   This function will generate the speficied number of 2D coordinates where
   all coordinates are inside the specified geometry and generated coordinates
   are constraint by distance_to_agents and distance_to_polygon. This function
   may not always be able to generate the requested coordinate because it
   cannot do so without violating the constraints. In this case the function
   will stop after max_iterations and raise an Exception.

   :param polygon: polygon where the agents shall be placed
   :param number_of_agents: number of agents to be distributed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


.. py:function:: distribute_by_density(*, polygon: shapely.Polygon, density: float, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000) -> list[tuple[float, float]]

   Generates randomized 2D coordinates based on a desired agent density per
   square meter.

   This function will generate as many 2D coordinates as required to reach the
   desired density. Essentially this function tries to place area * density
   many agents while adhering to the distance_to_polygon and
   distance_to_agents constraints. This function may not always be able to
   generate the requested coordinate because it cannot do so without violating
   the constraints. In this case the function will stop after max_iterations
   and raise an Exception.

   :param polygon: Area where to generate 2D coordinates in.
   :param density: desired density in agents per square meter
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


.. py:function:: distribute_in_circles_by_number(*, polygon: shapely.Polygon, distance_to_agents: float, distance_to_polygon: float, center_point: tuple[float, float], circle_segment_radii: list[tuple[float, float]], numbers_of_agents: list[int], seed=None, max_iterations=10000) -> list[tuple[float, float]]

   Generates randomized 2D coordinates in a user defined number of rings.

   This function will generate 2D coordinates in the intersection of the
   polygon and the rings specified by the centerpoint and the min/max radii of
   each ring. `number_of_agents` is expected to contain the number of agents
   to be placed for each ring. This function may not always be able to
   generate the requested coordinate because it cannot do so without violating
   the constraints. In this case the function will stop after max_iterations
   and raise an Exception.

   :param polygon: polygon where agents can be placed.
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param center_point: Center point of the rings.
   :param circle_segment_radii: min/max radius per ring, rings may not overlap
   :param number_of_agents: agents to be placed per ring
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`
   :raises OverlappingCirclesError: if rings in circle_segment_radii
       overlapp


.. py:function:: distribute_in_circles_by_density(*, polygon: shapely.Polygon, distance_to_agents: float, distance_to_polygon: float, center_point: tuple[float, float], circle_segment_radii: list[tuple[float, float]], densities: list[float], seed: int | None = None, max_iterations: int = 10000) -> list[tuple[float, float]]

   Generates randomized 2D coordinates in a user defined number of rings
   with defined density.

   This function will generate 2D coordinates in the intersection of the
   polygon and the rings specified by the centerpoint and the min/max radii of
   each ring. The number of positions generated is defined by the desired
   density and available space of each ring. This function may not always by
   able to generate the requested coordinate because it cannot do so without
   violating the constraints. In this case the function will stop after
   max_iterations and raise an Exception.

   :param polygon: polygon where agents can be placed.
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param center_point: Center point of the rings.
   :param circle_segment_radii: min/max radius per ring, rings may not overlap
   :param desnities: density in positionsper square meter for each ring
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to place a
                          random point without constraint violation, default is 10_000

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`
   :raises OverlappingCirclesError: if rings in circle_segment_radii
       overlapp


.. py:function:: distribute_until_filled(*, polygon: shapely.Polygon, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000, k: int = 30) -> list[tuple[float, float]]

   Generates randomized 2D coordinates that fill the specified area.

   This function will generate 2D coordinates in the specified area. The
   number of positions generated depends on the ability to place aditional
   points. This function may not always be able to generate the requested
   coordinate because it cannot do so without violating the constraints. In
   this case the function will stop after max_iterations and raise an
   Exception.

   :param polygon: polygon where agents can be placed.
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000
   :param k: maximum number of attempts to place neighbors to already inserted
             points. A higher value will result in a higher density but will greatly
             increase runtim.

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


.. py:function:: distribute_by_percentage(*, polygon: shapely.Polygon, percent: float, distance_to_agents: float, distance_to_polygon: float, seed: int | None = None, max_iterations: int = 10000, k: int = 30)

   Generates randomized 2D coordinates that fill the specified area to a
   percentage of a possible maximum.

   This function will generate 2D coordinates in the specified area. The
   number of positions generated depends on the ability to place aditional
   points. This function may not always be able to generate the requested
   coordinate because it cannot do so without violating the constraints. In
   this case the function will stop after max_iterations and raise an
   Exception.

   :param polygon: polygon where agents can be placed.
   :param percent: percent value of occupancy to generate. needs to be in
                   the intervall (0, 100]
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents
                               and the polygon edges
   :param seed: Will be used to seed the random number generator.
   :param max_iterations: Up to max_iterations are attempts are made to
                          place a random point without constraint violation, default is 10_000
   :param k: maximum number of attempts to place neighbors to already inserted
             points. A higher value will result in a higher density but will greatly
             increase runtim.

   :returns: 2D coordinates

   :raises AgentNumberError: if not all agents could be placed.
   :raises IncorrectParameterError: if polygon is not of type
       :class:`~shapely.Polygon`


