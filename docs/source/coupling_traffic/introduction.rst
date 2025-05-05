============
Introduction
============

When analyzing the movement of crowds in urban areas — such as at train stations or during the arrival and departure phases of major events — it is essential to consider traffic-related aspects.
Factors like the frequency and capacity of trains, the positioning of train doors, and the interaction between cars and pedestrians at intersections significantly influence the dynamics of pedestrians.
To investigate such scenarios holistically, including both traffic and pedestrian dynamics, basic functionalities of *JuPedSim* have been integrated into *Eclipse SUMO*.

`Eclipse SUMO <https://eclipse.dev/sumo/>`__ (**S**\ imulation of **U**\ rban **Mo**\ bility) is an open-source, multi-modal microscopic simulator that supports both vehicular and pedestrian traffic models.
However, until recently, it lacked the capability to simulate pedestrians freely in 2D space — this is where *JuPedSim* comes in.
*JuPedSim* fills the gap in *SUMO*’s pedestrian modeling by enabling the simulation of crowd movement in 2D space.
This required coupling two fundamentally different simulation concepts and data structures, which had to be carefully aligned.
The integration utilizes the existing framework and data structures of *SUMO*, fitting *JuPedSim* into its simulation processes and tools.
Not all features of *JuPedSim* are available in *SUMO*.
Based on specific research projects and use cases, a subset of functionalities was selected.
As a result, not all Python functionalities of *JuPedSim* are accessible — only those exposed via the C interface can be used within *SUMO*.
Therefore, *SUMO-JuPedSim* is not intended to serve as a standalone pedestrian simulation tool; for more advanced pedestrian-specific configurations, *JuPedSim* alone remains the preferred choice.


What is it for?
===============

The integration is particularly useful in scenarios where crowd and traffic dynamics interact, such as:

*	Scenarios involving congestion-prone pedestrian facilities that influence urban traffic flow or vice versa.
*	Crowded public spaces with connections to vehicular transport systems, e.g., train stations.
*	Modeling intermodal arrival and departure traffic during major public events.

In summary: The *SUMO-JuPedSim* integration becomes especially relevant when crowd dynamics play a significant role in traffic simulation scenarios.

.. figure:: /_static/coupling/intro/Fig1_crossing.png
    :width: 80%
    :align: center
    :alt: Simulation snapshot for a crossing scenario with pedestrians and cars

    Simulation snapshot for a crossing scenario with multidirectional pedestrian streams that interact with cars.

.. figure:: /_static/coupling/intro/Fig2_event.png
    :width: 80%
    :align: center
    :alt: Simulation snapshot for the arrival process to a major event

    Simulation snapshot for the arrival process to a major event considering different modes of transport (train, car, bus, by foot).


Configuration Possibilities
===========================

Following functionalities for configuring a *JuPedSim* simulation in *SUMO* are available:

*	Precise definition of a 2D **walkable area** that is accessible for *JuPedSim* agents including obstacles (such as barriers, trees and signs) either via the DXF import tool or by drawing in netedit
*	Conversion of pedestrian facilities included in a *SUMO* network to a 2D walkable area
*	Configuration of **model parameters** for the :class:`~jupedsim.models.CollisionFreeSpeedModel`
*	Configuration or temporary adjustment of pedestrian speed in defined areas

.. note::
    Even though it is in theory possible to use other *JuPedSim* models, only the :class:`~jupedsim.models.CollisionFreeSpeedModel` has been tested extensively so far.

*	Configuration of **sources** in which *JuPedSim* agents are spawned
*	Configuration of coupled **transfer** points where agents switch from a vehicle (bus, cars or train) to walking or vice versa
*	Configuration of flexible **routing** (journeys) for agents consisting of intermediate waypoints, distribution waypoints and exits
*	Configuration of **vanishing zones** at exits to model ticket control at venues according to the capacity of an entrance system
*	**Interaction of cars** and pedestrians in an ordered manner (stopping by management measures)



Technical Background
====================

The development of *SUMO* started in 2001 with the idea of a multi-platform modeling approach and test bed for car-following models.
Since then, *SUMO* evolved into a suite for intermodal traffic systems including private motor vehicles (PMV), delivery and freight transport, public transport (PT), railways and pedestrians.

The simulation cycle encompasses an initialization phase, where the network, various traffic demands, and additional configurations for traffic infrastructures (e.g. bus or train stops, pedestrian crossings, traffic light timings, parking facilities, etc.) are loaded.
In the following execution steps (“main loop”), the state respectively the position of all traffic objects (e.g. vehicles, pedestrians and traffic management infrastructure as for example traffic lights) is updated at discrete time intervals based on defined models and rules over the runtime of the *SUMO* simulation.
Finally, if a defined simulation step has been reached or all traffic objects have left the network, the *SUMO* simulation is terminated.

The road network is represented as a directed graph with nodes (junctions) and edges (roads), further detailed by lanes (and sub-lanes depending on the model used) and internal connections for detailed intersection modeling.

