---
title: Build JuPedSim on Windows
tags: [getting_started, troubleshooting]
summary: "Instructions how to compile the code on windows using Visual Studio"
keywords: installation
sidebar: jupedsim_sidebar
folder: jupedsim
permalink: jupedsim_install_on_windows.html
last_updated: Apr 23, 2020
---

{% include note.html content="To use executables rather than build the code, follow this [link](jupedsim_exe_windows.html"%}

## Prerequisites:

- Windows 10
- Visual Studio 2019 or newer
- Git: [download](https://git-scm.com/downloads)
- CMake 3.2 or newer: [download](https://cmake.org/download/)
- Python 3. We recommend the Anaconda distribution: [download](https://repo.anaconda.com/archive/Anaconda3-2019.10-Windows-x86_64.exe)

For `jpscore` and `jpsreport` following C++ libraries are needed:

- zlib
- fmt
- spdlog
- cli11
- Boost

Additionally, for `jpseditor` and `jpsvis` `vtk` and `qt5` are needed.

## Install using package manager vcpkg

To install the required libraries, we recommend using windows pakage manager `vcpkg`.

First install [vcpkg](https://github.com/Microsoft/vcpkg):

```bash
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat
```

then install the required libraries as followed:

```bash
.\vcpkg.exe install zlib:x64-windows fmt:x64-windows spdlog:x64-windows cli11:x64-windows boost:x64-windows
```

## Build jpscore and jpsreport from source

First get the code

```bash
 git clone https://github.com/JuPedSim/jpscore.git
```

Then proceed compiling it as follows

```bash
 cd jpscore
 mkdir build
 cd build
 cmake -DCMAKE_TOOLCHAIN_FILE="path\\to\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake" ..
```

(change `path to` in the cmake call above).

If this runs without errors, it means all requirements are fulfilled and you can proceed with the compilation


In Visual Studio 

```bash
 File -> Open -> Project/Solution
```

and select the Visual-Studio solution produced in the prior step by cmake (sln-file).
After a while when VS finishes indexing the project click on

```bash
 Build -> Build Solution.
```

{% include links.html %}
