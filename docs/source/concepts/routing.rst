========
Routing
========

Routing in pedestrian dynamics simulations refers to the process of determining how individuals navigate through a physical environment, such as a street, building, or public space.
In *JuPedSim* this consists of two fundamental parts:

- :ref:`Route planning <Route planning>`:
    Route planning models the decision-making process of the agents on their route through the simulation.
    On base of the underlying route, the target of the agents is distinguished.
    These may include condition-based decisions, where the way of different agents on the same route may differ.

- :ref:`Way finding <Way finding>`:
    Wayfinding is the process by which agents decide on a route to reach their target.
    This process typically considers factors like the agent's goal (target), the layout of the environment, available pathways, and the agent's knowledge of the surroundings.

For more information how this can be included in a *JuPedSim* simulation, and what the underlying principles are, see the following sections.


Route planning
==============

For modeling complex scenarios in which the agents will move through different specific targets on their way, it is possible to define routes.
These routes are modeled as a network, where each target is a node, and the nodes can be connected, indicating how the agent might continue their route.
The illustration of such a network can be seen below:

.. figure:: /_static/routing/route-planning.svg
    :width: 80%
    :align: center
    :alt: A graphical representation of a directed graph.

    Graphical representation of the underlying routing network of a more complex simulation.
    Each circle represents an intermediate stage, while the arrows the corresponding connections.

In the following sections, we will explain how to set-up such routing networks in *JuPedSim*.

Stages
------

In *JuPedSim*, a stage refers to an interim target that the agent aims to reach.
When a stage is reached, the agent will wait for its completion and then continue to the next stage.
A stage may already count as completed when the stage is reached or when some condition is fulfilled.
The various types of stages will be explained below.

.. note::

    All stages added to the simulation need to be inside the walkable area.
    This needs to be uphold when the geometry is changed.

Waypoint
^^^^^^^^

The most fundamental kind of stage is a :class:`Waypoint <jupedsim.stages.WaypointStage>`. It represents a target the agent needs to reach.
For the way finding, the agent always targets the Waypoint directly.
A Waypoint is considered completed once agents arrive within a designated distance from the defined Waypoint.
The figure below shows how a Waypoint is used in the simulation. Here the center (colored orange) represents the Waypoint, and the surrounding blue area indicates the zone an agent must enter to complete the stage.

.. figure:: /_static/stages/waypoint.svg
    :width: 20%
    :align: center
    :alt: A dot representing the Waypoint, with an circle around it, depicting the area an agent needs to enter to complete the stage.

    Representation of a :class:`Waypoint <jupedsim.stages.WaypointStage>` (colored orange) with the given distance (orange line).

A Waypoint at :math:`(0.6, -1.4)` with an accepted distance of 2m can be added to the simulation via:

.. code:: python

    waypoint_id = simulation.add_waypoint_stage([0.6, -1.4], 2)

Exit
^^^^

An other stage type is the :class:`Exit <jupedsim.stages.ExitStage>`. As the name suggests, this stage models the exits within the simulation.
The `ExitStage` designates an area where agents are marked for removal from the simulation upon their arrival.
The removal itself is done at the beginning of the next iteration step.
As a target the agents will aim for the center of the exit polygon.

.. note::

    Agents will be removed from the simulation when they reach the designated exit area.

.. figure:: /_static/stages/exit.svg
    :width: 80%
    :align: center
    :alt: A polygon representing the exit area is shown in blue, with the center highlighted in orange.

    Representation of an :class:`Exit <jupedsim.stages.ExitStage>` (colored blue). The agents will aim for the center (colored orange).

An exit located in the polygon :math:`(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)` can be added to the simulation via:

.. code:: python

    # create exit from coordinates
    exit_id = simulation.add_exit_stage([(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)])

    # create exit from shapely.Polygon
    exit_polygon = shapely.Polygon([(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)])
    exit_id = simulation.add_exit_stage(exit_polygon)

.. warning::

    Currently, pedestrians in the simulation are programmed to target the center of the exit area.
    Therefore, creating wide exits could potentially lead to unpredictable behavior.
    In certain situations, it may be more appropriate to establish multiple exits rather than a single wide one.

