# JPScore

[![Build Status](https://travis-ci.org/JuPedSim/jpscore.svg?branch=develop)](https://travis-ci.org/JuPedSim/jpscore)
[![GitHub license](https://img.shields.io/badge/license-LGPL-blue.svg)](https://raw.githubusercontent.com/JuPedSim/jpscore/master/LICENSE)
[![DOI](https://zenodo.org/badge/36440436.svg)](https://zenodo.org/badge/latestdoi/36440436)

The core module for performing simulations. See the [online documentation](http://jupedsim.org/jpscore/)


## Building from source

### Linux / macOS
The build is tested on Ubuntu 18.04 with gcc-8 / clang-8 and on macOS Mojave
10.14.5 and apple clang.

#### Requirements
For the compilation of JuPedSim core you need a C++17 capable compiler and standard library.

Required:
* boost (>= 1.65)
* libomp (if you want to use OpenMP with Apple LLVm or Clang on Linux)
* cmake (>= 3.1)

Recommended:
* ninja-build

The following dependencies could be installed system wide or will be installed locally using a script:
* spdlog (>= 1.3)
* fmtlib (>= 6.0)
* CLI11 (>= 1.8)
* catch2

#### How to build
Once you have installed the dependencies continue building as follows:
```bash
git clone https://github.com/JuPedSim/jpscore.git
cd jpscore
mkdir build && cd build
./../scripts/setup-deps.sh
cmake -DCMAKE_PREFIX_PATH=$(pwd)/deps ..
make -j$(nproc)
```

Note: If you do not want to use OpenMP you have to pass `-DUSE_OPENMP=OFF` to
cmake on generation.
```bash
cmake -DUSE_OPENMP=OFF -DCMAKE_PREFIX_PATH=$(pwd)/deps ..
```

The following configuration flags are additionally available:

##### USE_OPENMP defaults to ON
Build `jpscore` with OpenMP support, generation will fail if OpenMP cannot be
found.

##### JPSFIRE defaults to OFF
Build `jpscore` with jpsfire features

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
