---
title: JuPedSim Installation
tags: [getting_started, troubleshooting]
summary: "Install JuPedSim using Windows executables and dmg files for OSX"
keywords: installation
sidebar: jupedsim_sidebar
folder: jupedsim
permalink: jupedsim_install_executables.html
last_updated: Apr 01, 2022
---

{% include note.html content="Tested on Windows 10 and OSX Monterey (12.1)."%}

## Releases and Installation
Binaries can be downloaded from the [GitHub-repository](https://github.com/JuPedSim/jpscore/releases/).

{%include note.html content="For the installation you will have to accept the [license agreement](https://raw.githubusercontent.com/JuPedSim/jpscore/master/LICENSE)."%}

### OSX

On OSX a DMG file is provided. 
After copying open `jpscore` from Terminal and `jpsvis` by **right-click** on the application.
Your system will warn you that the developer cannot be verified.
Go ahead an open it anyway!

{% include note.html content="For the very first execution of `jpscore`, open it via right-click and explicitly allow it to run."%}

### Windows

Double-click the `jpscore` installer and click on `next` a couple of times.

{%include important.html content="Make sure to add `jpscore` to the PATH as shown in the following screenshot. In case you don't have sufficient rights to change the system path, you will have to call `jpscore` using its full path."%}

![Windows: Install jpscore 2]({{ site.baseurl }}/images/win/jpscore_install2.png)

## Start a simulation 

{%include important.html content="Before starting a simulation make sure to copy the directory examples files to a directory, where you have write permissions, e.g. Desktop."%}

On Windows open `Powershell` in the directory of `examples\jpscore\` as shown in the following screenshot:

![Windows: run simulation 1]({{ site.baseurl }}/images/win/run_sim1.png)

Choose a scenario and run a simulation by calling `jpscore` with an argument (inifile) as follows: 

```bash
jpscore.exe .\02_bottleneck\bottleneck_ini.xml
```

![Windows: run simulation 2]({{ site.baseurl }}/images/win/run_sim2.png)

To run this demo on OSX open the terminal and type:

```bash
./jpscore.exe ./examples/jpscore/02_bottleneck/bottleneck_ini.xml
```

The output of a successful simulation in the Powershell or Terminal should look like the following:

```bash
2022-04-01 05:22:24.656] [info] Loading and parsing the persons attributes
[2022-04-01 05:22:24.656] [info] Done loading pedestrian distribution.
[2022-04-01 05:22:24.656] [info] Init Distribute
[2022-04-01 05:22:24.657] [info] Distributing 80 Agents in Room/Subrom 0/1! Maximum allowed: 3564
[2022-04-01 05:22:24.657] [info] Finished distributing pedestrians
[2022-04-01 05:22:24.705] [info] Adding distances in Room 0 to matrix.
[2022-04-01 05:22:24.706] [info] Adding distances in Room 1 to matrix.
[2022-04-01 05:22:24.706] [info] ffRouter: FloydWarshall done!
[2022-04-01 05:22:24.706] [info] Init Operational Model starting ...
[2022-04-01 05:22:24.706] [info] Init Operational Model done.
[2022-04-01 05:22:27.773] [info] Simulation completed
[2022-04-01 05:22:27.773] [info] Exec Time 3.00s
[2022-04-01 05:22:27.773] [info] Evac Time 49.84s
[2022-04-01 05:22:27.773] [info] Realtime Factor 16.61x
```

A directory called `results` should be produced in the directory where the program was executed.

In this old [YouTube-video](https://youtu.be/qVG607GQaKI) these steps to run a simulation are visualized.

## Visualizing the Results

To visualize the trajectory file produced in the simulation, `jpsvis` can be used.

In `JPSvis` click on File -> Open file and choose the trajectory file that was created previously.
The result should look like in the following screenshot

TODO

{% include links.html %}