Waiting Queue
^^^^^^^^^^^^^

It is not only possible to steer agents with waypoints, it is also possible to let them wait in queues.
*JuPedSim* offers :class:`Queues <jupedsim.stages.NotifiableQueueStage>` where the agents will wait at predefined positions.
The positions given are arranged according to the sequence in which they are defined, and agents will wait at the first available spot.
When agents leave the queue, the other agents will move up in their waiting positions until they reach the front.
In order to allow agents to leave the queue, a signal must be sent to it, indicating that the first `n` agents in the queue are permitted to exit.

.. note::

    If the number of agents arriving at the queue exceeds the available waiting positions, all additional agents beyond capacity will wait at the last designated waiting position.

.. figure:: /_static/stages/queue.svg
    :width: 80%
    :align: center
    :alt: A series of dots represent the different waiting positions, connected with arrows which show in which direction the agents will move up.

    Representation of the :class:`Queue <jupedsim.stages.NotifiableQueueStage>` (dots).
    The first position is marked orange and the last position light-blue.
    The movement of the agents to move up the queue is indicated by the red arrows.

The following code snippet creates a Queue with five waiting spots at (0, 0), (0, 5), (0, 10), (0, 15), and (0, 20) and adds it to the simulation.
Afterwards, it retrieves the Queue object from the simulation and then releases the first two and then the first agent from the simulation.

