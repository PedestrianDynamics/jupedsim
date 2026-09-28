# jupedsim-vis

A Qt/VTK desktop viewer for [JuPedSim](https://www.jupedsim.org) geometries
and trajectory recordings.

It opens WKT geometry files and the SQLite recordings written by JuPedSim's
`SqliteTrajectoryWriter` (schema versions 1, 2 and 3), shows the walkable
area and lets you replay a simulation, step through frames and inspect
agents.

## Install

```sh
pip install jupedsim-vis
```

## Run

```sh
jupedsim-vis
```

`python -m jupedsim_vis` works as well. Files can also be passed on
the command line:

```sh
jupedsim-vis --wkt geometry.wkt --recording trajectory.sqlite
```

Recordings are opened read-only, so a running simulation writing to the same
file is never disturbed and the file is never migrated to another schema
version.

## Optional: shortest-path preview

The viewer does not need JuPedSim to display a geometry or replay a
recording — the walkable area is triangulated with
[shapely](https://shapely.readthedocs.io).

The interactive shortest-path preview (drag across the geometry to see the
route an agent would take, and its length) is the one feature that uses
JuPedSim's routing engine. To enable it, install JuPedSim alongside the
viewer:

```sh
pip install jupedsim
```

Without it the viewer stays fully usable and reports the reason in its status
bar.

## License

LGPL-3.0-or-later. See [LICENSE](https://github.com/PedestrianDynamics/jupedsim/blob/master/jupedsim_vis/LICENSE).
