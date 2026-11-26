---
date: "2026-11-10"
author: "JuPedSim team"
description: "How floor fields guide agents towards waypoints and exits, and how speed fields can redirect them around congestion."
---

<!-- SPDX-License-Identifier: LGPL-3.0-or-later -->

# Floor fields in JuPedSim 2.0

In JuPedSim 2.0, agents use floor fields to reach their targets. A floor field assigns an estimated travel time to each grid cell, based on a prescribed speed field. Agents follow decreasing travel times towards their destination, around obstacles and through connected regions and stairs. Their actual movement is determined by the operational model.

In this article, we will cover how to look at the fields, and how to change them, for example to reroute agents
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

- **Agents head towards the part of the destination with the lowest estimated travel time.** For a wide exit or a destination comprising several areas, this need not be its centre or the closest point.
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
    geometry=geo,
    floorfield_options=jps.FloorfieldOptions(
        cell_size=0.2,  # m, the default
        wall_influence_radius=0.3,  # m, the default
    ),
)
```

- **`cell_size`** is the edge length of a grid cell. Smaller cells resolve the
  geometry more finely and give smoother routes; they cost memory and time when a
  field is computed (four times as many cells when you halve the size).
- **`wall_influence_radius`** is how far from a wall the field assumes slower
  walking. Set it smaller to let routes run closer to walls.

```{warning}
A passage narrower than two grid cells may become blocked in the floor field. Cells intersected by a wall are marked as non-walkable, which can leave no continuous path of walkable cells through the passage.
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

geo = jps.GeometryBuilder()
room = geo.add_region(polygon=shapely.box(0, 0, 20, 10))
simulation = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=geo)
north = simulation.add_exit_stage([(shapely.box(19, 8, 20, 10), room)])
south = simulation.add_exit_stage([(shapely.box(19, 0, 20, 2), room)])

grid = simulation.travel_times(north)[room]
plt.imshow(
    np.ma.masked_invalid(grid.values), origin="lower", extent=grid.extent
)
plt.colorbar(label="travel time to the north exit [s]")
```

```{figure} floor-fields/travel-time.png
:alt: Travel time to the north exit over the 20 m by 10 m room, with white contour lines every 2 s and grey arrows for the walking direction

The field of the example, with contour lines every 2 s and one arrow of
`grid.directions` per metre. The travel time grows with the distance to the
exit. Near the walls the field assumes slower walking, so the contour lines
bend and the arrows point away from the wall.
```

- `grid.values` has shape `(height, width)`; row 0 is the lowest. It is zero inside
  the destination's area and `inf` where a cell is not walkable.
- `grid.directions` has shape `(height, width, 2)`: the unit vector an agent at the
  cell's centre is sent along, zero inside the destination's area, `nan` where a cell is
  not walkable.
- `grid.origin`, `grid.width`, `grid.height`, `grid.cell_size` and `grid.extent`
  describe the grid's position and dimensions. The grids of all regions share one lattice, so the
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

Travel times are computed from a *speed field*, which assigns a walking speed to each grid cell. The default speed is 1 m/s away from walls and decreases near them, without accounting for other agents. To include congestion, you can periodically estimate local density from agent positions, convert it into a speed using a fundamental diagram, and update the speed field. This increases the estimated travel time through crowded areas, allowing agents to take a longer route to a less congested exit. The speed field affects route choice; the operational model still determines how fast agents move.

Here 96 agents start in the north half of a room whose exit has two areas, one in
the north-east and one in the south-east corner. Every 2 s the density around every
cell (agents within 1 m) is turned into a speed with Weidmann's fundamental diagram:

```python
import numpy as np
import jupedsim as jps
import shapely

geo = jps.GeometryBuilder()
room = geo.add_region(polygon=shapely.box(0, 0, 20, 10))
simulation = jps.Simulation(model=jps.CollisionFreeSpeedModel(), geometry=geo)
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

Without the update, 93 of the 96 agents queue at the north-east area, the one
closest to them, and the room is empty after 23.8 s. With it, 34 agents switch to
the south-east area once the queue builds up, and the room is empty after 16.8 s.

```{figure} floor-fields/congestion.gif
:alt: Animation of both runs, one above the other: without the update nearly all agents queue at the north-east area; with it, part of the crowd turns to the south-east area

Both runs, agents marked by the exit area they leave by. The background shows
the speed field; with the update it is recomputed every 2 s and darkens where
agents crowd.
```

```{figure} floor-fields/congestion.png
:alt: Four panels: travel time to the exit at 8 s without and with the update, the updated speed field at 8 s, and the number of agents in the room over time for both runs

Why agents switch. (a) Without the update the travel time ignores the crowd,
and 93 of the 96 agents head for the north-east area. (b) With the update the queue
slows the speed field (c), the travel time through the queue grows, and for the
agents in the south part of the crowd the south-east area becomes quicker. (d)
Agents in the room over time.
```

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

Changing a speed field invalidates the cached travel times that depend on it; they are computed
again the next time they are needed, so update a speed field every few seconds
rather than every iteration: each update costs a new floor field computation.

## Implementation details

- **One grid per region, on a shared lattice.** Every region of a `WalkableSurface`
  gets its own grid, so stacked floors and overlapping regions never mix. Because all
  grids share one lattice, travel times pass from one region to the next through
  their connecting edges cell by cell, and the result is the same as if the whole
  surface were one grid.
- **Computed when needed, then kept.** A destination's field is computed the first time an
  agent needs it and reused afterwards.
- **Connectivity checked at creation.** When the simulation is created, JuPedSim checks for disconnected walkable areas and reports an error if it finds any. Agents may still get stuck during the simulation due to interactions in the operational model.
- **32-bit storage.** Travel times and speeds are stored as 32-bit floats, which
  halves the memory per destination; the difference from 64-bit storage is well below a millisecond
  of travel time.
- **Written in Rust.** The floor field library is a small Rust crate behind the C++
  core. That is invisible from Python; invalid input, such as a speed field of the
  wrong shape, comes back as an error instead of a crash.

## Known limitations

- The first-order solver introduces errors in travel times and directions, particularly near corners and destinations. Reducing the cell size improves resolution.
- **The first step that needs a field is slower.** Computing a field takes time once,
  and again after its speed field changed. With many destinations and large geometries
  this shows up as a pause in the first iteration.
- **Memory grows with destinations times area.** Every destination keeps one grid per region.
  For large geometries with many destinations, a coarser `cell_size` helps.
- **Narrow passages need small cells,** see the warning above.

## What comes next

We are considering more accurate direction estimates, parallel computation of fields before a simulation starts, and lower memory use by storing only walkable cells. Adaptive grids or mesh-based solvers are further options.

If your simulations would benefit from these changes, describe your use case in [GitHub discussions](https://github.com/PedestrianDynamics/jupedsim/discussions).
Please also report behaviour that differs from this description.
