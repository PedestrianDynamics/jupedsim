---
title: "Getting started with jupedsim"
keywords: simulation
tags: [jpscore, getting_started]
sidebar: jupedsim_sidebar
permalink: jpscore_introduction.html
summary: jpscore is the simulation module of JuPedSim. It's a command line tool to simulate the evacuate of pedestrians in continous space.
last_updated: 20, December 2019
toc: false
---

`jpscore` implements models on the operational as well as on the tactical level of pedestrian dyanmics.

## Demo files

In the directory `demos` there are some examples to start with:


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


![Simulation using demo 7 of `jpscore` ]({{ site.baseurl  }}/images/kobe.gif)


{% include links.html %}
