---
layout: post
title: Quick start
subtitle: Compiling the code
permalink: 2016-11-02-quickstart.html
---

To install `JPScore` there are three steps to follow: 

# 1. Get the code

```bash
git clone https://cst.version.fz-juelich.de/jupedsim/jpscore
```

# 2. Check the requirements

```bash
cd jpscore
mkdir build && cd build
cmake ..
```


Some packages and libraries are required to get `jpscore` compiled. 
See also [requirements](2016-11-03-requirements.html).

In case something is missing, `CMake` will report an error and point to the missing package.

# 3. Compile the code

If the two first steps succeed then you can proceed with the compilation 

```bash 
make -j2
```

In case of a successful compilation, the executables will be located in the directory `bin/`.

# Run an example 

In the directory `demos` there are some examples to start with:  

```
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

![simulation]({{ site.baseurl  }}/img/kobe.gif)


