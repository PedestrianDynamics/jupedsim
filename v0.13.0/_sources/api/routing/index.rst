:py:mod:`routing`
=================

.. py:module:: routing


Module Contents
---------------

Classes
~~~~~~~

.. autoapisummary::

   routing.RoutingEngine




.. py:class:: RoutingEngine(geo: jupedsim.native.geometry.Geometry)


   .. py:method:: compute_waypoints(frm: tuple[float, float], to: tuple[float, float]) -> list[tuple[float, float]]


   .. py:method:: is_routable(p: tuple[float, float]) -> bool


   .. py:method:: mesh() -> list[tuple[tuple[float, float], tuple[float, float], tuple[float, float]]]


   .. py:method:: edges_for(vertex_id: int)



