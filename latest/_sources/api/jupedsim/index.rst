:py:mod:`jupedsim`
==================

.. py:module:: jupedsim


Submodules
----------
.. toctree::
   :titlesonly:
   :maxdepth: 1

   distributions/index.rst
   recording/index.rst
   serialization/index.rst
   sqlite_serialization/index.rst
   util/index.rst


Package Contents
----------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.Recording
   jupedsim.RecordingAgent
   jupedsim.RecordingFrame
   jupedsim.TrajectoryWriter
   jupedsim.SqliteTrajectoryWriter



Functions
~~~~~~~~~

.. autoapisummary::

   jupedsim.distribute_by_density
   jupedsim.distribute_by_number
   jupedsim.distribute_by_percentage
   jupedsim.distribute_in_circles_by_density
   jupedsim.distribute_in_circles_by_number
   jupedsim.distribute_till_full
   jupedsim.geometry_from_coordinates
   jupedsim.geometry_from_shapely
   jupedsim.geometry_from_wkt



Attributes
~~~~~~~~~~

.. autoapisummary::

   jupedsim.__version__
   jupedsim.__commit__
   jupedsim.__compiler__


.. py:exception:: AgentNumberError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:exception:: IncorrectParameterError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:exception:: NegativeValueError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:exception:: OverlappingCirclesError(message)


   Bases: :py:obj:`Exception`

   Common base class for all non-exit exceptions.


.. py:function:: distribute_by_density(*, polygon, density, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000)

   returns points randomly placed inside the polygon with the given density

   :param polygon: shapely polygon in which the agents will be placed
   :param density: Density of agents inside the polygon
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, Default is 10_000
   :return: list of created points


.. py:function:: distribute_by_number(*, polygon, number_of_agents, distance_to_agents, distance_to_polygon, seed=None, max_iterations=10000)

   "returns number_of_agents points randomly placed inside the polygon

   :param polygon: shapely polygon in which the agents will be placed
   :param number_of_agents: number of agents distributed
   :param distance_to_agents: minimal distance between the centers of agents
   :param distance_to_polygon: minimal distance between the center of agents and the polygon edges
   :param seed: define a seed for random generation, Default value is None which corresponds to a random value
   :param max_iterations: no more than max_iterations must find a point inside the polygon, default is 10_000
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


.. py:class:: Recording(db_connection_str: str, uri=False)


   .. py:property:: num_frames
      :type: int


   .. py:property:: fps
      :type: float


   .. py:method:: frame(index: int) -> RecordingFrame


   .. py:method:: geometry() -> shapely.GeometryCollection


   .. py:method:: bounds() -> jupedsim.internal.aabb.AABB



.. py:class:: RecordingAgent


   .. py:attribute:: id
      :type: int

      

   .. py:attribute:: position
      :type: tuple[float, float]

      

   .. py:attribute:: orientation
      :type: tuple[float, float]

      


.. py:class:: RecordingFrame


   .. py:attribute:: index
      :type: int

      

   .. py:attribute:: agents
      :type: list[RecordingAgent]

      


.. py:class:: TrajectoryWriter


   Interface for trajectory serialization

   .. py:method:: begin_writing(simulation) -> None
      :abstractmethod:

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...


   .. py:method:: write_iteration_state(simulation) -> None
      :abstractmethod:

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.


   .. py:method:: end_writing(simulation) -> None
      :abstractmethod:

      End writing trajectory data.

      This method is intended to handle finalizing writing of trajectory
      data, e.g. write closing tags, or footer meta data.


   .. py:method:: every_nth_frame() -> int
      :abstractmethod:

      Returns the intervall of this writer in frames between writes.

      1 indicates all frames are writen, 10 indicates every 10th frame is
      writen and so on.

      Returns:
          Number of frames beween writes as int



.. py:class:: SqliteTrajectoryWriter(*, output_file: pathlib.Path, every_nth_frame: int = 4)


   Bases: :py:obj:`jupedsim.serialization.TrajectoryWriter`

   Write trajectory data into a sqlite db

   .. py:method:: begin_writing(simulation: jupedsim.native.simulation.Simulation) -> None

      Begin writing trajectory data.

      This method is intended to handle all data writing that has to be done
      once before the trajectory data can be written. E.g. Meta information
      such as framerate etc...


   .. py:method:: write_iteration_state(simulation: jupedsim.native.simulation.Simulation) -> None

      Write trajectory data of one simulation iteration.

      This method is intended to handle serialization of the trajectory data
      of a single iteration.


   .. py:method:: end_writing(simulation) -> None

      End writing trajectory data.

      This method is intended to handle finalizing writing of trajectory
      data, e.g. write closing tags, or footer meta data.


   .. py:method:: every_nth_frame() -> int

      Returns the intervall of this writer in frames between writes.

      1 indicates all frames are writen, 10 indicates every 10th frame is
      writen and so on.

      Returns:
          Number of frames beween writes as int


   .. py:method:: connection() -> sqlite3.Connection



.. py:exception:: GeometryError(message)


   Bases: :py:obj:`Exception`

   Class reflecting errors when creating JuPedSim geometry objects.


.. py:function:: geometry_from_coordinates(coordinates: List[Tuple], *, excluded_areas: Optional[List[Tuple]] = None) -> jupedsim.native.geometry.Geometry


.. py:function:: geometry_from_shapely(geometry_input: shapely.Polygon | shapely.MultiPolygon | shapely.GeometryCollection | shapely.MultiPoint) -> jupedsim.native.geometry.Geometry


.. py:function:: geometry_from_wkt(wkt_input: str) -> jupedsim.native.geometry.Geometry





