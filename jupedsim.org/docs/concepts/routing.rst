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
These routes are modeled as a network (so called :ref:`Routes`), consisting of:

- targets as nodes (so called :ref:`Stages`) ,
- directed connections between these targets, indicating the next target, as edges (so called :ref:`Transitions`),

The illustration of such a network can be seen below:

.. figure:: /_static/routing/route-planning.svg
    :width: 80%
    :align: center
    :alt: A graphical representation of a directed graph.

    Graphical representation of the underlying routing network of a more complex simulation.
    Each circle represents an intermediate stage, while the arrows the corresponding transitions.

In the following sections, we will explain how to set-up such Routes in *JuPedSim*.

Stages
------

In *JuPedSim*, a stage refers to an interim target that the agent aims to reach.
A stage counts as completed when the agent reaches it; the agent then continues to the next stage.
The various types of stages will be explained below.

.. note::

    All stages added to the simulation need to be inside the walkable area.
    This needs to be uphold when the geometry is changed.

Each stage lies in one region of the walkable area. Its ``region`` is the handle
:meth:`~jupedsim.WalkableSurface.add_region` returned for that region, see :doc:`geometry`.
Stages are owned by the simulation and returned as handles.

Waypoint
^^^^^^^^

The most fundamental kind of stage is a :class:`Waypoint <jupedsim.Waypoint>`. It represents a target the agent needs to reach.
For the way finding, the agent always targets the Waypoint directly.
A Waypoint is considered completed once agents arrive within a designated distance from the defined Waypoint.
The figure below shows how a Waypoint is used in the simulation. Here the center (colored orange) represents the Waypoint, and the surrounding blue area indicates the zone an agent must enter to complete the stage.

.. figure:: /_static/stages/waypoint.svg
    :width: 20%
    :align: center
    :alt: A dot representing the Waypoint, with an circle around it, depicting the area an agent needs to enter to complete the stage.

    Representation of a :class:`Waypoint <jupedsim.Waypoint>` (colored orange) with the given distance (orange line).

A Waypoint at :math:`(0.6, -1.4)` with an accepted distance of 2m can be added to the simulation via:

.. code:: python

    waypoint = simulation.add_waypoint(jps.circle_to_polygon(center=(0.6, -1.4), radius=2), region=region)

Exit
^^^^

An other stage type is the :class:`Exit <jupedsim.Exit>`. As the name suggests, this stage models the exits within the simulation.
The :class:`~jupedsim.Exit` designates an area where agents are marked for removal from the simulation upon their arrival.
The removal itself is done at the beginning of the next iteration step.
As a target the agents will aim for the center of the exit polygon.

.. note::

    Agents will be removed from the simulation when they reach the designated exit area.

.. figure:: /_static/stages/exit.svg
    :width: 80%
    :align: center
    :alt: A polygon representing the exit area is shown in blue, with the center highlighted in orange.

    Representation of an :class:`Exit <jupedsim.Exit>` (colored blue). The agents will aim for the center (colored orange).

An exit located in the polygon :math:`(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)` can be added to the simulation via:

.. code:: python

    # create exit from coordinates
    exit_stage = simulation.add_exit([(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)], region=region)

    # create exit from shapely.Polygon
    exit_polygon = shapely.Polygon([(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)])
    exit_stage = simulation.add_exit(exit_polygon, region=region)

.. note::

    When adding exits close to the boundary of the walkable area make sure that there is enough room for the agents to enter the exit polygon.
    Otherwise they will not be able to reach the exit at all.

.. warning::

    Currently, pedestrians in the simulation are programmed to target the center of the exit area.
    Therefore, creating wide exits could potentially lead to unpredictable behavior.
    In certain situations, it may be more appropriate to establish multiple exits rather than a single wide one.

Routes
------

Now that we have added the nodes in the network, we need to combine them to a so called Route in *JuPedSim*.
An agent will always continue to move along the transitions of a route after completing its current stage.
A route is built and validated in one call, either as a list of stages that are walked in order, or as a list of ``(from, to)`` pairs that define the transitions:

.. code-block:: python

    # create stages
    ...

    # define a route that walks the stages in order
    route = jps.Route([waypoint, exit_stage])

Transitions
-----------

The pair form of a route defines for each stage which stage an agent will target next after completing its current stage.
These connections are known as Transitions.
*JuPedSim* offers different types of connections to directly model certain decision making processes.

.. note::

    When adding the transitions to your routes make sure, that :ref:`Exits <Exit>` are only added at the end of a trip.
    Otherwise the agents will be removed from the simulation when they reach the exit and will not advance to the next stage.

Fixed transition
^^^^^^^^^^^^^^^^

