[![Build Status](https://gitlab.version.fz-juelich.de/jupedsim/jpsvis/badges/master/build.svg)](https://gitlab.version.fz-juelich.de/JuPedSim/jpsvis/pipelines)
[![GitHub license](https://img.shields.io/badge/license-GPL-blue.svg)](https://raw.githubusercontent.com/JuPedSim/jpsvis/master/LICENSE)


## Showcase and tutorials

To highlight some features of JuPedSim we have uploaded some videos on our [YouTube channel](https://www.youtube.com/channel/UCKS8w8CUClHEeN4K1SUSMBA).


## Support 

We are heavily working on this project which means that:

- It’s not done. This package is not actively developed.
- We love your support. If you find any errors or have suggestions, please write an issue in our [issue-tracker](https://github.com/jupedsim/jpsvis/issues). We will try hard to fix it.
- Be patient. We are scientists and PhD/master students. Therefore, we primarily care about our research and theses. 

## Requirements 
For the visualization module (`jpsvis`) at least 

- Qt version 4.5 
- and VTK version 4.8 are required.

### Install requirements for Mac OS X (with Homebrew)

```
brew update
brew install vtk --with-qt --without-python --with-python3 --without-boost  --build-from-source
```

We recommend using brew to install `jpsvis`. See [here](https://github.com/JuPedSim/homebrew-jps).

### Install packages for Ubuntu

```
sudo apt-get install qt5-default libvtk7-dev
```

Alternatively, you can download the latest version of QT [here](https://www.qt.io/download/) and the latest version of VTK [here](http://www.vtk.org/download/).


## Compiling from sources


You can compile the simulation core for your specific platform with the supplied cmake script.
The only requirement is a compiler supporting the new standard c++11.

### Windows (tested on Win7 with MinGW 4.8)


    cmake -G "MinGW Makefiles" CMakeLists.txt
    mingw32-make.exe

### Linux (tested on Ubuntu 14.04 with gcc 4.8)

    cmake CMakeLists.txt
    make

### OSX (tested on OSX Maverick with clang 5.1 and Yosemite with clang 6.1)


    cmake   CMakeLists.txt
    make

Note that the OpenMP acceleration might be missing under OSX.
