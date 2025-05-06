===================
Crossing Simulation
===================

In this tutorial you will learn how to configure a basic simulation scenario with *netedit*.
The scenario consists of crossing with interacting cars and pedestrians.
First, you will run a simulation with the default pedestrian model of *SUMO* (`striping model <https://sumo.dlr.de/docs/Simulation/Pedestrians.html#model_striping>`__).
Then you will switch to the *JuPedSim* model and inspect the results.

.. note::

    If you made a mistake press *Ctrl + Z* to undo the last step.
    
    The message window at the bottom of *netedit* informs you about warnings and errors. For this tutorial, the window was hidden. 


Setting up the Network
======================

Open *netedit* and create a new network by clicking on *File > New Network*. 
Now you are in the **Network supermode** and can draw the roads for the crossing. 
By default the *Edge mode* is activated.
To draw two-way roads enable the *Edge opposite direction* on the right side of the toolbar. 
Then you can draw the edges (roads) in the editor. 

.. figure:: /_static/coupling/crossing_simulation/Network_edges1.png
    :width: 100%
    :align: center
    :alt: Edge mode for creating the network

    Netedit with highlighted *Network supermode*, *Edge mode* and *Edge opposite direction*. Four egdes have been drawn that are connected in the center.

Make sure that the egdes are connected by a node in the center so that a crossing can be generated.
To learn more about the *SUMO* road network we refer to `this <https://sumo.dlr.de/docs/Networks/SUMO_Road_Networks.html>`__ website.

As pedestrians are spawned distributed along an edge we define small edges at the end of the roads of interest.
In this way, we reduce the effects of the initial conditions and the pedestrians are already in motion when they enter the network defined above.
To compute junctions press *F5* or click *Processing > Compute Junctions*.

.. figure:: /_static/coupling/crossing_simulation/Network_edges2.png
    :width: 100%
    :align: center
    :alt: Adapted network for spawning and removing agents

    Short edges at the end of the roads are created which will be used for spawning and removing pedestrians (and vehicles).

Now we add sidewalks for the pedestrians to our network. 
For this, we enable the *Select Mode*.
In the selection menu on the left choose *Apply selection* for the specified match attributes. 
The selected edges will be highlighted in blue.

.. figure:: /_static/coupling/crossing_simulation/Network_select.png
    :width: 100%
    :align: center
    :alt: Network with selected edges

    Netedit with highlighted *Select mode* and operations. After applying all edges are highlighted in blue.

Then right click on the selected edges *Lane operations > Add restricted lane > Sidewalk* and confirm the operation as shown here:

.. figure:: /_static/coupling/crossing_simulation/Network_sidewalk.png
    :width: 100%
    :align: center
    :alt: Adding a sidewalk to each edge

    Adding a sidewalk to each edge.

For each road a sidewalk is created. 
Clear the selection by pressing *ESC* and compute the junctions again (press *F5*).
The network should look like this:

.. figure:: /_static/coupling/crossing_simulation/Network_edges_final.png
    :width: 100%
    :align: center
    :alt: Final egdes

    Final definition of edges for this tutorial.

Now we add a traffic light. 
Switch to *Traffic light mode* and select the junction in the middle of the network. 
Then click on *Create* in the left menu.

.. figure:: /_static/coupling/crossing_simulation/Network_traffic_lights1.png
    :width: 100%
    :align: center
    :alt: Traffic light mode

    Netedit with highlighted *Traffic light mode* and operations.

.. figure:: /_static/coupling/crossing_simulation/Network_traffic_lights2.png
    :width: 100%
    :align: center
    :alt: Network with traffic lights

    Network after the creation of the traffic lights.

Now we configure the crossing options for pedestrians. 
Enable the *Crossing mode* and click on the junction. 
Select the pair of roads that should be connected by a crosswalk and press *Enter*. 
Possible candidates are highlighted in dark green. 
Once you have selected a candidate the matching one is highlighted in light green.

.. figure:: /_static/coupling/crossing_simulation/Network_crossing2.png
    :width: 100%
    :align: center
    :alt: Adding crossings to the junction

    Netedit with highlighted *Crossing mode* and information on edges of the junction.

In this example, pedestrians are allowed to cross the junction on all roads.
So you have to repeat this process for the three remaining roads.

You have finished the configuration of the network.
Save the network file by clicking *Save network* in the toolbar on the left. 
In this example, the file is named *crossing.net.xml*.

.. figure:: /_static/coupling/crossing_simulation/Network_final.png
    :width: 100%
    :align: center
    :alt: Final network file

    Save the final network file.

Setting up the Demand
=====================

Next, we configure the flow of cars and pedestrians at the border of the defined crossing. 

Vehicle Flow
^^^^^^^^^^^^

Person Flow
^^^^^^^^^^^





