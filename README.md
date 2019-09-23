# JPScore

[![Build Status](https://travis-ci.org/JuPedSim/jpscore.svg?branch=develop)](https://travis-ci.org/JuPedSim/jpscore)
[![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/JuPedSim/jpscore/master/LICENSE)
[![DOI](https://zenodo.org/badge/36440436.svg)](https://zenodo.org/badge/latestdoi/36440436)

The core module for performing simulations. See the [online documentation](http://jupedsim.org/jpscore/)


## Building from source

### Linux / macOS
The build is tested on Ubuntu 18.04 with gcc-9.1 / clang-8 and on macOS Mojave
10.14.5 with dependencies from Homebrew and Apple LLVM 10.

#### Requirements
On Linux you will need a C++17 capable compiler and a standard library that
supports `<filesystem>`. On macOS you will need the system compiler with C++17
and filesystem from boost.

Required:
* boost (>= 1.65)
* libomp (if you want to use OpenMP with Apple LLVm or Clang on Linux)
* cgal
* spdlog (libspdlog-dev on Ubuntu, spdlog on brew)
* fmtlib (libfmt-dev on Ubuntu, fmt on brew)
* cmake (>= 3.1)

Recommended:
* ninja-build

#### How to build
Once you have installed all dependencies and cloned the repository continue
with a ninja based build:
```bash
mkdir build
cd build
cmake -GNinja -DCMAKE_BUILD_TYPE=Debug <path-to-cmakelists>
ninja
```

Alternatively you can generate a make based build with:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug <path-to-cmakelists>
make -j$(nproc)
```
Note: If you do not want to use OpenMP you have to pass `-DUSE_OPENMP=OFF` to
cmake on generation.
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_OPENMP=OFF <path-to-cmakelists>
```

The following configuration flags are available:

##### USE_OPENMP defaults to ON
Build `jpscore` with OpenMP support, generation will fail if OpenMP cannot be
found.

##### JPSFIRE defaults to OFF
Build `jpscore` with jpsfire features

##### AIROUTER defaults to OFF
Build `jpscore` with an AI based router

##### BUILD_DOC defaults to OFF
Build internal Doxygen based documentation

##### BUILD_CPPUNIT_TEST defaults to OFF
Build unit tests and add them to ctest

##### BUILD_TESTING defaults to OFF
Build full system tests and add them to ctest

##### BUILD_WITH_ASAN defaults to OFF (Does not support Windows)
Build an additional target `jpscore_asan` with address and undefined behavior
sanitizer enabled. Note there is an approx. 2x slowdown when using
`jpscore_asan` over `jpscore`

## Quick start

See [installation and configuration](http://jupedsim.org/jpscore/2016-11-02-quickstart.html)

## Showcase and tutorials

To highlight some features of JuPedSim we have uploaded some videos on our [YouTube channel](https://www.youtube.com/channel/UCKS8w8CUClHEeN4K1SUSMBA).


## Support

We are heavily working on this project which means that:

- It’s not done. We will be releasing new enhancements, bug fixes etc.
- We love your support. If you find any errors or have suggestions, please write an issue in our [issue-tracker](https://github.com/JuPedSim/jpscore/issues). We will try hard to fix it.
- Be patient. We are scientists and PhD/master students. Therefore, we primarily care about our research and theses.

Enjoy!
