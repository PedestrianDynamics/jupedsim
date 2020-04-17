---
title: Trajectory file
keywords: simulation
tags: [jpscore, file]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_trajectory.html
summary: The results of jpscore simulation are written to files in TXT format. This format can be used by other JuPedSim modules.
last_updated: April 17, 2020
---

Possible formats are:

- `plain` a flat format (just numbers)

## TXT
The other format of the trajectory file is `plain`.

{%include note.html content="If a file size of 16Mb is exceeded, `jpscore` automatically splits the output into several files. The files are numbered in ascending order and contain the information of consecutive frames. If you wish to analyze the data with `jpsreport` the output data need to be merged. "%}

### Default output
A sample trajectory in the plain format is as follows:

```xml
#description: jpscore (0.8.4)
#count: 0
#framerate: 16.00
#geometry: geometry.xml
#sources: sources.xml
#goals: goals.xml
#ID: the agent ID
#FR: the current frame
#X,Y,Z: the agents coordinates (in metres)
#A, B: semi-axes of the ellipse
#ANGLE: orientation of the ellipse
#COLOR: color of the ellipse

#ID	FR	X	Y	Z	A	B	ANGLE	COLOR
1	0	3.30	3.33	0.00	0.18	0.25	-90.00	0
2	0	4.50	4.44	0.00	0.18	0.25	-90.00	0
3	0	3.60	3.70	0.00	0.18	0.25	180.00	0
4	0	3.60	4.07	0.00	0.18	0.25	180.00	0
5	0	4.50	4.07	0.00	0.18	0.25	-90.00	0
6	0	4.20	3.33	0.00	0.18	0.25	-90.00	0
```
`count` is a running number. Handy when big simulations are splitted into small 10 MB large files.

{%include note.html content="Other files can be included as well. For example events.xml or schedule.xml"%}

### Addtional output
```xml
#description: jpscore (0.8.4)
#count: 0
#framerate: 8.00
#geometry: [absolute path to file]/bottleneck_geo.xml
#ID: the agent ID
#FR: the current frame
#X,Y,Z: the agents coordinates (in metres)
#A, B: semi-axes of the ellipse
#ANGLE: orientation of the ellipse
#COLOR: color of the ellipse
#V: speed of the pedestrian (in m/s)
#Vx: x component of the pedestrian's velocity
#Vy: y component of the pedestrian's velocity
#FG: id of final goal
#CG: id of current goal
#Dx: x component of the pedestrian's desired direction
#Dy: y component of the pedestrian's desired direction
#SPOT: ped is highlighted
#ROUTER: routing strategy used during simulation
#GROUP: group of the pedestrian


#ID	FR	X	Y	Z	A	B	ANGLE	COLOR	V	Vx	Vy	FG	CG	Dx	Dy	SPOT	ROUTER	GROUP
1	0	55.70	103.00	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
2	0	52.70	102.10	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
3	0	54.80	100.60	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
4	0	53.60	102.70	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
5	0	50.30	102.70	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
6	0	54.20	100.30	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
7	0	55.40	102.10	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
8	0	52.70	100.60	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
9	0	53.00	103.30	0.00	0.15	0.15	0.00	0	0.00	0.00	0.00	0	16	0.00	0.00	0	2	1
```