.. figure:: /_static/coupling/intro/Fig3_network_description.png
    :width: 80%
    :align: center
    :alt: Overview of SUMO network elements

    Overview of different pedestrian elements composing a SUMO network.

These elements have several properties, e.g. a configurable width that determines the number of “parallel” objects (standard case: for pedestrians only; using sub-lane model: for pedestrians and road traffic) in the traffic flow.
By default, there are no 2D areas where pedestrians can move independently of the road network.
For simulations with *JuPedSim* pedestrian-only infrastructures need to be defined separately or imported from dxf-files; however, *SUMO* offers graphical tooling in the *netedit* program for this.

All traffic elements like vehicles, traffic lights, and detectors are defined by specific data structures that capture their physical properties, behavior, and control logic.
Corresponding OpenStreetMap (OSM) data can be import directly by *SUMO* using the so called *osmWebwizard* python script.
Thus, OSM data for pedestrian facilities can be utilized for *JuPedSim* simulations.

SUMO manages the simulation status in memory and provides multiple, mainly XML-based output formats at different levels of granularity and aggregation.
Also, the output of the *JuPedSim* simulation can be saved in form of a wkt-file for the walkable area and xml-file for agent’s trajectories.

While containing two internal pedestrian models (non-interacting and striping model), *SUMO* (currently) models the pedestrian traffic as a purely lane-based phenomena – which, however, deviates from real world observation to a certain extent.
Therefore, *SUMO* has been significantly improved by coupling with *JuPedSim* to provide a full two-dimensional pedestrian movement.

The *SUMO-JuPedSim* simulation loop starts with *JuPedSim* independently updating pedestrian positions over multiple time steps independent of *SUMO*’s simulation cycle, as the temporal resolution for the pedestrian model is significantly higher.
Once positions are calculated, they are mapped to *SUMO*’s network in the following way: Pedestrians on regular *SUMO* lanes (part of the defined network) are assigned to their corresponding edges, while those outside *SUMO*’s standard network (e.g. inside additionally defined walkable areas) are assigned to the nearest edge for technical consistency.
These edges primarily serve as reference points for waypoints (start, end, or intermediate goals).
After mapping, additional adjustments are made, such as modifying pedestrian speeds or removing agents who reach their destinations or enter predefined *vanishing zones*.

.. figure:: /_static/coupling/intro/Fig4_network.png
    :width: 60%
    :align: center
    :alt: Crossing simulation with *SUMO* striping model

    Simulation snapshot of a simple crossing scenario. In this simulation the pedestrians are modeled by the original striping model as they move in the lane-based *SUMO* network.

.. figure:: /_static/coupling/intro/Fig5_network_walkable.png
    :width: 60%
    :align: center
    :alt: Crossing simulation with *SUMO-JuPedSim*

    Simulation snapshot of a simple crossing scenario with *JuPedSim* agents. The 2D walkable area is highlighted in light blue. The walkable area was automatically generated from the *SUMO* network.

.. figure:: /_static/coupling/intro/Fig6_network_walkable_additional.png
    :width: 60%
    :align: center
    :alt: Crossing simulation with *SUMO-JuPedSim* and an additional walkable area

    Simulation snapshot of a simple crossing scenario with *JuPedSim* agents and an additionally defined walkable area. This area could be used, for example, to model movement in a crowded public space or for exiting/entering a building.


To define a journey for the *JuPedSim* agents in *SUMO*, a walk (or trip) must be configured by connecting the corresponding edges in the *SUMO* network (waypoints in *JuPedSim*).
For dynamic rerouting, the system allows agents to adapt their routes during simulation.
This is particularly useful for scenarios like crowd management, where pedestrians might need to choose between exits (e.g. different exits at platform).
Rerouters — placed on *SUMO* edges — define probabilistic next-waypoint options, enabling flexible decision-making.
While these rerouters must be edge-based, they don’t require integration into *SUMO*’s interconnected road network.

Interactions between cars and pedestrians occur in two contexts.
First, in shared spaces (e.g. roads or parking lots), cars treat pedestrians as slow-moving obstacles and adjust their behavior accordingly, though pedestrians currently do not react to vehicles (a feature planned for future implementation).
Second, at crosswalks or traffic lights, *SUMO*’s “link” concept (which opens or closes paths based on signals) translates into active or inactive waiting positions in *JuPedSim*.
Pedestrians approach these crossings but only halt if the link is closed.
However, at the time of writing, these waiting positions are static and need to be refined together with better modeling of the waiting behavior.


Future Perspective
==================

*SUMO-JuPedSim* is being continuously developed based on new use cases and emerging research questions.
Our planned developments include the conversion of the striping model into the *JuPedSim* model within defined areas to enable efficient large-scale traffic simulation including small-scale *JuPedSim* simulations.
Besides, we are actively working on improving the waiting behavior of pedestrians, which is particularly relevant in scenarios such as waiting at traffic lights or at train stations before boarding.
To support this, an advanced *JuPedSim* model for waiting is being developed and is planned to be integrated into *SUMO*.

With the coupled simulators, we have established an open-source framework for advanced modeling in shared spaces, where interactions between different traffic participants become especially complex.
