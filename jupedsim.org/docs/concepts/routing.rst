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
These routes are modeled as a network (see :ref:`Routes`), consisting of:

- targets as nodes (see :ref:`Targets`),
- directed connections between these targets, indicating the next target, as edges (so called :ref:`Transitions`),

The illustration of such a network can be seen below:

.. figure:: /_static/routing/route-planning.svg
    :width: 80%
    :align: center
    :alt: A graphical representation of a directed graph.

    Graphical representation of the underlying routing network of a more complex simulation.
    Each circle represents an intermediate target, while the arrows the corresponding transitions.

In the following sections, we will explain how to set up such routes in *JuPedSim*.

Targets
-------

In *JuPedSim*, a target is an area that the agent aims to reach.
A target counts as reached when the agent stands in it; the agent then continues to the next target.
The various types of targets will be explained below.

.. note::

    All targets added to the simulation need to be inside the walkable area.
    This needs to be uphold when the geometry is changed.

A target consists of one or more areas, each given together with the region of the walkable area it lies in.
The region is the handle :meth:`~jupedsim.GeometryBuilder.add_region` returned for it, see :doc:`geometry`.

Waypoint
^^^^^^^^

The most fundamental kind of target is a :class:`Waypoint <jupedsim.Waypoint>`. It represents an area the agent needs to reach.
A Waypoint is reached once the agent stands in its area.
The figure below shows a circular Waypoint. The blue area around the center (colored orange) is the zone an agent must enter to reach it.

.. figure:: /_static/stages/waypoint.svg
    :width: 20%
    :align: center
    :alt: A dot representing the center of the Waypoint, with a circle around it, depicting the area an agent needs to enter to reach it.

    Representation of a circular :class:`Waypoint <jupedsim.Waypoint>` with its center (colored orange) and its radius (orange line).

A circular Waypoint around :math:`(0.6, -1.4)` with a radius of 2m can be added to the simulation via:

.. code:: python

    waypoint = simulation.add_target(jps.Waypoint([(jps.circle_to_polygon(center=(0.6, -1.4), radius=2), region)]))

Exit
^^^^

An other type of target is the :class:`Exit <jupedsim.Exit>`. As the name suggests, this target models the exits within the simulation.
An `Exit` designates an area where agents are marked for removal from the simulation upon their arrival.
The removal itself is done at the beginning of the next iteration step.
The agents head for the part of the exit area they reach soonest, which need not be its center.

.. note::

    Agents will be removed from the simulation when they reach the designated exit area.

.. figure:: /_static/stages/exit.svg
    :width: 80%
    :align: center
    :alt: A polygon representing the exit area is shown in blue, with the center highlighted in orange.

    Representation of an :class:`Exit <jupedsim.Exit>` (colored blue) with its center (colored orange).

An exit located in the polygon :math:`(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)` can be added to the simulation via:

.. code:: python

    # create exit from coordinates
    exit_ = simulation.add_target(jps.Exit([([(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)], region)]))

    # create exit from shapely.Polygon
    exit_polygon = shapely.Polygon([(-0.2, -1.9), (0.2, -1.9), (0.2, -1.7), (-0.2, -1.7)])
    exit_ = simulation.add_target(jps.Exit([(exit_polygon, region)]))

.. note::

    When adding exits close to the boundary of the walkable area make sure that there is enough room for the agents to enter the exit polygon.
    Otherwise they will not be able to reach the exit at all.

Routes
------

Now that we have added the nodes in the network, we need to combine them to a so called route in *JuPedSim*.
An agent will always continue to move along the transitions of its route after reaching its current target.
A route is created from its transitions:

.. code-block:: python

    # create targets
    ...

    # define a route
    route = simulation.add_route(
        [
            jps.FixedTransition(jps.AGENT_LOCATION, waypoint),
            jps.FixedTransition(waypoint, exit_),
        ]
    )

Transitions
-----------

A route is defined by the connections between its targets, known as Transitions.

Transitions specify which target an agent will head for next after reaching its current one.
The transition from ``jps.AGENT_LOCATION`` decides which target an agent that joins the route heads for first.
*JuPedSim* offers different types of connections to directly model certain decision making processes.

