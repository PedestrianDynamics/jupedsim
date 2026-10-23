# trame_viewer

## Install & run

You need a python setup with all of jupedsims dependencies.

`jupedsim` is intentionally **not** a declared dependency, ensure `jupedsim` is
built and `environemnt` from the build folder is sourced before trying to run
`trame_viewer`.

Development flow (shared venv + build tree):

```sh
source <build-dir>/environment               # PYTHONPATH: jupedsim + built bindings
pip install -e python_modules/trame_viewer   # from the checkout root, once

trame_viewer [--obj mesh.obj | --h5 run.h5] [--native] [--port N]
```

`python -m trame_viewer` works as well.

## Replay

Opening a file written by `jupedsim.Hdf5TrajectoryWriter` (`--h5` or
*File → Load OBJ or HDF5…*) shows its geometry and replays the recorded agents:
play/pause, a frame slider and a playback speed sit in the drawer. Frames are
read from the file as they are shown. Agents follow the height clip like the
mesh. A file whose run stopped before the writer was closed plays too.

## Travel times

Files written by `jupedsim.Hdf5TrajectoryWriter(..., write_travel_times=True)`
can be opened with `--h5` or *File → Load OBJ or HDF5…*. The *Travel Times*
mode then colours the walkable surface by the floor field travel time to the
selected stage; a right-click reads off the value. An HDF5 file brings only its
mesh, not a `Geometry`, so *Pathfinding* and *Boundary Highlight* need an OBJ.
