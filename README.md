[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1293771.svg)](https://doi.org/10.5281/zenodo.1293771)
[![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/PedestrianDynamics/jupedsim/master/LICENSE)
![PyPI - Python Version](https://img.shields.io/pypi/pyversions/jupedsim)
![PyPI - Version](https://img.shields.io/pypi/v/jupedsim)

# Jülich Pedestrian Simulator - JuPedSim

JuPedSim is a library to simulate pedestrian dynamics. This software is mainly
developed at the Institute for Civil Safety
[IAS-7](https://www.fz-juelich.de/en/ias/ias-7) of the Jülich Research Center
(Forschungszentrum Jülich) in Germany.

## Installation

It is easiest to install directly with pip from
[PyPi.org](https://pypi.org/project/jupedsim/)

```
pip install jupedsim
```

## Usage

Please consult our [documentation.](http://jupedsim.org)

## Contributing

We are open for contributions and would be happy to see Questions, Issues and
Pull Requests.

### Questions

If you have a question or a problem please open a new topic in [GitHub
discussions](https://github.com/PedestrianDynamics/jupedsim/discussions).

### Issues

If you found a bug and want to give us a chance to fix it we would be very
happy to hear from you. To make it easy for us to help you please include the
following information when you open a [new
issue](https://github.com/PedestrianDynamics/jupedsim/issues):

* What did JuPedSim do?
* What did you expect JuPedSim to do?
* How can we reproduce the issue?

### Pull Requests

If you encounter a bug and are would like to submit a fix feel free to open a
PR, we will look into it.

Before embarking on larger work it is a good idea to
[discuss](https://github.com/PedestrianDynamics/jupedsim/discussions) what you
plan.

While we are very happy if you contribute we reserve us the right to
decline your PR because it may not fit into our vision of JuPedSim.

## License

[GNU LGPLv3](LICENSE)

## Building from source

Here you have two options.

### With setuptools

You will need a C++20 capable compiler and CMake >= 3.19 installed on your
system. Then install our python dependencies via pip. Our python package
dependencies are listed in `requirements.txt` in the root of this repository.
Now you can call `pip install .`

E.g.:

```bash
cd jupedsim
pip install -r requirements.txt
pip install .
```

### Compile yourself

You will need a C++20 capable compiler and CMake >= 3.19 installed on your
system. Then install our python dependencies via pip. Our python package
dependencies are listed in `requirements.txt` in the root of this repository.
Now you can generate makefiles with CMake, then compile and run the python
library.

```bash
pip install -r jupedsim/requirements.txt
mkdir jupedsim-build
cd jupedsim-build
cmake ../jupedsim
make -j
source ./environment
```

The last line in the above description will populate the python path with the
location of our python code and the native library.

> [!WARNING]
>
> When sourcing `./environment` from the build folder you need to ensure JuPedSim
> is not installed in the current python environment. Otherwise there will be
> erroneous calls to the wrong python code, resulting in crashes and/or
> exceptions.