The most basic form of transition is to define that the agent will proceed on its route with one specific subsequent stage.


.. figure:: /_static/transitions/fixed.svg
    :width: 40%
    :align: center

.. code-block:: python

    route = jps.Route(
        [(start_stage, next_stage), (next_stage, exit_stage)],
        start=start_stage,
    )

Round-robin transition
^^^^^^^^^^^^^^^^^^^^^^

It is also possible to model a decision making process and split the agents at a stage, with a round-robin transition.
Here, the agents proceed by smooth weighted round-robin.
Each decision adds every stage's weight to its running total, picks the stage with the largest total (ties go to the earlier stage) and takes the sum of all weights off it.
E.g., with two outgoing stages and the weights 3 and 2, the agents making a choice continue with the first, second, first, second and first stage.
After this, the cycle restarts with the first stage.

Only the ratio of the weights matters: 60 and 40 give the same sequence as 3 and 2.
With the weights 10, 5, 1 of the example below, the three stages are interleaved over a cycle of 16 agents rather than served in blocks.

.. figure:: /_static/transitions/round_robin.svg
    :width: 40%
    :align: center

To create a round-robin transition as described above, you can follow these steps:

.. code-block:: python

    route = jps.Route(
        [
            (
                start_stage,
                jps.RoundRobin(
                    {
                        first_candidate_stage: 10,
                        second_candidate_stage: 5,
                        third_candidate_stage: 1,
                    }
                ),
            ),
            (first_candidate_stage, exit_stage),
            (second_candidate_stage, exit_stage),
            (third_candidate_stage, exit_stage),
        ],
        start=start_stage,
    )

Least-targeted transition
^^^^^^^^^^^^^^^^^^^^^^^^^

An other option to model decision making processes, is the least-targeted transition.
The next stage is determined from a list of candidates by checking which of the candidates has the least number of agents targeting it already.
When multiple stages have the same number of agent targeting the first defined will be chosen.

.. figure:: /_static/transitions/least_targeted.svg
    :width: 40%
    :align: center

A least-targeted transition can be added to a route with:

.. code-block:: python

    route = jps.Route(
        [
            (
                start_stage,
                jps.LeastTargeted(
                    [
                        first_candidate_stage,
                        second_candidate_stage,
                        third_candidate_stage,
                    ]
                ),
            ),
            (first_candidate_stage, exit_stage),
            (second_candidate_stage, exit_stage),
            (third_candidate_stage, exit_stage),
        ],
        start=start_stage,
    )

Rules in cycles
^^^^^^^^^^^^^^^

Route validation is structural: it checks that an exit or ``STAY`` can be reached from every stage along the transitions.
It does not check that agents reach it.
Least-travel-time and least-targeted rules are deterministic, so in a cycle they can send agents around it forever.
Two examples:

.. code-block:: python

    # The waypoint itself has zero travel time and wins every decision.
    # Route() warns about a rule that lists its own stage.
    jps.Route([(wp, jps.LeastTravelTime([wp, exit_stage]))], start=wp)

    # A single agent finds both candidates untargeted when it decides; the
    # tie goes to other_wp, which leads back to wp.
    jps.Route(
        [
            (wp, jps.LeastTargeted([other_wp, exit_stage])),
            (other_wp, wp),
        ],
        start=wp,
    )

Direct Steering
---------------

In certain scenarios, e.g., developing a specific behavior model, you may want to bypass the decision-making process modeled by the routes.
For this purpose, *JuPedSim* offers a method called "direct steering", it moves an agent to a specific location in the walkable area on the shortest path (see :ref:`Way finding <Way finding>`).
To use direct steering in your simulation, add the agent with a ``target`` location instead of a route:

.. code:: python

    agent = simulation.add_agent(region.at(0, 0), target=region.at(-10, -10), state=state)

The target of an agent that is already in the simulation is changed with :meth:`~jupedsim.Agent.steer_to`.
This also switches an agent that follows a route to direct steering.
It will move towards the specified location and if the location is reached, it will come to a halt there:

.. code:: python

    agent.steer_to(region.at(-10, -10))

.. note::

    Remember, as the agents do not walk towards an exit when using direct steering, they will not be removed from the simulation.
    If they should be removed from the simulation if they reached a certain position, you have to do it manually:

    .. code:: python

        agent.remove()

    Alternatively, the agent can be switched to some route, see :ref:`Switch route`.

Switch route
------------

In complex scenarios it might be necessary to work with multiple routes to model the movement of the agents.
At some point or at some time agents might need to switch between these routes depending on some conditions.
After the following code, the agent `agent` will continue on the route `new_route` targeting `new_stage`:

.. code::

        agent.follow(new_route, start=new_stage)

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
