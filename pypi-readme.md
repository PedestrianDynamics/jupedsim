<div align="center">
  <img src="https://www.jupedsim.org/stable/_static/jupedsim.svg" height="100px" alt="JuPedSim Logo"><br>
</div>

-----------------

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1293771.svg)](https://doi.org/10.5281/zenodo.1293771)
[![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/PedestrianDynamics/jupedsim/master/LICENSE)
![PyPI - Python Version](https://img.shields.io/pypi/pyversions/jupedsim)
![PyPI - Version](https://img.shields.io/pypi/v/jupedsim)

# JuPedSim - Jülich Pedestrian Simulator

*JuPedSim* is a library for simulating pedestrian dynamics.
This software is mainly developed at the Institute for Civil Safety [IAS-7](https://www.fz-juelich.de/en/ias/ias-7) of the Jülich Research Center (Forschungszentrum Jülich) in Germany. 

## Table of Contents

- [Main Features](#main-features)
- [Where to get it](#where-to-get-it)
- [License](#license)
- [Documentation](#documentation)
- [Getting Help](#getting-help)
- [Issues, Discussion and Development](#issues-discussion-and-development)
 
## Main Feature

*JuPedSim* offers a Python interface, to set up and conduct pedestrian dynamics simulations.
With *JuPedSim* you can simulate small, simple layouts as bottleneck, but you can also simulate large, complex scenarios.
In these complex scenarios, different crowd management measures can be modeled with built-in modules.

For simulating the pedestrian movement and interactions *JuPedSim* offers two different microscopic models out of the box.
The Collision Free Speed Model, as the name imply a speed model.
And a force model with the Generalized Centrifugal Force Model.

With *JuPedSim* different decisions-making processes in the agents route choice can be modeled.
These routes may consist of multiple steps the agents have to complete. 
One step may already be completed when an agent reaches the target, but it may also only be completed when a condition like a waiting is fulfilled.
After each completion one of the built-in deciders can be used to conditionally modify the agents next target. 

## Where to get it

It is easiest to install directly with pip from
[PyPi.org](https://pypi.org/project/jupedsim/)

```
pip install jupedsim
```

For information how to build *JuPedSim* from source, visit our [GitHub repository](https://github.com/PedestrianDynamics/jupedsim/).

## License

[GNU LGPLv3](LICENSE)

## Documentation

The documentation of *JuPedSim* is hosted on: https://www.jupedsim.org

## Getting Help

For usage questions, the best place to go to are the [GitHub discussions]() and the [documentation](https://www.jupedsim.org).

## Issues, Discussion and Development

If you found a bug and want to give us a chance to fix it we would be very
happy to hear from you. To make it easy for us to help you please include the
following information when you open a [new
issue](https://github.com/PedestrianDynamics/jupedsim/issues):

* What did JuPedSim do?
* What did you expect JuPedSim to do?
* How can we reproduce the issue?

If you encounter a bug and are would like to submit a fix feel free to open a
PR, we will look into it.

Before embarking on larger work it is a good idea to
[discuss](https://github.com/PedestrianDynamics/jupedsim/discussions) what you
plan.
