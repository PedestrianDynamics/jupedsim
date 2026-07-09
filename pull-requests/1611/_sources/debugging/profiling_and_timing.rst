.. _profiling_and_timing:

====================
Profiling and Timing
====================

*JuPedSim* provides two complementary instrumentation systems that work on both
the C++ core and from Python:

* **Timer** — lightweight wall-clock measurement that accumulates elapsed time
  per named region.  Useful for comparing the relative cost of simulation
  subsystems and for writing results to CSV.
* **Profiler** — Perfetto-based tracing that records a full timeline of events
  to a ``.pftrace`` file.  Useful for visual inspection in `Perfetto UI
  <https://ui.perfetto.dev>`_.

Both systems are available through the ``jupedsim`` public API, so no C++
knowledge is required to use them.

-------
Timings
-------

How it works
============

Every :class:`~jupedsim.simulation.Simulation` instance owns a
:class:`~jupedsim.internal.tracing.Timer`.  The timer is backed by the C++
``Timer`` class which uses ``std::chrono::high_resolution_clock``.  Elapsed
time is accumulated in **microseconds** across all ``push`` / ``pop`` calls for
the same name, so multiple intervals are summed together.

Timer log levels
================

Each timer probe carries a *probe log level*.  The simulation's *timer log
level* acts as a filter: only probes with a level **≤** the simulation's
log level are recorded.

+-------+------------+
| Value | Meaning    |
+=======+============+
|   1   | General    |
+-------+------------+
|   2   | Detailed   |
+-------+------------+
|   3   | Debug      |
+-------+------------+

Pass ``timer_log_level`` when creating the simulation:

.. code:: python

    import jupedsim as jps

    sim = jps.Simulation(
        model=jps.ModelType.COLLISION_FREE_SPEED,
        geometry=geometry,
        timer_log_level=3,   # record everything including debug probes
    )

The default is ``1`` (General).

Built-in timer keys
===================

The C++ core automatically times these regions during every call to
:meth:`~jupedsim.simulation.Simulation.iterate`:

* ``"Total Simulation Time"`` — running total since the simulation was created
* ``"Total Iteration"`` — cumulative time spent inside ``iterate()``
* ``"Agent Removal System"``
* ``"Neighborhood Search"``
* ``"Stage System"``
* ``"Strategical Decision System"``
* ``"Tactical Decision System"``
* ``"Operational Decision System"``
* ``"Add Agent"``
* ``"Add Journey"``

Python timer API
================

The timer is accessed via :attr:`simulation.timer`.

Measuring a code block
----------------------

Use :meth:`~jupedsim.internal.tracing.Timer.push_timer` and
:meth:`~jupedsim.internal.tracing.Timer.pop_timer` to bracket any region:

.. code:: python

    sim.timer.push_timer("my_preprocessing")
    # ... work ...
    sim.timer.pop_timer("my_preprocessing")

    elapsed = sim.timer.elapsed_time_us("my_preprocessing")
    print(f"preprocessing took {elapsed / 1000:.2f} ms")

The name is free-form; the same name can be pushed and popped multiple times
and the durations accumulate.

Decorator usage
---------------

Decorate a function so that every call is automatically timed:

.. code:: python

    @sim.timer.timer_event
    def spawn_agents():
        for pos in positions:
            sim.add_agent(...)

    spawn_agents()   # timer key "spawn_agents()" is recorded

Use ``name=`` to override the key:

.. code:: python

    @sim.timer.timer_event(name="agent-init")
    def spawn_agents():
        ...

Context-manager usage
---------------------

Wrap any inline block without defining a function:

.. code:: python

    with sim.timer.timer_event("route-computation"):
        for agent_id in agent_ids:
            routing.compute_waypoints(...)

Per-iteration timing
--------------------

:attr:`sim.timer.iteration_duration_us` returns the
time spent in the **last** call to ``iterate()``, making it easy to display
live throughput:

.. code:: python

    while sim.agent_count() > 0:
        sim.iterate()
        it_ms = sim.timer.iteration_duration_us / 1000
        print(f"iteration {sim.iteration_count():5d}  {it_ms:.2f} ms/it", end="\r")

:attr:`~jupedsim.internal.tracing.Timer.operational_level_duration_us` is the
analogous per-iteration time for the Operational Decision System subsystem
only.

Reading all accumulated durations
----------------------------------

Call :meth:`~jupedsim.internal.tracing.Timer.elapsed_time_us` with a key to
get a single value, or retrieve all keys at once from the underlying C++
object:

.. code:: python

    keys = [
        "Total Simulation Time",
        "Neighborhood Search",
        "Operational Decision System",
    ]
    for key in keys:
        us = sim.timer.elapsed_time_us(key)
        print(f"{key:<35} {us / 1_000_000:.3f} s")

Formatted timing report
-----------------------

Converting the timer to a string prints a human-readable table.
Entries below 0.01 % of total time are suppressed:

