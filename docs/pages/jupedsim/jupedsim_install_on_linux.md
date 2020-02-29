---
title: Install JuPedSim on Linux
tags: [getting_started, troubleshooting]
keywords: installation
summary: "JuPedSim does not offer any Linux binaries. A compilation of the code is necessary."
sidebar: jupedsim_sidebar
permalink: jupedsim_install_on_linux.html
folder: jupedsim
last_updated: Feb 21, 2020
---

## Get the code 

```bash
git clone https://github.com/JuPedSim/jpscore.git
```

## Get dependencies

Before starting building `jpscore` and `jpsreport` it is necessary to install the required [dependencies](jupedsim_requirements.html).

To do so, run the script `scripts/setup-deps.sh`, for example as follows:

```bash
mkdir build
cd build
../scripts/setup-deps.sh
```

This creates a new directory `deps` inside `build` with the required libraries.

## How to build
Once you have installed all dependencies and cloned the repository continue
with a ninja based build:
```bash
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug <path-to-cmakelists> -DCMAKE_PREFIX_PATH=$(pwd)/deps
ninja
```

{%include note.html content="For faster builds you may want to use -DCMAKE_BUILD_TYPE=Release instead."%}

Alternatively you can generate a make based build with:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug <path-to-cmakelists>
make -j$(nproc)
```

{% include note.html content="If you do not want to use OpenMP you have to pass `-DUSE_OPENMP=OFF` to cmake on generation." %}
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_OPENMP=OFF <path-to-cmakelists>
```

## CMake configuration flags

The following configuration flags are available:

- USE_OPENMP defaults to ON (Disabled on Windows)
Build `jpscore` with OpenMP support, generation will fail if OpenMP cannot be
found.
MSVC only support OpenMP 2.0 Standard and is therefor disabled on Windows.

- BUILD_DOC defaults to OFF
Build internal Doxygen based documentation

- BUILD_CPPUNIT_TEST defaults to OFF
Build unit tests and add them to ctest

- BUILD_TESTING defaults to OFF
Build full system tests and add them to ctest

- BUILD_WITH_ASAN defaults to OFF (Does not support Windows)
Build an additional target `jpscore_asan` with address and undefined behavior
sanitizer enabled. Note there is an approx. 2x slowdown when using
`jpscore_asan` over `jpscore`


{% include links.html %}

