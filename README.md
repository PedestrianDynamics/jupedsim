[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.6144559.svg)](https://doi.org/10.5281/zenodo.6144559) [![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/PedestrianDynamics/jupedsim/master/LICENSE)

## Introduction

JuPedSim is a library to simulate pedestrian dynamics. The main API is the
`jupedsim` python module, we also support a C-API onto of which the python
module is build.

For how to use JuPedSim see the code in `examples`

## Questions and Suggestions

If you have a question or a problem and need support from our team feel free to
open a new topic in GitHub discussions
[https://github.com/PedestrianDynamics/jupedsim/discussions]

Please do not use the issue tracker for personal support requests.

### Reporting bugs and requesting features

If you find a bug in the source code, a mistake in the documentation or think
there is functionality missing you can help us by submitting an issue.

If you submit a bug please make sure to include:

* What version / commit id you are using
* What did you expect the software to do
* What did the software do instead
* A description of how to reproduce the issue if possible
* Log output

If you submit a feature request please help us by including:

* Use a clear and descriptive title for the issue to identify the suggestion.
* Provide a step-by-step description of the suggested enhancement in as many
  details as possible.
* Provide specific examples to demonstrate the steps.
* Describe the current behavior and explain which behavior you expected to see
  instead and why.

## Building from source

It should be possible to build on all major platforms, however we only test on
a limited set:

Right now, we ensure a working Build for:

* Windows 10
* MacOS 13 (Ventura)
* Ubuntu 22.04

### General Information

All of the following descriptions assume the following layout on disk:

```txt
.
├── jupedsim <- code repository
└── jupedsim-build <- build folder

```

We support only a few settings. For a list of build options, please see
[CMakeLists.txt] directly.

### Build on Linux / MacOS / Windows

#### System Requirements

To compile from source you will need a C++20 capable toolchain to be available
and development headers for python3.10 or later.

#### Configure and Compile

```bash
cmake -S jupedsim -B jupedsim-build
cmake --build jupedsim
```

### How to use `jupedsim`

On Linux and MacOS you will find a file `environment` inside the build folder,
this contains the PYTHONPATH required to import `jupedsim`. On Windows you will
need to add the paths to the PYTHONPATH by yourself.

The following locations need to be on the PYTHONPATH

```
jupedsim-build/lib/
jupedsim/python_modules/jupedsim
jupedsim/python_modules/jupedsim-visualizer
```