.. code:: python

    print(sim.timer)

Example output::

    JuPedSim Timings:
    -----------------------------------------------------
    Total Iteration               |    12.34 s (98.72%)
    Neighborhood Search           |     3.21 s (25.69%)
    Operational Decision System   |     8.10 s (64.82%)
    Strategical Decision System   |     0.02 s ( 0.16%)
    -----------------------------------------------------
    Total Simulation Time         |    12.50 s

Saving timing results to CSV
-----------------------------

.. code:: python

    import pandas as pd

    keys = [
        "Total Simulation Time",
        "Neighborhood Search",
        "Operational Decision System",
    ]

    row = {key: sim.timer.elapsed_time_us(key) for key in keys}
    df = pd.DataFrame([row])
    df.to_csv("timings.csv", index=False)

--------
Tracing
--------

How it works
============

The Profiler records a timeline of named events using
`Perfetto <https://perfetto.dev>`_.  When enabled, events from both the C++
core and from Python code are written continuously to a temporary
``.pftrace`` file on disk.  Calling :func:`~jupedsim.dump_traces` stops the
session and moves the temporary file to the path you specify.

Traces can be inspected visually at `ui.perfetto.dev <https://ui.perfetto.dev>`_
by loading the saved file.

.. note::
   The Profiler is a process-wide singleton.  Enabling it adds overhead to
   every instrumented C++ function.  Leave it disabled for production
   benchmarks.

Python profiler API
===================

All profiler functions are available directly from ``jupedsim``:

.. code:: python

    import jupedsim as jps

Enabling and disabling
----------------------

.. code:: python

    jps.enable_tracing()

    # ... run your simulation ...

    jps.disable_tracing()   # discards the trace without saving

Saving the trace
----------------

:func:`~jupedsim.dump_traces` stops the session, saves the trace, and resets
the profiler so a new session can be started:

.. code:: python

    jps.enable_tracing()

    while sim.agent_count() > 0:
        sim.iterate()

    jps.dump_traces("simulation.pftrace")

Open ``simulation.pftrace`` at `ui.perfetto.dev <https://ui.perfetto.dev>`_
to view the full timeline.

Checking whether tracing is active
------------------------------------

.. code:: python

    if jps.is_tracing_enabled():
        print("tracing is on")

Manual event spans
------------------

Bracket any code region with explicit begin/end calls:

.. code:: python

    jps.start_trace_event("agent-spawning")
    for pos in positions:
        sim.add_agent(...)
    jps.end_trace_event()

Events nest correctly — each ``end_trace_event()`` closes the most recently
opened event on the calling thread.

Decorator usage
---------------

Decorate a function to wrap every call in a trace event:

.. code:: python

    @jps.trace_event
    def spawn_agents():
        for pos in positions:
            sim.add_agent(...)

    spawn_agents()   # recorded as "spawn_agents()" in the timeline

Override the displayed name:

.. code:: python

    @jps.trace_event(name="agent-init")
    def spawn_agents():
        ...

Context-manager usage
---------------------

.. code:: python

    with jps.trace_event("route-computation"):
        for agent_id in agent_ids:
            routing.compute_waypoints(...)

Saving mid-run on interrupt
-----------------------------

Use a ``try / except`` around the simulation loop so traces are always
preserved even when the user presses :kbd:`Ctrl-C`:

.. code:: python

    import sys
    import jupedsim as jps

    jps.enable_tracing()
    try:
        while sim.agent_count() > 0:
            sim.iterate()
    except KeyboardInterrupt:
        print("Interrupted — saving partial trace")
        jps.dump_traces("partial_trace.pftrace")
        sys.exit(1)

    jps.dump_traces("full_trace.pftrace")

----------------------------
Combining Timing and Tracing
----------------------------

Both systems can be active simultaneously.  The timer gives you numbers;
the profiler gives you a visual timeline with the same event boundaries.

.. code:: python

    import jupedsim as jps
    import pandas as pd

    sim = jps.Simulation(
        model=jps.ModelType.COLLISION_FREE_SPEED,
        geometry=geometry,
        timer_log_level=2,
    )

    jps.enable_tracing()

    @sim.timer.timer_event
    @jps.trace_event
    def spawn_agents():
        for pos in initial_positions:
            sim.add_agent(...)

    spawn_agents()

    while sim.agent_count() > 0:
        sim.iterate()

    # save visual trace
    jps.dump_traces("simulation.pftrace")

    # print timing table
    print(sim.timer)

    # write per-subsystem timings to CSV
    keys = [
        "Total Simulation Time",
        "Neighborhood Search",
        "Operational Decision System",
        "Strategical Decision System",
        "Tactical Decision System",
    ]
    pd.DataFrame([{k: sim.timer.elapsed_time_us(k) for k in keys}]).to_csv(
        "timings.csv", index=False
    )