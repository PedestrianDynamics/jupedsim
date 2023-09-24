:py:mod:`jupedsim.distributions`
================================

.. py:module:: jupedsim.distributions


Module Contents
---------------


Functions
~~~~~~~~~

.. autoapisummary::

   jupedsim.distributions.distribute_by_number
   jupedsim.distributions.distribute_by_density
   jupedsim.distributions.distribute_in_circles_by_number
   jupedsim.distributions.distribute_in_circles_by_density
   jupedsim.distributions.distribute_till_full
   jupedsim.distributions.distribute_by_percentage



.. py:exception:: AgentNumberError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:exception:: IncorrectParameterError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:exception:: OverlappingCirclesError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:exception:: NegativeValueError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:function:: distribute_by_number(*, polygon, number_of_agents, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000)

   "returns number_of_agents points randomly placed inside the polygon

   :param polygon: shapely polygon in which the agents will be placed
   :param number_of_agents: number of agents distributed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
   :return: list of created points


.. py:function:: distribute_by_density(*, polygon, density, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000)

   returns points randomly placed inside the polygon with the given density

   :param polygon: shapely polygon in which the agents will be placed
   :param density: Density of agents inside the polygon
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_in_circles_by_number(*, polygon, distance_to_agents, distance_to_polygon, center_point, circle_segment_radii, numbers_of_agents, seed=None, max_iterations=10000)

   returns points randomly placed inside the polygon inside each the circle segments

   :param polygon: shapely polygon in which the agents will be placed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param center_point: the Center point of the circle segments
   :param circle_segment_radii: a list of minimal and maximal radius for each circle segment
       Circle segments must not overlap
       formatted like [(minimum_radius, maximum_radius)]
   :param numbers_of_agents: a list of number of agents for each Circle segment
       the position of the number corresponds to the order in which the Circle segments are given
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_in_circles_by_density(*, polygon, distance_to_agents, distance_to_polygon, center_point, circle_segment_radii, densities, seed=None, max_iterations=10000)

   returns points randomly placed inside the polygon inside each the circle segments

   :param polygon: shapely polygon in which the agents will be placed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param center_point: the Center point of the circle segments
   :param circle_segment_radii: a list of minimal and maximal radius for each circle segment
       Circle segments must not overlap
       formatted like [(minimum_radius, maximum_radius)]
   :param densities: a list of densities for each Circle segment
       the position of the number corresponds to the order in which the Circle segments are given
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_till_full(*, polygon, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000, k=30)

   returns as many randomly placed points as fit into the polygon.
   Points are distributed using Bridson’s algorithm for Poisson-disc sampling
   The algorithm is explained in Robert Bridson´s Paper "Fast Poisson Disk Sampling in Arbitrary Dimensions"
       :param polygon: shapely polygon in which the agents will be placed
       :param distance_to_agents: minimal distance between the centers of agents
       :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
       :param seed: define a seed for random generation, Default value is None which corresponds to a random value
       :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
       :param k: around each point k point will be created before the point is considered inactive
       :return: list of created points


.. py:function:: distribute_by_percentage(*, polygon, percent, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000, k=30)

   returns points for the desired percentage of agents that fit inside the polygon (max possible number)
   fills the polygon entirely using Bridson’s algorithm for Poisson-disc sampling and then selects the percentage of placed agents


    :param polygon: shapely polygon in which the agents will be placed
    :param percent: percentage of agents selected - 100% ≙ completely filled polygon 0% ≙ 0 placed points
    :param distance_to_agents: minimal distance between the centers of agents
    :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
    :param seed: define a seed for random generation, Default value is None which corresponds to a random value
    :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
    :return: list of created points


