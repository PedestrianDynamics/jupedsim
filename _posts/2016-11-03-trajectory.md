---
layout: post2
title: Trajectory file
subtitle:
#bigimg: /img/jupedsim_small.png
permalink: 2016-11-03-trajectory.html
nav:
- page: XML (xml-plain)
  permalink: 2016-11-03-trajectory.html#xml
- page: TXT (plain)
  permalink: 2016-11-03-trajectory.html#txt
- page: Back to top
  permalink: 2016-11-03-trajectory.html#top
---


The results of the simulation are written to files or streamed to a network socket.

Possible formats are:

- `xml-plain` which is the default xml format
- `plain` a flat format (just numbers)

Note that if you are using the streaming mode or want to visualize the trajectories with `jpsvis`,
the format should be `xml-plain`.

## XML
The file has three main sections: `header`, `geometry` and `frames`.

```xml
 <header version = "0.8">
     <agents>1</agents>
    <frameRate>8</frameRate>
 </header>
```

where

- `agents`: The total number of agents at the beginning of the simulation.
- `frameRate`: Divide the total number of frames by the framerate to obtain the overall evacuation time.

The geometry can be completely embedded within the trajectories or a reference to a file can be supplied.

```xml
 <geometry>
    <file location="corridor_geometry.xml"/>
 </geometry>

```

The coordinates of the trajectory are defined in the session `frames`


```xml
 <frame ID="0">
     <agent ID="1"	x="660.00"	y="333.00"	z="30.00"
    rA="17.94"	rB="24.94"	eO="-168.61" eC="0"/>
 </frame>
 <frame ID="1">
 <agent ID="1"	x="658.20"	y="332.86"	z="30.00"
 rA="31.29"	rB="23.87"	eO="-175.41" eC="54"/>
 </frame>
```


- `ID` the id of the pedestrians starting with 1.
- `x, y, z` the position of the agent.

- `rA, rB`  The shape which is defined by a circle (ellipse) drawn around a human-like figure.
  `radiusA` and `radiusB` are respectively the semi major axis and the semi minor axis of the ellipse,
  if the modeled pedestrians' shape is an ellipse.
  Otherwise, if it is a circle those values should be equal to the radius of the circle.
- `eO, eC` are the "ellipseOrientation" and the "ellipseColor".
  "ellipseOrientation" is the angle between the  major axis and the X-axis (zero for circle).
   A color can also be provided, for example for displaying change in velocity.
   The colours are in the range `[0=red, 255=green]` and define the proportion between
   the desired speed ($$v_0$$) and the instantaneous velocity.



A sample trajectory in the xml format is

```xml
 <?xml version="1.0" encoding="UTF-8"?>
 <trajectories>
    <header version = "0.5">
        <agents>1</agents>
        <frameRate>8</frameRate>
    </header>

    <geometry>
        <file location="corridor_geometry.xml"/>
    </geometry>

    <frame ID="0">
        <agent ID="1"	x="660.00"	y="333.00"	z="30.00"
        rA="17.94"	rB="24.94"	eO="-168.61"	eC="0"/>
    </frame>

    <frame ID="1">
        <agent ID="1"	x="658.20"	y="332.86"	z="30.00"
    rA="31.29"	rB="23.87"  eO="-175.41"	eC="54"/>
    </frame>
 </trajectories>
```

## TXT
The other format of the trajectory file is `plain`

A sample trajectory in the plain format is as follows:

```xml
#description: jpscore (0.8.4)
#count: 0
#framerate: 16.00
#geometry: geometry.xml
#sources: sources.xml
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
