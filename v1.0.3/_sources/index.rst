========
JuPedSim
========

.. toctree::
   :maxdepth: 1
   :hidden:

   Getting started <notebooks/getting_started>
   API reference <api/jupedsim/index>
   Concepts <concepts/index>
   Notebooks <notebooks/index>
   History <history>
   Disclaimer <disclaimer>


**Useful links**:
`Python Packages <https://pypi.org/project/jupedsim/>`__ |
`Source Repository <https://github.com/PedestrianDynamics/jupedsim>`__ |
`Issues <https://github.com/PedestrianDynamics/jupedsim/issues>`__ |
`Discussions <https://github.com/PedestrianDynamics/jupedsim/discussions>`__

What is JuPedSim
================

*JuPedSim* is a Python package with a C++ core to simulate pedestrian dynamics.
This project originally started in 2010 as a C++ based CLI application. However
looking at how this application was used and abused we decided to rewrite
*JuPedSim* as a Python package.

.. note::
   If you find references to versions < 0.10 you need to be aware that they
   refer to the old CLI application.

If you are interested to learn about the history of this project, see
:ref:`_history`

Where to get it
===============

It is easiest to install directly with pip from
`PyPi.org <https://pypi.org/project/jupedsim/>`_:

.. code::

    pip install jupedsim


For information how to build *JuPedSim* from source, visit our `GitHub repository <https://github.com/PedestrianDynamics/jupedsim/>`_.

Main features
=============
*JuPedSim* offers a Python interface, to set up and conduct pedestrian dynamics simulations.
With *JuPedSim* you can simulate small, simple layouts as bottleneck, but you can also simulate large, complex scenarios.
In these complex scenarios, different crowd management measures can be modeled with built-in modules.

For simulating the pedestrian movement and interactions *JuPedSim* offers two different microscopic models out of the box.
The Collision Free Speed Model, as the name imply a speed model.
And a force model with the Generalized Centrifugal Force Model.

With *JuPedSim* different decisions-making processes in the agents route choice can be modeled.
These routes may consist of multiple steps the agents have to complete.
One step may already be completed when an agent reaches the target, but it may also only be completed when a condition like a waiting is fulfilled.
After each completion one of the built-in deciders can be used to conditionally modify the agents next target.
