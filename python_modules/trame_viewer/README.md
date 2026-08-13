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

trame_viewer [--obj mesh.obj] [--native] [--port N]
```

`python -m trame_viewer` works as well.
