:py:mod:`jupedsim.routing`
==========================

.. py:module:: jupedsim.routing


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   jupedsim.routing.RoutingEngine




.. py:class:: RoutingEngine(geometry, **kwargs)


   RoutingEngine to compute the shortest paths with navigation meshes.

   .. py:method:: compute_waypoints(frm, to)

      Computes shortest path between specified points.

      :param frm: point from which to find the shortest path
      :type frm: tuple[float, float]
      :param to: point to which to find the shortest path
      :type to: tuple[float, float]

      :returns: List of points (path) from 'frm' to 'to' including from and to.


   .. py:method:: is_routable(p)

      Tests if the supplied point is inside the underlying geometry.

      :returns: If the point is inside the geometry.


   .. py:method:: mesh()

      Access the navigation mesh geometry.

      The navigation mesh is store as a collection of triangles in CCW order.

      :returns: The triangles composing the navigation mesh. Each triangle is encoded as 3-tuple of points.


   .. py:method:: edges_for(vertex_id)



