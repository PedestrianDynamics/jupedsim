[![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/JuPedSim/jpscore/master/LICENSE)

[**documentation**](http://www.jupedsim.org) | [**jpscore**](http://www.jupedsim.org/jpscore_introduction.html) | [**jpsreport**](http://www.jupedsim.org/jpsreport_introduction.html)

## Introduction

This repository contains software for simulating pedestrian dynamics (jpscore)
and visualizing the resulting trajectories (jpsvis).

Analysis of results can be done with [jpsreport](https://github.com/JuPedSim/jpsreport)

## Quick start

See [Getting started with jupedsim](http://www.jupedsim.org/jpscore_introduction.html).

## Showcase and tutorials

To highlight some features of JuPedSim, we have uploaded some videos on our
[YouTube channel](https://www.youtube.com/channel/UCKS8w8CUClHEeN4K1SUSMBA).

## Support

If you got a question or a problem and need support from our team feel free to
contact us. You can do this via [email](mailto:dev@jupedsim.org).

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

It should be possible to build on all major platforms however, we only test a few:

Right now, we ensure a working Build for:

* Windows 10
* MacOS Montery
* Ubuntu 21.10

### Build Options

We support only a few settings. For a list of build options, please see
[CMakeLists.txt] directly.

### General Information

All of the following descriptions assume the following layout on disk:

```txt
.
├── jpscore <- code repository
├── jpscore-build <- build folder
└── jpscore-deps <- install location of library dependencies

```

### Build on Linux

#### System Requirements

Builds are only tested on the latest Ubuntu version (at this time 21.04). To
compile from source you will need a couple of system dependencies.

* C++17 capable compiler (default GCC will do)
* wget
* vtk9 with qt support
* qt 5

Recommended:

* ninja

For a exact list of ubuntu packages required to build please consult
[container/build/Dockerfile]

#### Library Dependencies

On Linux and MacOS all dependencies except VTK & QT are built from source, they are
either part of the source tree and do not need any special attention or
they are built with `scripts/setup-deps.sh`. To compile dependencies invoke the
script and specify the install path:

```bash
./scripts/setup-deps.sh --install-path ~/jspcore-deps
```

The output created in `~jpscore-deps` now contains an install tree of all required
library dependencies.

Warning: If you do not specify an install path the script tries to install into
`/usr/local`.

#### Compiling

Now that you have all library dependencies, you need to generate build files
with CMake and compile.

```bash
mkdir jpscore-build
cd jpscore-build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH=<path-to-dependencies> \
    <path-to-cmakelists>
ninja
```

Alternatively you can generate a make based build with:

```bash
mkdir jpscore-build
cd jpscore-build
cmake -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH=<path-to-dependencies> \
    <path-to-cmakelists>
make -j$(nproc)
```

You will find `jpscore` and `jpsvis` executables in `jpscore-build/bin` after
the build.

### Build on MacOS

#### System Requirements

* homebrew for qt / vtk packages
* wget
* vtk9 from homebrew
* qt from homebrew

Recommended:

* ninja

Once the above-listed requirements are installed, build the library dependencies.

#### Library Dependencies

On Linux and MacOS all dependencies except VTK & QT are built from source, they are
either part of the source tree and do not need any special attention or
they are built with `scripts/setup-deps.sh`. To compile dependencies invoke the
script and specify the install path:

```bash
./scripts/setup-deps.sh --install-path ~/jspcore-deps
```

The output created in `~jpscore-deps` now contains an install tree of all required
library dependencies.

Warning: If you do not specify an install path the script tries to install into
`/usr/local`.

#### Compiling

Now that you have all library dependencies, you need to generate build files
with CMake and compile.

```bash
mkdir jpscore-build
cd jpscore-build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH=<path-to-dependencies> \
    <path-to-cmakelists>
ninja
```

Alternatively you can generate a make based build with:

```bash
mkdir jpscore-build
cd jpscore-build
cmake -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH=<path-to-dependencies> \
    <path-to-cmakelists>
make -j$(nproc)
```

You will find `jpscore` executable and `jpsvis.app` app bundle in
`jpscore-build/bin` after the build. To start `jpsvis` either click on the
bundle in `jpscore/bin` or run it from the command line with
`./bin/jpsvis.app/Contents/MacOS/jpsvis`

### Build on Windows

#### System Dependencies

* Visual Studio

#### Library Dependencies

On Windows dependencies are installed with `vcpkg`, get it from <vcpkg.io>.
`vcpkg` will automatically download and install the dependencies listen in
`vcpkg.json` if used with CMake.

During the CMake invocation listed in the next section, add this to your invocation:

```bash
-DCMAKE_TOOLCHAIN_FILE=<PATH-TO-VCPKG-INSTALLATION>/scripts/buildsystems/vcpkg.cmake
-DVCPKG_MANIFEST_DIR=<PATH-TO-SOURCE-FOLDER>/vcpkg.json
```

## Contributing to JuPedSim

This project is mainly developed by a small group of researchers and students
from [Jülich Research Center](http://www.fz-juelich.de/en) and
[BUW](http://www.uni-wuppertal.de/). However, you are kindly invited not only to
use JuPedSim but also contributing to our open-source-project. It does not
matter if you are a researcher, student or just interested in pedestrian
dynamics. There are only a few rules and advices we want to give to you:

### Workflow

We use a fork based workflow for all development. If you want to contribute any
modification we kindly ask you to fork this repository, create a branch with
your changes on your fork and then open a Pull Request via GitHub.

As part of our CI integration we run formatting checks so make sure your code
is formatted with clang-format!

If you want to support us by writing the enhancement yourself, consider what
kind of change it is:

* **Major changes** that you wish to contribute to the project should be
  discussed first on our **dev mailing list** so that we can better coordinate
  our efforts, prevent duplication of work, and help you to craft the change so
  that it is successfully accepted into the project.
* **Small changes** can be crafted and submitted to our repository as a **pull
  or merge request**.

Nevertheless, open an issue for documentation purposes with the following template:

### Code formatting

This has become very easy:

* All code needs to be formatted with clang-format
  (A build target for checking / formatting is available: `check-format` /
  `reformat`)
* Use only spaces in code
