:py:mod:`jupedsim.grid`
=======================

.. py:module:: jupedsim.grid


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.grid.Grid




.. py:class:: Grid(box, distance_to_agents)


   Class to save points and check for neighbours within a radius

   box : an Axis Aligned Bounding Box where the Grid will be able to save points
   distance_to_agents : radius in which points are searched for

   .. py:method:: append_point(pt)


   .. py:method:: get_samples()

      returns a copy of the samples saved


   .. py:method:: get_cell_coords(pt)

      Get the coordinates of the cell that pt = (x,y) falls in.
      box is bounding box containing the minimal/maximal x and y values


   .. py:method:: no_neighbours_in_distance(pt)


   .. py:method:: has_neighbour_in_distance(pt, coords)

      "returns true if there is any point in grid with lt or equal the distance `agent radius` to `pt`



