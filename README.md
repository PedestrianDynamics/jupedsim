[![Build Status](https://travis-ci.org/JuPedSim/jpscore.svg?branch=v0.8)](https://travis-ci.org/JuPedSim/jpscore)

`jpscore` command line simulation core for JuPedSim.

Features
========

- Simulate pedestrians movement in a space continuous geometry
- Forces-based models for describing the pedestrians interactions
- Shortest and quickest path route choice strategies
- Loading and visualizing trajectories and geometries
- Easy to use visualization interface
- Making high quality videos directly from the visualization interface or generating png screenshots
- XML based input files


Showcase
========

To highlight some features of JuPedSim we have uploaded some videos on our [YouTube channel](https://www.youtube.com/user/JuPedSim).


Installing
==========

This version comes with no installer. So just download the archive corresponding to your architecture and unzip them and you are ready to go. The binaries are only available for windows at the moment. For other architectures (Linux, OSX) you will need to compile the sources. See the section [Compiling from sources](#compiling-from-sources).

Uninstalling
============

As JuPedSim comes with no installer, you just need to delete the unziped directory.

Running
=======

    jpscore.exe my_simulation_ini.xml

from the command line (or also dropping the file on the executable) will generate a trajectory file, which you can visualize with `jpsvis`.
You will find some projects samples in the downloaded files and further information in the manual.

Compiling from sources
======================

You can compile the simulation core for your specific platform with the supplied cmake script.
The only requirement is a compiler supporting the new standard c++11.

Windows (tested on Win7 with MinGW 4.8)
---------------------------------------

    cmake -G "MinGW Makefiles" CMakeList.txt
    make-mingw32.exe

Linux (tested on Ubuntu 14.04 with gcc 4.8)
---------------

    cmake CMakeList.txt
    make

OSX (tested on OSX Maverick with clang 5.1 and Yosemite with clang 6.1)
---------------------

    cmake   CMakeList.txt
    make

Note that the OpenMP acceleration might be missing under OSX

For the visualization module (`jpsvis`) at least Qt version 4.5 and VTK version 4.8 are required.
You can download the latest version of QT [here](https://www.qt.io/download/) and the latest version of VTK [here](http://www.vtk.org/download/).

System Requirements
==============

There is no special hardware/software requirements for running JuPedSim. The distributed binaries however, are only available for windows at the moment. For compiling from sources you need a compiler with c++11 support is needed for the core. Qt version >=4.5 and VTK >5.8 are needed for the visualization.


Known Issues
============

Some verification tests are still failing. The actual cdash-board can be found [here](http://my.cdash.org/index.php?project=JuPedSim) 
Occasionaly jpscore crashes if the input file is not valid. Make sure to validate your XML input files with the supplied XSD files.

Frequently Asked Questions
===========================

- What is the official page of JuPedSim?
www.jupedsim.org and the contact is info at jupedsim.org. You will find more information on the working group and other tools and experimental pedestrians data we have been collecting over the years.

- Where is the official repository?
JuPedSim is developed at the Forschungszentrum Jülich in Germany and the bleeding edge code is in their intern git repository. At the moment only specific tags are pushed to GitHub.

- Is there a manual?
Of course, the user's guide is found in the downloaded archive.

- Are the models validated?
We are actually setting up verification and validation tests. Some verification tests are included in this version but most of them will be available with the next version. This is an ongoing process.

- How can I contribute to JuPedSim?
Testing and reporting bugs will be great. If you may want to contribute actively to the code, by implementing new models and/or features, you are welcome to do so.
