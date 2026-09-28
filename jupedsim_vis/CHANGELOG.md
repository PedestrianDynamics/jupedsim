# Changelog

## 1.0.0

- First release as a separate package. Up to jupedsim 1.4 the viewer shipped
  inside the `jupedsim` distribution.
- Renamed: the distribution is `jupedsim-vis`, the Python package
  `jupedsim_vis` and the command `jupedsim-vis`. The old names collide
  with the copy of the viewer that jupedsim <= 1.4 installs itself, so
  installing such a jupedsim next to the old package silently overwrote
  its files and the `jupedsim-visualizer` command.
- `jupedsim-vis` is a console-script entry point;
  `python -m jupedsim_vis` works as well.
- `jupedsim` is not a dependency. It is only used for the interactive
  shortest-path preview; install it alongside the viewer to enable that
  feature. Without it the viewer is fully usable and explains the missing
  feature in its status bar.
- The walkable area is triangulated with shapely
  (`constrained_delaunay_triangles`) instead of jupedsim's routing engine.
  This requires `shapely >= 2.1`.
- The geometry info reports "Polygons" as the number of input polygons (the
  viewer bundled with jupedsim showed the navigation mesh triangle count)
  and adds a separate "Triangles" count.
- Recordings are opened read-only and are never migrated: schema versions 1,
  2 and 3 are read directly, so a file written by an older jupedsim stays
  untouched.
- Command line flags `--wkt FILE` and `--recording FILE` (both repeatable)
  open files on startup, plus a `testing` flag group with `--smoke-test` to
  start headless, run self-checks and exit.
