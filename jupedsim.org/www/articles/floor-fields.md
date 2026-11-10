---
date: "2026-11-10"
author: "JuPedSim team"
description: "How agents in JuPedSim 2 find their way to waypoints and exits with floor fields, what you can tune, how to look at the fields and how to reroute agents around congestion with speed fields."
---

<!-- SPDX-License-Identifier: LGPL-3.0-or-later -->

# Floor fields: how agents find their way in JuPedSim 2

In JuPedSim 2, agents walk to waypoints and exits along *floor fields*. A floor
field is a map of how long it takes to reach one destination from every point of the
walkable area. An agent simply walks down the gradient on that map, so it always heads for
the part of the destination it can reach fastest, around walls and holes, through doors
and up and down stairs.

This article explains what that means for your models, which settings you have,
how to look at the fields, and how to change them, for example to reroute agents
around congestion. It ends with what we chose not to do (yet) and what comes next.

```{note}
Everything here is part of the upcoming JuPedSim 2.0 and available on the
`master` branch today. It is not in the 1.x releases.
```

## What a floor field is

A destination is the area of a waypoint or an exit; in the API it is identified by
the id of its stage (`stage_id`). For every destination, JuPedSim covers each region of
your walkable surface with a grid of square cells and computes, for every cell, the
travel time to the destination. The computation follows a wave that starts in the
destination's area and spreads out at the local walking speed: around obstacles, through
the openings between regions, and across stairs that connect floors. Mathematically
it solves the eikonal equation with the fast sweeping method.

Two consequences are worth knowing:

- **Agents head for the nearest part of a destination, not its centre.** A wide exit
  or a destination made of several areas works as you would expect: every agent takes the part
  that is quickest to reach.
- **Routes keep some distance from walls.** Within `wall_influence_radius` of a wall
  the field assumes slower walking, so the quickest route runs a little away from
  walls and does not cut corners tightly. Doors and the openings between regions do
  not count as walls.

Direct steering is not affected: an agent with a direct steering target
still walks the shortest path to that single point.

## Tuning the grid

Two parameters control the fields. Both apply to the whole simulation:

```python
import jupedsim as jps

simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(),
    geometry=surface,
    floorfield_options=jps.FloorfieldOptions(
        cell_size=0.2,  # m, the default
        wall_influence_radius=0.5,  # m, the default
    ),
)
```

- **`cell_size`** is the edge length of a grid cell. Smaller cells resolve the
  geometry more finely and give smoother routes; they cost memory and time when a
  field is computed (four times as many cells when you halve the size).
- **`wall_influence_radius`** is how far from a wall the field assumes slower
  walking. Set it smaller to let routes run closer to walls.

```{warning}
A passage narrower than two cells may close. A cell that a wall runs through counts
as not walkable, so a gap that holds no whole cell is blocked for the floor field.
If that disconnects part of your geometry, creating the simulation fails with
`SimulationError: Floor field: some walkable cells could not be reached, you can try
reducing the cell size.` Reduce `cell_size` until the narrowest passage is at least
two cells wide.
```

## Looking at the fields

`Simulation.travel_times(stage_id)` returns one grid per region, keyed by region id.
Each grid has its travel times in seconds and the walking direction an agent would
get in every cell, and plots directly with matplotlib:

```python
import matplotlib.pyplot as plt
import numpy as np
import shapely

surface = jps.WalkableSurface()
room = surface.add_region(polygon=shapely.box(0, 0, 20, 10))
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(), geometry=surface
)
north = simulation.add_exit_stage([(shapely.box(19, 8, 20, 10), room)])
south = simulation.add_exit_stage([(shapely.box(19, 0, 20, 2), room)])

grid = simulation.travel_times(north)[room]
plt.imshow(
    np.ma.masked_invalid(grid.values), origin="lower", extent=grid.extent
)
plt.colorbar(label="travel time to the north exit [s]")
```

- `grid.values` has shape `(height, width)`, row 0 is the lowest. It is zero inside
  the destination's area and `inf` where a cell is not walkable.
- `grid.directions` has shape `(height, width, 2)`: the unit vector an agent at the
  cell's centre is sent along, zero inside the destination's area, `nan` where a cell is
  not walkable.
- `grid.origin`, `grid.width`, `grid.height`, `grid.cell_size` and `grid.extent`
  place the grid in the world. The grids of all regions share one lattice, so the
  cells of neighbouring regions line up.

Fields are computed when an agent first needs them; `travel_times` computes them on
demand if no agent has yet.

### Saving them with the trajectory

The HDF5 writer can store the travel times of every waypoint and exit next to the
trajectory, and the loader reads them back in the same form as `travel_times`:

```python
writer = jps.Hdf5TrajectoryWriter(output_file=path, write_travel_times=True)
# ... run the simulation with trajectory_writer=writer, then
writer.close()

with jps.Hdf5TrajectoryLoader(path) as loader:
    for stage_id in loader.load_travel_time_stage_ids():
        grids = loader.load_travel_times(stage_id)  # {region_id: grid}
```

The travel times are written when the writer is closed, so destinations added during the
run are included. They are stored as 32-bit floats.

## Rerouting around congestion: speed fields

The travel times come from a *speed field*: a walking speed for every cell. By
default it is 1 m/s in the open and slower near walls, and it does not know about
other agents. You can replace it. A common use is congestion: estimate the local
density from the agents' positions every few seconds, turn it into a walking speed
with a fundamental diagram, and set that as the speed field. Crowded areas then
look slow to the floor field, and agents weigh the queue at the nearest door against
a longer walk to a free one.

