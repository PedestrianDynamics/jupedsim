=======================
Custom Serialization
=======================

*JuPedSim* provides a built-in SQLite trajectory writer, but it is also
possible to write your own serializer to output trajectory data in any format you need
(CSV, JSON, HDF5, a remote API, etc.).

This guide explains the serialization interface and shows how to implement a
custom writer.


How serialization works
========================

When you create a :class:`~jupedsim.simulation.Simulation`, you can pass a
``trajectory_writer`` argument. If provided, the simulation will call the
writer automatically during :meth:`~jupedsim.simulation.Simulation.iterate`:

1. **Before the first iteration** — ``begin_writing(simulation)`` is called once.
   Use this to write metadata (frame rate, geometry, column headers, etc.).

2. **Every iteration** — ``write_iteration_state(simulation)`` is called.
   The writer decides internally whether to actually write this frame based on
   the ``every_nth_frame`` interval.

You never need to call these methods yourself — the simulation handles it.

.. important::

    If your writer buffers data, you must manually flush or close it after the
    simulation loop ends. See :ref:`resource-cleanup` below.


The TrajectoryWriter interface
================================

To create a custom writer, subclass
:class:`~jupedsim.serialization.TrajectoryWriter` and implement three methods:

.. code-block:: python

    from jupedsim.serialization import TrajectoryWriter
    from jupedsim.simulation import Simulation


    class MyCustomWriter(TrajectoryWriter):
        def begin_writing(self, simulation: Simulation) -> None:
            """Called once before the first iteration.

            Use this to write headers, metadata, or set up output files.
            """
            ...

        def write_iteration_state(self, simulation: Simulation) -> None:
            """Called every iteration.

            Check the iteration count against every_nth_frame() to decide
            whether to actually write data.
            """
            ...

        def every_nth_frame(self) -> int:
            """Return the write interval.

            1 = write every frame, 5 = every 5th frame, etc.
            """
            ...


Data available from the Simulation
=====================================

Inside ``begin_writing`` and ``write_iteration_state``, the ``simulation``
argument gives you access to all required data:

.. list-table::
    :widths: 35 65
    :header-rows: 1

    * - Method / Property
      - Description

    * - ``simulation.agents()``
      - Iterator over all :class:`~jupedsim.agent.Agent` objects

    * - ``simulation.agent_count()``
      - Number of agents currently in the simulation

    * - ``simulation.iteration_count()``
      - Current iteration number (starts at 0)

    * - ``simulation.elapsed_time()``
      - Elapsed simulation time in seconds

    * - ``simulation.delta_time()``
      - Time step length in seconds

    * - ``simulation.get_geometry()``
      - The simulation geometry (supports ``.as_wkt()``)

Each :class:`~jupedsim.agent.Agent` exposes:

.. list-table::
    :widths: 25 75
    :header-rows: 1

    * - Property
      - Description

    * - ``agent.id``
      - Unique numeric ID

    * - ``agent.position``
      - ``(x, y)`` tuple of the agent's position

    * - ``agent.orientation``
      - ``(x, y)`` tuple of the agent's orientation vector

    * - ``agent.model``
      - Model-specific state (speed, radius, etc.)


Example: CSV writer
=====================

Here is a complete example that writes trajectory data to a CSV file:

.. code-block:: python

    import csv
    from pathlib import Path

    from jupedsim.serialization import TrajectoryWriter
    from jupedsim.simulation import Simulation


    class CsvTrajectoryWriter(TrajectoryWriter):
        """Write trajectory data to a CSV file."""

        def __init__(self, output_file: Path, every_nth_frame: int = 1) -> None:
            if every_nth_frame < 1:
                raise TrajectoryWriter.Exception("'every_nth_frame' must be > 0")

            self._output_file = output_file
            self._every_nth_frame = every_nth_frame
            self._file = None
            self._csv_writer = None

        def begin_writing(self, simulation: Simulation) -> None:
            self._file = open(self._output_file, "w", newline="")
            self._csv_writer = csv.writer(self._file)
            self._csv_writer.writerow(
                ["frame", "id", "pos_x", "pos_y", "ori_x", "ori_y"]
            )

        def write_iteration_state(self, simulation: Simulation) -> None:
            iteration = simulation.iteration_count()
            if iteration % self._every_nth_frame != 0:
                return

            frame = iteration // self._every_nth_frame
            for agent in simulation.agents():
                self._csv_writer.writerow([
                    frame,
                    agent.id,
                    agent.position[0],
                    agent.position[1],
                    agent.orientation[0],
                    agent.orientation[1],
                ])

        def every_nth_frame(self) -> int:
            return self._every_nth_frame

        def close(self) -> None:
            """Flush and close the CSV file."""
            if self._file:
                self._file.close()
                self._file = None


Using the custom writer is identical to using the built-in SQLite writer:

