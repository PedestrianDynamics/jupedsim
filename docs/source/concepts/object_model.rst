=========================
Object model & lifecycle
=========================

A *JuPedSim* simulation is assembled from a small set of objects. Understanding
how they fit together makes the :doc:`Fundamentals notebooks
<../notebooks/fundamentals/index>` easy to follow.

Lifecycle
=========

::

    Geometry ─┐
              ├─▶ Simulation ─▶ Stages ─▶ Journey ─▶ RoutingEngine ─▶ Agents
    Model  ───┘                                                        │
                                                                       ▼
                                                            iterate()  ──▶  Trajectory (sqlite)

The objects
===========

Geometry
    The walkable area, defined as a ``shapely`` polygon; obstacles are polygon
    holes. See :doc:`../notebooks/fundamentals/01_geometry`.

Model
    The operational model that moves agents each step (e.g. the Collision-Free
    Speed Model). Chosen when constructing the ``Simulation``. See
    :doc:`../notebooks/fundamentals/02_models`.

Simulation
    Ties geometry, model, and a trajectory writer together and advances the
    world via ``iterate()``. See :doc:`../notebooks/fundamentals/00_getting_started`.

Stages
    Destinations an agent heads to: ``ExitStage``, ``WaypointStage``,
    ``NotifiableQueueStage``, ``WaitingSetStage``. See
    :doc:`../notebooks/fundamentals/03_exits` and the following notebooks.

Journey & Transition
    A ``JourneyDescription`` wires stages into a route; a ``Transition`` decides
    how an agent leaves a stage. See
    :doc:`../notebooks/fundamentals/07_journeys_transitions`.

RoutingEngine
    Computes paths through the geometry and supports runtime re-routing via
    ``switch_agent_journey``. See :doc:`../notebooks/fundamentals/08_routing`.

Agents
    The pedestrians, placed with ``jupedsim.distributions`` and advanced by the
    model. See :doc:`../notebooks/fundamentals/09_agents_distributions`.

Trajectory output
    ``SqliteTrajectoryWriter`` records positions each frame; read them back for
    analysis and animation. See
    :doc:`../notebooks/fundamentals/11_output_visualization`.
