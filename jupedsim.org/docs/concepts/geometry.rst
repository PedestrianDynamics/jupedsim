========
Geometry
========

When setting up a pedestrian dynamics simulation, one of the most fundamental parts, is to define where the agents can move.
*JuPedSim* works with a walkable area, which means that every point inside this region can be reached by the agents and every point outside is not included in the simulation code.
The borders of the walkable area may not be crossed by any agent.

.. figure:: /_static/geometry/complex.svg
    :width: 80%
    :align: center
    :alt: Image of a more complex simulation geometry

    A more complex simulation geometry, with multiple obstacles for an entrance scenario.

Walkable area
=============

The walkable area is built with a :class:`~jupedsim.GeometryBuilder` out of regions.
Its :meth:`~jupedsim.GeometryBuilder.build` creates the :class:`~jupedsim.Geometry` that is passed to :class:`~jupedsim.Simulation`.
Each region is one simple, i.e., non self intersecting polygon covering a non-zero area.
It is added either as a *Shapely* :class:`~shapely.Polygon` or as a list of points describing its boundary.

.. figure:: /_static/geometry/simple-polygon.svg
    :width: 80%
    :align: center
    :alt: A simple polygon representing the walkable area of a simulation

    A simple polygon, which can be used to model the walkable area of a simulation.

Such a region can be added like:

.. code:: python

    import jupedsim as jps
    import shapely

    geo_builder = jps.GeometryBuilder()
    room = geo_builder.add_region(polygon=shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)]), height=0.0)

    # or, equivalently, from the points of the boundary
    room = geo_builder.add_region(boundary=[(0, 0), (10, 0), (10, 10), (0, 10)], height=0.0)

:meth:`~jupedsim.GeometryBuilder.add_region` returns the id of the new region.
Agents and stages are placed in a region by this id, see :doc:`routing`.

A walkable area may consist of several regions, e.g. floors at different heights joined by stairs.
Each of them is added with its own call to :meth:`~jupedsim.GeometryBuilder.add_region`.

Preparing polygons
------------------

*JuPedSim* takes each region as-is. If the walkable area is made of several overlapping polygons,
build their union with *Shapely* first. The union **must** result in exactly one polygon.

.. figure:: /_static/geometry/collection.svg
    :width: 80%
    :align: center
    :alt: Combining multiple polygons, to one polygon which represents the walkable area.

    Combining two overlapping polygons to one, which then can be used to model the walkable area of a simulation.

.. code:: python

    import shapely

    first_polygon = shapely.Polygon([(0, 0), (10, 0), (10, 10), (0, 10)])
    second_polygon = shapely.Polygon([(8, 0), (18, 0), (18, 10), (8, 10)])

    walkable_area = shapely.union_all([first_polygon, second_polygon])

The same applies to a walkable area read from a WKT file.
Such files often hold a ``GEOMETRYCOLLECTION``, which has to be turned into a single polygon:

.. code:: python

    import pathlib

    import shapely

    wkt = shapely.from_wkt(pathlib.Path("walkable_area.wkt").read_text())
    walkable_area = shapely.union_all(shapely.get_parts(wkt))

Obstacles
=========

In some scenarios their may be some obstacles, e.g., gates, barriers, inside the walkable area.
Such obstacles will be avoided by the agents, they will not enter this region, but will move around them.
These unreachable regions can be added as holes to a polygon.

.. figure:: /_static/geometry/obstacles.svg
    :width: 80%
    :align: center
    :alt: A polygon, with holes, which represent unreachable areas of the simulation.

    By excluding areas from a polygon (as holes), these areas are marked as unreachable for the simulation.

For creating walkable areas with holes you can use:

.. code:: python

    import shapely

    walkable_area = shapely.Polygon(
        [(0, 0), (20, 0), (20, 20), (0, 20)],
        # now come the holes/obstacles
        [
            # first hole/obstacle
            [(1, 1), (1, 2), (2, 2), (2, 1)],
            # second hole/obstacle
            [(10, 1), (10, 2), (11, 2), (11, 1)],
        ],
    )
    room = geo_builder.add_region(polygon=walkable_area, height=0.0)

    # or, equivalently, from points
    room = geo_builder.add_region(
        boundary=[(0, 0), (20, 0), (20, 20), (0, 20)],
        holes=[
            [(1, 1), (1, 2), (2, 2), (2, 1)],
            [(10, 1), (10, 2), (11, 2), (11, 1)],
        ],
        height=0.0,
    )

Holes have to lie strictly inside the boundary and must not touch each other, not even in a
single point.
An obstacle that touches the boundary along an edge is instead "subtracted" from the walkable area:

.. code:: python

    import shapely

    walkable_area = shapely.Polygon([(0, 0), (20, 0), (20, 20), (0, 20)])
    obstacle = shapely.Polygon([(0, 1), (0, 2), (2, 2), (2, 1)])

    walkable_area = walkable_area.difference(obstacle)

.. warning::

    Be careful when adding obstacles, that they do not split the walkable area in two parts.

Requirements for regions
------------------------

:meth:`~jupedsim.GeometryBuilder.add_region` rejects a region with a
:class:`~jupedsim.SimulationError` if

* a ring (the boundary or a hole) has fewer than three points or intersects itself,
* a hole does not lie strictly inside the boundary, or two holes overlap or touch,
* two vertices of the region are closer than 2.5 cm to each other, or
* a coordinate or the height is not finite.

Polygons from *Shapely* operations or from WKT files can have vertices closer than 2.5 cm, e.g. the
many vertices of a small circle made with ``buffer``. Use fewer segments, e.g.
``point.buffer(0.2, quad_segs=4)``, or remove such vertices, e.g. with
``shapely.simplify(walkable_area, 0.025)``, and check the result.

Regions at the same height must neither overlap nor touch each other; join them into one polygon
instead. :meth:`~jupedsim.GeometryBuilder.build` reports this, as it can only be checked once all
regions are known. A region cannot be removed from a builder again, so create a new
:class:`~jupedsim.GeometryBuilder` after such an error.

.. note::

    For more information how to create polygons with Shapely have a look at their `documentation <https://shapely.readthedocs.io>`_.


Adding a walkable area to the simulation
========================================

After creating the walkable area, it now needs to be passed to the simulation.
This can be done with:

.. code:: python

    import pathlib

    import jupedsim as jps

    # define walkable area
    geo_builder = jps.GeometryBuilder()
    room = geo_builder.add_region(polygon=..., height=0.0)

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=geo_builder.build(),
        trajectory_writer=jps.Hdf5TrajectoryWriter(
            output_file=pathlib.Path("traj.h5")
        ),
    )
