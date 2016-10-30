## Get the source code 

- The project is being developed in a [Gitlab repository](https://cst.version.fz-juelich.de/jupedsim/jpsvis). 
- Our [GitHub repository](https://github.com/JuPedSim/jpsvis) is used to host selected tags and releases.

So, get the code from Gitlab and the releases from GitHub. :-)

## Showcase and tutorials


To highlight some features of JuPedSim we have uploaded some videos on our [YouTube channel](https://www.youtube.com/channel/UCKS8w8CUClHEeN4K1SUSMBA).


## Support 

We are heavily working on this project which means that:

- It’s not done. We will be releasing new enhancements, bug fixes etc.
- We love your support. If you find any errors or have suggestions, please write an issue in our [issue-tracker](https://cst.version.fz-juelich.de/jupedsim/jpsvis/issues). We will try hard to fix it.
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

### Install packages for Debian 7.7 (Ubuntu)

```
sudo apt-get install libmgl-qt5
sudo apt-get install libvtk5-dev
sudo apt-get install libvtk5.8-qt4
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

Note that the OpenMP acceleration might be missing under OSX