Here 96 agents start in the north half of a room whose exit has two areas, one in
the north-east and one in the south-east corner. Every 2 s the density around every
cell (agents within 1 m) is turned into a speed with Weidmann's fundamental diagram:

```python
import numpy as np
import jupedsim as jps
import shapely

surface = jps.WalkableSurface()
room = surface.add_region(polygon=shapely.box(0, 0, 20, 10))
simulation = jps.Simulation(
    model=jps.CollisionFreeSpeedModel(), geometry=surface
)
exit_id = simulation.add_exit_stage(
    [(shapely.box(19, 8, 20, 10), room), (shapely.box(19, 0, 20, 2), room)]
)
journey = simulation.add_journey(jps.JourneyDescription([exit_id]))
for x in np.arange(10.5, 16.5, 0.5):
    for y in np.arange(6.25, 9.8, 0.5):
        simulation.add_agent(
            journey_id=journey,
            stage_id=exit_id,
            position=(x, y),
            state=jps.CollisionFreeSpeedModelState(),
            region_id=room,
        )


def weidmann(density):
    """Walking speed in m/s at a density in persons/m² (Weidmann 1993)."""
    rho = np.maximum(density, 1e-3)
    return np.clip(
        1.34 * (1 - np.exp(-1.913 * (1 / rho - 1 / 5.4))), 0.01, 1.34
    )


default = simulation.default_speed_fields()[room]
cx, cy = default.cell_centers


def update_speed_field(radius=1.0):
    pos = np.array([agent.position for agent in simulation.agents()])
    if len(pos) == 0:
        return
    dist2 = (cx[..., None] - pos[:, 0]) ** 2 + (cy[..., None] - pos[:, 1]) ** 2
    density = (dist2 < radius**2).sum(axis=-1) / (np.pi * radius**2)
    # Keep the slow-down near walls, scale by how much the crowd slows walking.
    factor = weidmann(density) / weidmann(0.0)
    simulation.set_default_speed_field(
        room, np.maximum(default.values * factor, 0.01)
    )


while simulation.agent_count() > 0:
    if simulation.iteration_count() % 200 == 0:  # every 2 s at the default dt
        update_speed_field()
    simulation.iterate()
```

Without the update, 92 of the 96 agents queue at the north-east area, the one
closest to them, and the room is empty after 22.8 s. With it, 36 agents switch to
the south-east area once the queue builds up, and the room is empty after 18.3 s.

- **The default speed field** of a region is used by every destination. Use
  `set_default_speed_field(region_id, values)` to change it for all of them.
- **A destination can have its own** speed field in a region, for example when only
  the agents heading for one destination should react to the congestion: `set_custom_speed_field(stage_id, region_id,
  values)`. `custom_speed_fields(stage_id)` shows which regions have one, and
  `clear_custom_speed_field(stage_id, region_id)` goes back to the default.
- **Values** are an array of shape `(height, width)` on the region's grid, in m/s.
  On walkable cells they must be finite and at least 0.01 m/s; whether a cell is
  walkable is decided by the geometry, not by the speed field, so values on cells
  that are not walkable are ignored.
- **The speeds change routes, not walking speeds.** How fast agents actually walk is
  still up to the operational model.

Setting a speed field drops the travel times that depend on it; they are computed
again the next time they are needed, so update a speed field every few seconds
rather than every iteration: each update costs a new floor field computation.

## Design decisions, briefly

- **One grid per region, on a shared lattice.** Every region of a `WalkableSurface`
  gets its own grid, so stacked floors and overlapping regions never mix. Because all
  grids share one lattice, travel times pass from one region to the next through
  their connecting edges cell by cell, and the result is the same as if the whole
  surface were one grid.
- **Computed when needed, then kept.** A destination's field is computed the first time an
  agent needs it and reused afterwards. Destinations nobody walks to cost nothing.
- **Checked up front.** The geometry is checked when the simulation is created: if a
  walkable area cannot be reached from the rest, you get an error then, not agents
  stuck later on.
- **32-bit storage.** Travel times and speeds are stored as 32-bit floats, which
  halves the memory per destination; the difference to 64 bits is well below a millisecond
  of travel time.
- **Written in Rust.** The floor field library is a small Rust crate behind the C++
  core. That is invisible from Python; invalid input, such as a speed field of the
  wrong shape, comes back as an error instead of a crash.

## Known limitations

- **Directions are approximate.** The solver is first order: directions can be a few
  degrees off near a destination and behind corners, and travel times along diagonals are
  overestimated by a few percent. Smaller cells help.
- **The first step that needs a field is slower.** Computing a field takes time once,
  and again after its speed field changed. With many destinations and large geometries
  this shows up as a pause in the first iteration.
- **Memory grows with destinations times area.** Every destination keeps one grid per region.
  For large geometries with many destinations, a coarser `cell_size` helps.
- **Narrow passages need small cells,** see the warning above.

## What comes next

We are looking at more accurate directions, computing fields for many destinations in
parallel ahead of time, storing only walkable cells to save memory, finer grids
where the geometry needs them. A complete change to mesh-based floor fields and solvers 
might also be imagineable.
If one of these matters for your work, or something above does not behave 
as described, start a thread in the
[GitHub discussions](https://github.com/PedestrianDynamics/jupedsim/discussions).
