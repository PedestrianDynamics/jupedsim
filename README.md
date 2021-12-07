[![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/JuPedSim/jpscore/master/LICENSE)

[**documentation**](http://www.jupedsim.org) | [**jpscore**](http://www.jupedsim.org/jpscore_introduction.html) | [**jpsreport**](http://www.jupedsim.org/jpsreport_introduction.html)

## Introduction

This repository contains software for simulating pedestrian dynamics.

Analysis of results can be done with [jpsreport](https://github.com/JuPedSim/jpsreport)
Visualization of results can be done with [jpsvis](https://github.com/JuPedSim/jpsvis)

## Building from source

It should be possible to build jpscore on all major platforms however, we only
test a few:

Right now, we ensure a working Build for:

* Windows 10
* MacOS Montery
* MacOS BigSur
* Ubuntu 21.10

### Linux / MacOS

We suggest to use the following layout for developing on jpscore:

```txt
.
├── jpscore <- code repository
├── jpscore-build <- build folder
└── jpscore-deps <- install location of library dependencies

```

#### System Requirements

* C++17 capable compiler
* clang-format-13
* wget

Once the above-listed requirements are installed, build the library dependencies.

#### Library Dependencies

##### Linux MacOS

On Linux and MacOS all library dependencies except boost are built from source,
they are either part of the source tree and do not need any special attention or
they are built with `scripts/setup-deps.sh`. To compile dependencies invoke the
script from any path where you have write access. The script will create a
folder called `deps`, download and compile required dependencies. The resulting
`deps` folder should contain an install tree of all required library
dependencies. Move / rename this folder to a convenient location, e.g. to
`jpscore-deps` as outlined in the beginning.

Additionally, you will need to install boost >= 1.74 development packages for
your distribution.

##### Windows

On Windows dependencies are installed with `vcpkg`, get it from <vcpkg.io>.
`vcpkg` will automatically download and install the dependencies listen in
`vcpkg.json` if used with cmake.

During the CMake invocation listed in the next section, add this to your invocation:

```bash
-DCMAKE_TOOLCHAIN_FILE=<PATH-TO-VCPKG-INSTALLATION>/scripts/buildsystems/vcpkg.cmake
```

#### How to generate build files with CMake

Now that you have all library dependencies, you need to generate build files
with CMake.

Note: We do not recommend in-tree builds.

```bash
mkdir jpscore-build
cd jpscore-build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug <path-to-cmakelists>
ninja
```

Alternatively you can generate a make based build with:

```bash
mkdir jpscore-build
cd jpscore-build
cmake -DCMAKE_BUILD_TYPE=Debug <path-to-cmakelists>
make -j$(nproc)
```

### Build Options

For a list of build options, please see [CMakeLists.txt] directly.

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

