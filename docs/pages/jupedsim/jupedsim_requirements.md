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

Any compiler with support for C++11.

Tested with

- `g++` (linux/osx)
- `clang` (osx)
- Visual Studio 2019 (Windows)

## Required tools and libraries 

- CMake (>= 3.1)
- Python3 (highly recommended): needed to plot results of analysis.
- boost (>= 1.65)
- libomp (if you want to use OpenMP with Apple LLVm or Clang on Linux)
- spdlog (libspdlog-dev on Ubuntu, spdlog on brew)
- fmtlib (libfmt-dev on Ubuntu, fmt on brew)
- A C++ compiler with support for C++17

