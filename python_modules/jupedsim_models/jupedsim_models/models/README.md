# Community models

This directory collects operational models written in Python on top of
`jupedsim.CustomOperationalModel`. It is a place for the community to
share new models and to show how they are implemented.

Runnable example scripts live in the top-level `examples/` directory.
This directory holds model implementations, not scripts.

## What to expect

- **Community driven.** Models here are contributed by their authors.
  The JuPedSim maintainers do not maintain, validate or support them.
- **Slow.** They run in Python, not C++, and are much slower than the
  built-in models. Use them for research, prototyping, teaching and
  comparison, not for large-scale production runs.
- **Not the reference models.** The maintained models are the C++ models
  shipped with JuPedSim (for example the collision-free speed model, the
  generalized centrifugal force model and the anticipation velocity
  model). A model that proves itself here may later be reimplemented in
  C++, but there is no promise of that.

## Template

Use the Python social force model as a starting point:
[`pysocial_force.py`](pysocial_force.py).
It shows how to:

- define the per-agent state as a frozen dataclass,
- subclass `CustomOperationalModel`,
- implement `compute_next_state(state, step)` using what the agent
  perceives in `step` (route orientation, neighbours, walls, `dt`),
- return the updated state and the movement the agent wants to make.

## Where to put your model

- Model: `python_modules/jupedsim_models/jupedsim_models/models/my_model.py`
  (this directory), imported as `jupedsim_models.models.my_model`.
- Test: `python_modules/jupedsim_models/tests/test_my_model.py`.
- Example script, if any: `examples/`.

## Contributing a model

1. Add one module per model, for example `my_model.py`.
2. Start the file with `# SPDX-License-Identifier: LGPL-3.0-or-later`.
3. In the class docstring, cite the paper, give the equations you
   implement and list every parameter with its unit and default.
4. Add a test and a short usage example for the model.
5. State known limitations and deviations from the original publication.

Further collections of Python models:
[A_bunch_of_models](https://github.com/ThoChat/A_bunch_of_models).