.. code:: python

    # add the queue to the simulation
    queue_id = simulation.add_queue_stage(
        [
            (0, 0),
            (0, 5),
            (0, 10),
            (0, 15),
            (0, 20),
    )
    # retrieve queue from the simulation
    queue = simulation.get_stage(queue_id)

    ...

    # notify that the first 2 agents can move to the next stage
    queue.pop(2)

    # notify that the first agent can move to the next stage
    queue.pop(1)


Waiting Set
^^^^^^^^^^^

A second option to introduce a waiting stage to the simulation are :class:`WaitingSets <jupedsim.stages.WaitingSetStage>`.
They consists of multiple waiting positions which will be filled in the order of their definition, when creating the waiting set.
A waiting set can either be active, which means the agents will wait there, or inactive, which will release all waiting agents.
If a waiting set is inactive when an agents targets it, the first defined waiting position will be used as a waypoint.

.. note::

    When the number of agents targeting the waiting set exceeds the available waiting spots, the agents in excess will wait at the position where the last agent entered the waiting area.


.. figure:: /_static/stages/waiting_set.svg
    :width: 80%
    :align: center
    :alt: A waiting set is represented by a number of unevenly distributed circles. Two circles are highlighted.

    Representation of the :class:`WaitingSet <jupedsim.stages.WaitingSetStage>`.
    The first (colored orange) and last defined position (colored blue) are highlighted.

In the following, you can see how to add a waiting set to a simulation and how to activate and deactivate it:

.. code:: python

    # add the waiting set to the simulation
    waiting_set_id = simulation.add_waiting_set_stage(
        [
            (60, 50),
            (59, 50),
            (58, 50),
        ]
    )
    # retrieve waiting set from the simulation
    waiting_set = simulation.get_stage(waiting_set_id)

    ...

    # deactivate the waiting
    waiting_set.state = jupedsim.WaitingSetState.INACTIVE

    ...

    # activate the waiting again
    waiting_set.state = jupedsim.WaitingSetState.ACTIVE

Journeys
--------

For creating more complex routes in *JuPedSim* multiple stages can be combined to a so called Journey.

For example:

.. code-block:: python

    # create stages
    ...

    # define a journey
    journey = jps.JourneyDescription(
        [
            waypoint_id,
            queue_id,
            waiting_set_id,
            exit_id
        ]
    )

Transitions
-----------

We currently have a Journey composed of various stages that are not interconnected.
To establish a comprehensive routing setup, we need to define the connections between these stages, known as Transitions.

Transitions specify which stage an agent will target next after completing its current stage.
*JuPedSim* offers different types of connections to directly model certain decision making processes.

.. note::

    When adding the transitions to your journeys make sure, that :ref:`Exits <Exit>` are only added at the end of a trip.
    Otherwise the agents will be removed from the simulation when they reach the exit and will not advance to the next stage.

Fixed transition
^^^^^^^^^^^^^^^^

The most basic form of transition is to define that the agent will proceed on its journey with one specific subsequent stage.


.. figure:: /_static/transitions/fixed.svg
    :width: 40%
    :align: center

.. code-block:: python


    journey.set_transition_for_stage(
        start_stage_id,
        jps.Transition.create_fixed_transition(next_stage_id)
    )

Round-robin transition
^^^^^^^^^^^^^^^^^^^^^^

It is also possible to model a decision making process and split the agents at a stage, with a round-robin transition.
Here, the agents will proceed in a weighted round-robin manner.
E.g., when defining a round-robin transition with three outgoing stages and the corresponding weights 10, 5, 1, the first 10 agents to make a choice will continue with the first given stage.
The next 4 with the second one, and the next agent will continue with the third stage.

After this, the cycle restarts with the first stage.

.. figure:: /_static/transitions/round_robin.svg
    :width: 40%
    :align: center

To create a round-robin transition as described above, you can follow these steps:

.. code-block:: python

    journey.set_transition_for_stage(
        start_stage_id,
        jps.Transition.create_round_robin_transition(
            [
                (first_candidate_stage_id, 10),
                (second_candidate_stage_id, 5),
                (third_candidate_stage_id, 1),
            ]
        )
    )

Least-targeted transition
^^^^^^^^^^^^^^^^^^^^^^^^^

An other option to model decision making processes, is the least-targeted transition.
The next stage is determined from a list of candidates by checking which of the candidates has the least number of agents targeting it already.
When multiple stages have the same number of agent targeting the first defined will be chosen.

.. figure:: /_static/transitions/least_targeted.svg
    :width: 40%
    :align: center

A least-targeted transition can be added to a journey with:

.. code-block:: python

    journey.set_transition_for_stage(
        start_stage_id,
        jps.Transition.create_least_targeted_transition(
            [
                first_candidate_stage_id,
                second_candidate_stage_id,
                third_candidate_stage_id,
            ]
        )
    )

Direct Steering
---------------

In certain scenarios, e.g., developing a specific behavior model, you may not want to have a fixed set of stages along the path of an agent, but want to be more flexible.
For this purpose, *JuPedSim* offers a method called "direct steering" it moves an agent to specific position in the walkable area on the shortest path avoiding any obstacles (see :ref:`Way finding <Way finding>`).
To use direct steering in your simulation, you need to add a direct steering stage and a journey consisting only of the stage to the simulation:

.. code:: python

    direct_steering_stage = simulation.add_direct_steering_stage()
    direct_steering_journey = jps.JourneyDescription([direct_steering_stage])
    direct_steering_journey_id = simulation.add_journey(direct_steering_journey)

Afterward, an agents or agents can be added, targeting this journey and stage.
The agent with the ID direct_steering_agent_id is using the direct steering journey.
We now can directly set the target of this specific agent.
It will move towards the specified point and if the point is reached, it will come to a halt there:

.. code:: python

    agent = simulation.agent(direct_steering_agent_id)
    agent.target = (-10, -10)

.. note::

    Remember, as the agents do not walk towards an exit when using direct steering, they will not be removed from the simulation.
    If they should be removed from the simulation if they reached a certain position, you have to do it manually:

    .. code:: python

        simulation.mark_agent_for_removal(direct_steering_agent_id)

    Alternatively, the journey of an agent can be switched to some journey with an exit:

    .. code:: python

        simulation.switch_agent_journey(
            direct_steering_agent_id,
            new_journey_id,
            new_stage_id
        )

Way finding
===========

Each agent in the simulation is assigned a specific target destination towards which they will head.
The next crucial step is to determine the route that these agents will take to reach their assigned goals.
This involves calculating the most efficient or desired paths from their current locations to their targets within the simulation environment.

To determine the route, *JuPedSim* triangulates the geometry and computes the distance between two points through the triangulation.
Here, it will compute the distance between the centers of two neighboring triangles.
When multiple paths lead to the target, the shortest one will be preferred.

How the path is distinguished for different target points, you can see in the animation below:

.. image:: /notebooks/demo-data/journey/shortest_path.gif
    :width: 60%
    :align: center
