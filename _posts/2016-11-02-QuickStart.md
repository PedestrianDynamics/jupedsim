---
layout: post2
title: Quick start
subtitle: Compiling the code
permalink: 2016-11-02-quickstart.html
nav:
- page: Get the code
  permalink: #1-Get-the-code
  - page: Check the requirements
  permalink: 2016-11-02-quickstart.html#2-check-the-requirements
- page: Compile the code
  permalink: 2016-11-02-quickstart.html#3-compile-the-code
- page: Run an example
  permalink: 2016-11-02-quickstart.html#4-run-an-example
- page: Back to top
  permalink: 2016-11-02-quickstart.html#top
---


## Windows

See [this tutorial with VS](http://www.jupedsim.org/jpscore/2018-10-21-windows)


## Linux and MacOS

To install `JPScore` there  are three steps to follow:


## 1. Get the code

```bash
 git clone https://gitlab.version.fz-juelich.de/jupedsim/jpscore
```

## 2. Check the requirements

```bash
 cd jpscore
 mkdir build && cd build
 cmake ..
```


Some packages and libraries are required to get `jpscore` compiled.
See also [requirements](2016-11-03-requirements.html).

In case something is missing, `CMake` will report an error and point to the missing package.

## 3. Compile the code

If the two first steps succeed then you can proceed with the compilation

```bash
 make -j2
```

In case of a successful compilation, the executables will be located in the directory `bin/`.

## 4. Run an example

In the directory `demos` there are some examples to start with:

<!-- \dirtree{ -->
<!-- .1 \myfolder{red}{demos}. -->
<!-- .2 \myfolder{blue}{scenario\_1\_corridor}. -->
<!-- .3 \myfolder{green}{corridor\_geo.xml}. -->
<!-- .3 \myfolder{green}{corridor\_ini.xml}. -->
<!-- .2 \myfolder{blue}{scenario\_2\_bottleneck}. -->
<!-- .3 \myfolder{green}{bottleneck\_geo.xml}. -->
<!-- .3 \myfolder{green}{bottleneck\_ini.xml}. -->
<!-- .2 .. -->
<!-- .2 .. -->
<!-- .2 \myfolder{blue}{scenario\_7\_floorfield}. -->
<!-- .3 \myfolder{green}{Kobes\_geo.xml}. -->
<!-- .3 \myfolder{green}{ffRouter\_ini.xml}. -->
<!-- .2 .. -->
<!-- .2 .. -->
<!-- } -->

```bash
├── demos
│   ├── scenario_1_corridor
│   │   ├── corridor_geo.xml
│   │   ├── corridor_ini.xml
│   ├── scenario_2_bottleneck
│   │   ├── bottleneck_geo.xml
│   │   └── bottleneck_ini.xml
│   ├── scenario_3_corner
│   │   ├── corner_geo.xml
│   │   ├── corner_ini.xml
│   │   └── corner_routing.xml
│   ├── scenario_4_stairs
│   │   ├── stairs_geo.xml
│   │   ├── stairs_ini.xml
│   │   └── stairs_routing.xml
│   ├── scenario_7_floorfield
│   │   ├── Kobes_geo.xml
│   │   ├── ffRouter_ini.xml
|
|
|.....
│

```

Taking the 7th demo as input, we run a simulation as follows:

```bash
 ./bin/jpscore  demos/corner_ini.xml
```

which produces a trajectory file in the same directory. This can be visualized with `jpsvis`

```bash
 jpsvis demos/scenario_7_floorfield/Kobes_traj.xml
```

![Simulation using demo 7 of `jpscore` ]({{ site.baseurl  }}/img/kobe.gif)