.. note::

    When adding the transitions to your routes make sure, that :ref:`Exits <Exit>` are only added at the end of a trip.
    Otherwise the agents will be removed from the simulation when they reach the exit and will not advance to the next target.

Fixed transition
^^^^^^^^^^^^^^^^

The most basic form of transition is to define that the agent will proceed on its route with one specific subsequent target.


.. figure:: /_static/transitions/fixed.svg
    :width: 40%
    :align: center

.. code-block:: python


    jps.FixedTransition(start, next_target)

Round-robin transition
^^^^^^^^^^^^^^^^^^^^^^

It is also possible to model a decision making process and split the agents at a target, with a round-robin transition.
Here, the agents will proceed in a weighted round-robin manner.
E.g., when defining a round-robin transition with three outgoing targets and the corresponding weights 10, 5, 1, the first 10 agents to make a choice will continue with the first given target.
The next 5 with the second one, and the next agent will continue with the third target.

After this, the cycle restarts with the first target.

.. figure:: /_static/transitions/round_robin.svg
    :width: 40%
    :align: center

To create a round-robin transition as described above, you can follow these steps:

.. code-block:: python

    jps.RoundRobinTransition(
        start,
        {
            first_candidate: 10,
            second_candidate: 5,
            third_candidate: 1,
        },
    )

Least-targeted transition
^^^^^^^^^^^^^^^^^^^^^^^^^

An other option to model decision making processes, is the least-targeted transition.
The next target is determined from a list of candidates by checking which of the candidates has the least number of agents targeting it already.
When multiple targets have the same number of agent targeting the first defined will be chosen.

.. figure:: /_static/transitions/least_targeted.svg
    :width: 40%
    :align: center

A least-targeted transition can be added to a route with:

.. code-block:: python

    jps.LeastTargetedTransition(
        start,
        [
            first_candidate,
            second_candidate,
            third_candidate,
        ],
    )

Direct Steering
---------------

In certain scenarios, e.g., developing a specific behavior model, you may want to bypass the decision-making process modeled by the routes.
For this purpose, *JuPedSim* offers a method called "direct steering", it moves an agent to a specific position in the walkable area on the shortest path (see :ref:`Way finding <Way finding>`).
To use direct steering in your simulation, add the agent with a location to steer to instead of a route:

.. code:: python

    agent = simulation.add_agent(
        region.at(0, 0),
        steer_to=region.at(-10, -10),
    )

The agent will move towards the specified point and if the point is reached, it will come to a halt there.
We can set a new target for this specific agent at any time:

.. code:: python

    agent.reroute(steer_to=region.at(5, -10))

.. note::

    Remember, as the agents do not walk towards an exit when using direct steering, they will not be removed from the simulation.
    If they should be removed from the simulation if they reached a certain position, you have to do it manually:

    .. code:: python

        simulation.mark_agent_for_removal(agent)

    Alternatively, the agent can be put on a route, see :ref:`Switch route`.

Switch route
------------

In complex scenarios it might be necessary to work with multiple routes to model the movement of the agents.
At some point or at some time agents might need to switch between these routes depending on some conditions.
After the following code, the agent will continue on the route `new_route`, heading for `new_target` first:

.. code::

        agent.reroute(route=new_route, first_target=new_target)

Way finding
===========

Each agent in the simulation is assigned a specific target destination towards which they will head.
The next crucial step is to determine the route that these agents will take to reach their assigned goals.
This involves calculating the most efficient or desired paths from their current locations to their targets within the simulation environment.

Agents on a route are led by floor fields.
For every target, *JuPedSim* computes the estimated travel time to it from every place of the walkable area, around obstacles, through the openings between regions and across stairs.
An agent follows decreasing travel times towards its target.

Agents under :ref:`direct steering <Direct Steering>` walk the shortest path to their point instead.
To determine it, *JuPedSim* triangulates the geometry and computes the path through the triangulation.

How the shortest path is distinguished for different target points, you can see in the animation below:

.. image:: /notebooks/demo-data/journey/shortest_path.gif
    :width: 60%
    :align: center
