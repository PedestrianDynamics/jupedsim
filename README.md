[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1293771.svg)](https://doi.org/10.5281/zenodo.1293771)
[![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/PedestrianDynamics/jupedsim/master/LICENSE)
![PyPI - Python Version](https://img.shields.io/pypi/pyversions/jupedsim)
![PyPI - Version](https://img.shields.io/pypi/v/jupedsim)

# Jülich Pedestrian Simulator - JuPedSim

JuPedSim is a library to simulate pedestrian dynamics. This software was
initiated at the Institute for Civil Safety
[IAS-7](https://www.fz-juelich.de/en/ias/ias-7) of the Jülich Research Center
(Forschungszentrum Jülich GmbH) in Germany and continues to evolve with support and
contributions from a diverse and engaged community.

We are grateful to all the contributors who have helped shape JuPedSim into what
it is today.

You can find the full list of contributors [here](CONTRIBUTORS.md).

**AI Usage Disclosure**: We use AI coding agents in the development of JuPedSim.
Commits generated partially or fully by coding agents are annotated.

## Quick User Guide

### Installation

It is easiest to install directly with pip from
[PyPI](https://pypi.org/project/jupedsim/):

```
pip install jupedsim
```

### Usage

Please consult our [documentation](http://jupedsim.org).

## Contributing

JuPedSim is licensed under [GNU LGPLv3](LICENSE). We are open to
contributions. 

> [!NOTE]
> For contributions beyond trivial fixes we require you to first
> interact with us through GitHub discussions.

> [!IMPORTANT]
> We require all commits with AI-generated code to be flagged with
> a git trailer naming the model and the provider's no-reply address, e.g.:
> `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`

If you have found an issue with JuPedSim and want to report it, open a new GitHub
discussion, ideally with a reproducer that helps us diagnose the issue.

### Questions

If you have a question or a problem, please open a new topic in
[GitHub discussions](https://github.com/PedestrianDynamics/jupedsim/discussions).

## License

[GNU LGPLv3](LICENSE)

## Building from source

Here you have two options.

### With pip

You will need a C++20-capable compiler and a Rust toolchain (`rustc` and
`cargo`, >= 1.88) installed on your system. CMake and Ninja are installed
automatically if they are missing. Then call `pip install .` in the source
directory:

```bash
cd jupedsim
git submodule update --init
pip install .
```

### Compile yourself

You will need a C++20-capable compiler, CMake >= 3.22 as well as a Rust
toolchain (`rustc` and `cargo`, >= 1.88) installed on your system. Then install
our Python dependencies via pip. Our Python package dependencies are listed in
`requirements.txt` in the root of this repository. Now you can generate
makefiles with CMake, then compile and run the Python library.

```bash
git -C jupedsim submodule update --init
pip install -r jupedsim/requirements.txt
mkdir jupedsim-build
cd jupedsim-build
cmake ../jupedsim
make -j
source ./environment
```

The last line puts `<build>/stage` on your `PYTHONPATH`. The stage is an
importable `jupedsim` package: the Python sources (linked from the source
tree), the compiled module and its type stubs. After every `make`/`ninja`, you
can use the result from Python right away, without reinstalling.

> [!TIP]
> Enable automatic recursion into submodules for this repo by calling:
>
> ```bash
> git config submodule.recurse true
> ```

> [!NOTE]
> JuPedSim does not build against Python 3.13.4 on Windows.

> [!WARNING]
> When sourcing `./environment` from the build folder you need to ensure
> JuPedSim is not installed in the current Python environment. Otherwise there
> will be erroneous calls to the wrong Python code, resulting in crashes and/or
> exceptions.
