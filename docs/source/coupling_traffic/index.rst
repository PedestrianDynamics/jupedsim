=====================================
Coupling with Traffic Simulator SUMO
=====================================

Basic functionalities of *JuPedSim* have been integrated into the traffic simulator `SUMO <https://eclipse.dev/sumo/>`__.
The coupled simulator is referred to as *SUMO-JuPedSim*.

.. note::

  The coupling of *SUMO* and *JuPedSim* is a joint project in close cooperation with the *SUMO* team of the German Aerospace Center (DLR).
  The work was (and is) driven by the research projects `SISAME <https://www.sisame.de>`__ and `CroMa-PRO <https://www.croma-pro.de>`__.

  We would like to thank the *SUMO* team for their collaboration!

Below you find details on the implementation of the coupling and tutorials to try out.
The corresponding **configuration files** for the simulations are available in `this <https://github.com/PedestrianDynamics/SUMO-JuPedSim-Simulations>`__ repository.

.. list-table::
    :widths: 10 90

    * - :doc:`Introduction <introduction>`
      - Basic introduction and technical background of the coupling

    * - :doc:`Installation <installation>`
      - How to install *SUMO-JuPedSim*

    * - :doc:`Crossing Simulation <crossing-simulation>`
      - How to configure and run a crossing simulation including pedestrians and cars

.. toctree::
    :maxdepth: 1
    :hidden:

    Introduction <introduction>
    Installation <installation>
    Crossing Simulation <crossing-simulation>
