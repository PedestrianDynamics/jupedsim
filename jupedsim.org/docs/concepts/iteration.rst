.. SPDX-License-Identifier: LGPL-3.0-or-later

=================
Iteration Process
=================

Each call to :meth:`~jupedsim.Simulation.iterate` advances the simulation by one or more discrete time steps of duration ``dt``. Internally, *JuPedSim* executes a deterministic six-phase pipeline inside ``Simulation::iterate()`` so that agent removals, stage transitions, pathfinding, and collision avoidance see a consistent snapshot of the world.

.. figure:: /_static/iteration_process.svg
    :width: 90%
    :align: center
    :alt: Flowchart of the internal JuPedSim simulation iteration pipeline.

    Execution order of a single simulation iteration, from pre-step agent management and deferred removal through strategical, tactical, and double-buffered operational updates.

Step-by-Step Execution Order
============================

Between Iterations & Initial Frame
----------------------------------

Mutating API calls such as :meth:`~jupedsim.Simulation.add_agent`, :meth:`~jupedsim.Simulation.mark_agent_for_removal`, and :meth:`~jupedsim.Simulation.switch_agent_journey` are only permitted between iterations (never mid-iteration).

* **Adding agents (`add_agent`):** Validates model constraints against current walls and neighbors, seeds the agent's random number stream (only after validation succeeds so rejected placements do not advance the RNG sequence), inserts the agent into ``StageManager`` and ``NeighborhoodSearch``, and immediately runs the **Strategical** and **Tactical** systems on that new agent. As a result, a freshly added agent already has a valid ``final_target`` and ``route_orientation`` before the first :meth:`~jupedsim.Simulation.iterate` call.
* **Frame-0 trajectory recording:** When a :class:`~jupedsim.TrajectoryWriter` is attached and ``iteration_count() == 0``, the Python wrapper writes the initial state before executing step 1.

1. Agent Removal System
-----------------------

At the very beginning of ``Simulation::iterate()``, ``AgentRemovalSystem`` erases all agents scheduled for removal since the previous step—either explicitly via :meth:`~jupedsim.Simulation.mark_agent_for_removal` or because they reached an ``Exit`` stage during the previous iteration—and decrements their ``StageManager`` targeting counts.

* **Why removal runs at the start of the next step rather than immediately inside the Exit check:** Deferring ``Exit`` removal by one boundary ensures that an agent reaching an exit during step :math:`k` is still recorded at its arrival position when frame :math:`k` is written at the end of that step, and disappears cleanly before any neighbor interactions in step :math:`k + 1`.

2. Neighborhood Search Update
-----------------------------

``NeighborhoodSearch::update`` rebuilds the spatial lookup structure over the remaining active agents so that subsequent stage checks and neighbor queries never reference removed agents.

3. Strategical Decision System (Journeys & Stages)
--------------------------------------------------

``StrategicalDecisionSystem`` evaluates each agent's current stage within its :class:`~jupedsim.JourneyDescription`:

* **Waypoint stages:** When an agent enters its target area, the stage's :class:`~jupedsim.Transition` (fixed, round-robin, or least-targeted) selects the next ``stage_id``. ``StageManager::migrate_agent`` updates targeting counts immediately so subsequent agents evaluated in the same step observe accurate occupancy/targeting numbers.
* **Exit stages:** Agents inside the exit polygon are appended to the removal queue for Phase 1 of the *next* iteration.
* **Direct steering stages:** Keep the externally assigned ``final_target`` unchanged.

4. Tactical Decision System (Routing Engine)
--------------------------------------------

``TacticalDecisionSystem`` queries ``RoutingEngine::get_orientation`` from each agent's current ``location`` toward its ``final_target`` (updated in Phase 3) and stores the resulting unit vector in ``agent.route_orientation``.

5. Operational Decision System (Double-Buffered Movement)
---------------------------------------------------------

``OperationalDecisionSystem`` advances all agents synchronously using double buffering:

1. Copies the current agent container into a scratch generation ``_next``.
2. For each agent, constructs an ``AgentStep`` view over the read-only ``current`` generation and calls ``model.compute_next_state(current.state, next.state, step)``. Because every agent reads neighbor positions and states exclusively from ``current``, the operational step is independent of agent iteration order.
3. Moves ``next.location`` along the walkable surface via ``move_on_surface(movement)`` and swaps ``agents`` with ``_next``.
4. Rebuilds ``NeighborhoodSearch`` immediately after the swap so that user queries between iterations (such as :meth:`~jupedsim.Simulation.agents_in_range` or constraint checks inside :meth:`~jupedsim.Simulation.add_agent`) reflect the newly computed positions.

6. Clock Advance & Post-Step Output
-----------------------------------

``SimulationClock::advance()`` increments :meth:`~jupedsim.Simulation.iteration_count` and advances :meth:`~jupedsim.Simulation.elapsed_time` by ``dt``. Finally, if a :class:`~jupedsim.TrajectoryWriter` is configured, ``write_iteration_state`` serializes the completed frame.
