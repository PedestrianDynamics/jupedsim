---
title: JuPedSim Installation
tags: [getting_started, troubleshooting]
summary: "Install JuPedSim using Windows executables and dmg files for OSX"
keywords: installation
sidebar: jupedsim_sidebar
folder: jupedsim
permalink: jupedsim_install_executables.html
last_updated: Apr 23, 2020
---

{% include note.html content="Tested on Windows 10 and OSX Catalina (10.5)"%}

## OSX

Download the following dmg-files :

- [Installer for simulation](https://fz-juelich.sciebo.de/s/0xCVR1zYXao1YDW)
- [Installer for visualisation](https://fz-juelich.sciebo.de/s/L2M9VR4TdRrmV0c)

### Open jpscore

![OSX: Install jpscore 1]({{ site.baseurl }}/images/osx/jpscore0.png)


In the `bin` directory, right-click on the binary `jpscore` and open with a Terminal as shown in the following screenshot:

![OSX: Install jpscore 2]({{ site.baseurl }}/images/osx/jpscore1.png)

This step runs `jpscore` without any input file. This is a workaround to prevent osx from 
warning about not verified developer. 

![OSX: Install jpscore 3]({{ site.baseurl }}/images/osx/jpscore3.png)

Copy the `bin`-directory and the sample files to some directory, where you have right permissions, e.g. Desktop and run 
`jpscore` as follows 

```bash
./jpscore jpscore_samples/bottleneck_ini.xml
```


### Open jpsvis

![OSX: Install jpsvis]({{ site.baseurl }}/images/osx/jpsvis.png)

## Windows

First, Download the following files:

- [Installer for the simulation](https://fz-juelich.sciebo.de/s/D5ZzrpCiGYYyM5I)
- [ZIP-directory for visualisation](https://fz-juelich.sciebo.de/s/1CQ12M7RjXPwefu)

### Install jpscore

Double-click the `JPScore` installer and click on `next` a couple of times.
{%include note.html content="You will have to accept the [license agreement](https://raw.githubusercontent.com/JuPedSim/jpscore/master/LICENSE)."%}
![Windows: Install jpscore 1]({{ site.baseurl }}/images/win/jpscore_install1.png)

{%include important.html content="Make sure to add `JPScore` to the PATH as shown in the following screenshot. In case you don't have sufficient rights to change the system path, you will have to call jpscore using its full path."%}

![Windows: Install jpscore 2]({{ site.baseurl }}/images/win/jpscore_install2.png)

In the final installation-step, you can install all the components including the sample files. See:
![Windows: Install jpscore 3]({{ site.baseurl }}/images/win/jpscore_install3.png)

Finally, complete the installation by clicking on `Finish`

![Windows: Install jpscore 4]({{ site.baseurl }}/images/win/jpscore_install4.png)

### Start a simulation 

{%include important.html content="Before starting a simulation make sure to copy the directory jpscore_samples files to a direction, where you have write permissions, e.g. Desktop."%}

Open `Powershell` in the directory `jpscore_samples` as shown in the following screenshot:

![Windows: run simulation 1]({{ site.baseurl }}/images/win/run_sim1.png)

then run a simulation by giving jpscore an argument (inifile) as follows: 

```bash
jpscore.exe ./bottleneck_ini.xml
```

![Windows: run simulation 2]({{ site.baseurl }}/images/win/run_sim2.png)

The output of a successful simulation with the inifile used above should look like the following:

```
[2020-04-24 11:56:56.782] [info] time:  12.49 ( 100) | Agents:     24 / 30 [20.0%]
[2020-04-24 11:56:56.815] [info] time:  13.45 ( 100) | Agents:     22 / 30 [26.7%]
[2020-04-24 11:56:56.844] [info] time:  14.41 ( 100) | Agents:     22 / 30 [26.7%]
[2020-04-24 11:56:56.870] [info] time:  15.38 ( 100) | Agents:     20 / 30 [33.3%]
[2020-04-24 11:56:56.895] [info] time:  16.34 ( 100) | Agents:     18 / 30 [40.0%]
[2020-04-24 11:56:56.915] [info] time:  17.30 ( 100) | Agents:     16 / 30 [46.7%]
[2020-04-24 11:56:56.932] [info] time:  18.26 ( 100) | Agents:     15 / 30 [50.0%]
[2020-04-24 11:56:56.946] [info] time:  19.22 ( 100) | Agents:     13 / 30 [56.7%]
[2020-04-24 11:56:56.960] [info] time:  20.18 ( 100) | Agents:     11 / 30 [63.3%]
[2020-04-24 11:56:56.970] [info] time:  21.14 ( 100) | Agents:     10 / 30 [66.7%]
[2020-04-24 11:56:56.980] [info] time:  22.11 ( 100) | Agents:      9 / 30 [70.0%]
[2020-04-24 11:56:56.990] [info] time:  23.07 ( 100) | Agents:      7 / 30 [76.7%]
[2020-04-24 11:56:56.996] [info] time:  24.03 ( 100) | Agents:      5 / 30 [83.3%]
[2020-04-24 11:56:57.000] [info] time:  24.99 ( 100) | Agents:      5 / 30 [83.3%]
[2020-04-24 11:56:57.008] [info] time:  25.95 ( 100) | Agents:      3 / 30 [90.0%]
[2020-04-24 11:56:57.011] [info] time:  26.91 ( 100) | Agents:      1 / 30 [96.7%]
[2020-04-24 11:56:57.020] [info] Simulation completed
[2020-04-24 11:56:57.021] [info] Exec Time 1.00s
[2020-04-24 11:56:57.022] [info] Evac Time 27.13s
[2020-04-24 11:56:57.031] [info] Realtime Factor 27.13x
[2020-04-24 11:56:57.037] [info] Number of Threads 1
PS Z:\Desktop\JPScore 0.8.4\jpscore_samples>
```

A directory called `results` should be produced in the same directory as the inifile. 

## Visualize results

To visualize the trajectory file produced in the simulation, you can use `jpsvis`.
Please extract the compressed zip-file you downloaded [here](jupedsim_install_executables.html#windows). 

![Windows: jpsvis 1]({{ site.baseurl }}/images/win/jpsvis1.png)

In `JPSvis` click on File -> Open file 
and choose the trajectory file you produced previously.
![Windows: jpsvis 2]({{ site.baseurl }}/images/win/jpsvis2.png)

Finally, the results should look as follows

![Windows: jpsvis 3]({{ site.baseurl }}/images/win/jpsvis3.png)

{% include links.html %}