.. code-block:: python

    import jupedsim as jps

    simulation = jps.Simulation(
        model=jps.CollisionFreeSpeedModel(),
        geometry=my_geometry,
        trajectory_writer=CsvTrajectoryWriter(
            output_file=Path("output.csv"),
            every_nth_frame=5,
        ),
    )

    # Run the simulation — writing happens automatically
    while simulation.agent_count() > 0:
        simulation.iterate()

    # Flush remaining data
    simulation._writer.close()


.. note::

    Currently, ``close()`` is not part of the ``TrajectoryWriter`` interface
    and must be called manually via ``simulation._writer``. If your custom
    writer manages resources (file handles, network connections, etc.),
    make sure to call ``close()`` when the simulation is done.


.. _resource-cleanup:

Tips for custom writers
========================

- **Frame skipping**: Always check ``iteration % every_nth_frame`` in
  ``write_iteration_state`` — the simulation calls this method every iteration
  regardless of your interval setting.

- **Buffering**: For performance, consider buffering writes in memory and
  flushing periodically, as the built-in SQLite writer does.

- **Resource cleanup**: Implement a ``close()`` method if your writer opens
  files, connections, or other resources. Call it after the simulation loop
  completes.

- **Error handling**: Raise ``TrajectoryWriter.Exception`` for writer-specific
  errors to keep error reporting consistent.


.. _hdf5-writer:

Built-in HDF5 writer
======================

JuPedSim provides a built-in HDF5 trajectory writer at
:class:`jupedsim.hdf5_serialization.Hdf5TrajectoryWriter`. ``h5py`` is an
optional dependency; install it with ``pip install h5py`` to use this
writer. The schema is aligned with the
`Pedestrian Dynamics Data Archive (PDA) <https://ped.fz-juelich.de/da/doku.php?id=info>`_
HDF5 format used by the
`PedPy <https://github.com/PedestrianDynamics/PedPy>`_ analysis library,
so simulator output and experimental recordings can be analysed with the
same tools.

Usage::

    import jupedsim as jps
    from pathlib import Path

    writer = jps.Hdf5TrajectoryWriter(
        output_file=Path("traj.h5"),
        every_nth_frame=4,
        compression_level=1,
    )
    sim = jps.Simulation(
        model=..., geometry=..., trajectory_writer=writer, dt=0.01,
    )
    while sim.agent_count() > 0:
        sim.iterate()
    writer.close()

Reading the output with PedPy::

    from pedpy.io import load_trajectory_from_ped_data_archive_hdf5
    traj = load_trajectory_from_ped_data_archive_hdf5(
        trajectory_file="traj.h5"
    )

What we adopt from the PDA schema
-----------------------------------

The PDA schema is described in `the data archive documentation
<https://ped.fz-juelich.de/da/doku.php?id=info>`_ and accompanied by a
`JSON schema <https://ped.fz-juelich.de/data/experiments/RDM_Information/schema_v2.json>`_
covering experiment metadata. JuPedSim writes a subset that is
sufficient for analysis tooling:

- ``/trajectory`` compound dataset with the columns
  ``frame``, ``id``, ``x``, ``y``, ``z``, plus the JuPedSim-specific
  orientation columns ``ox`` and ``oy``. The four leading columns and
  the dataset name match the PDA convention exactly, so the dataset is
  consumed without modification by
  ``pedpy.io.load_trajectory_from_ped_data_archive_hdf5``.
- ``fps`` attribute on the ``/trajectory`` dataset (the attribute that
  PedPy reads to recover the frame rate).
- Root attribute ``wkt_geometry`` carrying the simulation walkable area
  as WKT (the attribute that
  ``pedpy.io.load_walkable_area_from_ped_data_archive_hdf5`` reads).
- Per-column ``column_units`` and ``column_descriptions`` attributes on
  ``/trajectory`` (JSON-encoded), in the spirit of the PDA self-
  documentation requirement.

What we do not write
-----------------------

The PDA schema is designed for *experimental* recordings, where many
fields describe physical setups that have no simulator counterpart.
JuPedSim deliberately does **not** populate these fields:

- ``participants``, ``persons`` (with ``orcid``, ``affiliations``, ROR
  identifiers) -- there are no human participants in a simulation.
- ``sensors`` (camera/lidar models, brand) -- no physical capture
  devices.
- ``locations`` (institution, postal address) and ``funding`` -- not
  applicable to a model run.
- ``date_modified`` / ``date_published`` and DOI / publication
  metadata -- belong to the eventual archival of a *dataset*, not the
  raw simulator output.

JuPedSim adds a small set of producer-oriented attributes on the file
root in their place: ``producer = "JuPedSim"``, ``producer_version``,
``schema_version``, ``dt``, ``every_nth_frame``, ``created``, and the
final bounding box (``xmin``, ``xmax``, ``ymin``, ``ymax``). For runs
in which the geometry changes during the simulation, the writer
additionally records ``/geometry/wkt``, ``/geometry/hash`` and
``/frame_geometry`` to preserve the per-frame mapping; these are
ignored by the PedPy loader and are intended for tools that need the
full time history.

