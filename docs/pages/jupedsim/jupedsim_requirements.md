---
title: Requirements
subtitle: Dependencies
summary: In order to compile JuPedSim it is necessary to first install the required libraries.
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jupedsim_requirements.html
las_updated: Dec 20, 2019
---


## Supported compilers

Any compiler with support for C++17.

At the moment The following compiler are tested:

- g++-8 on Linux
- clang++-8 on Linux
- Apple Clang 11 on MacOS
- MSVC 19 on Windows


## Required tools and libraries 

- CMake (>= 3.1)
- Python3 (highly recommended): needed to plot results of analysis.
- boost (>= 1.65)
- libomp (if you want to use OpenMP with Apple LLVm or Clang on Linux)
- spdlog (libspdlog-dev on Ubuntu, spdlog on brew)
- fmtlib (libfmt-dev on Ubuntu, fmt on brew)


