:py:mod:`simulation`
====================

.. py:module:: simulation


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   simulation.Simulation




.. py:class:: Simulation(*, model: jupedsim.native.models.VelocityModelParameters | jupedsim.native.models.GeneralizedCentrifugalForceModelParameters, geometry: jupedsim.native.geometry.Geometry, dt: float = 0.01, trajectory_writer: jupedsim.serialization.TrajectoryWriter | None = None)


   .. py:method:: add_waypoint_stage(position: tuple[float, float], distance) -> int


   .. py:method:: add_queue_stage(positions: list[tuple[float, float]]) -> int


   .. py:method:: add_waiting_set_stage(positions: list[tuple[float, float]]) -> int


   .. py:method:: add_exit_stage(polygon: list[tuple[float, float]]) -> int


   .. py:method:: add_journey(journey: jupedsim.native.journey.JourneyDescription) -> int


   .. py:method:: add_agent(parameters: jupedsim.native.models.GeneralizedCentrifugalForceModelAgentParameters | jupedsim.native.models.VelocityModelAgentParameters) -> int


   .. py:method:: remove_agent(agent_id: int) -> bool


   .. py:method:: removed_agents() -> list[int]


   .. py:method:: iterate(count: int = 1) -> None


   .. py:method:: switch_agent_journey(agent_id: int, journey_id: int, stage_id: int) -> None


   .. py:method:: agent_count() -> int


   .. py:method:: elapsed_time() -> float


   .. py:method:: delta_time() -> float


   .. py:method:: iteration_count() -> int


   .. py:method:: agents()


   .. py:method:: agent(agent_id)


   .. py:method:: agents_in_range(pos: tuple[float, float], distance: float)


   .. py:method:: agents_in_polygon(poly: list[tuple[float, float]])


   .. py:method:: get_stage_proxy(stage_id: int)


   .. py:method:: set_tracing(status: bool) -> None


   .. py:method:: get_last_trace() -> jupedsim.native.tracing.Trace


   .. py:method:: get_geometry() -> jupedsim.native.geometry.Geometry



