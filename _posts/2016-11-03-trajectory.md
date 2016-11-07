---
layout: post
title: Trajectory file
subtitle: 
#bigimg: /img/jupedsim_small.png
permalink: 2016-11-03-trajectory.html
---



The results of the simulation are written to files or streamed to a network socket.

Possible formats are:

- `xml-plain` which is the default xml format
- `plain` a flat format (just numbers)

Note that if you are using the streaming mode or want to visualize the trajectories with `jpsvis`, 
the format should be `xml-plain`.

## xml-plain
The file has three main sections: `header`, `geometry` and `frames`.

```xml
<header version = "0.8">
    <agents>1</agents>
	<frameRate>8</frameRate>
</header>
```

where

- `agents`: The total number of agents at the beginning of the simulation.
- frameRate`: Divide the total number of frames by the framerate to obtain the overall evacuation time.

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
   the desired speed ($v_0$) and the instantaneous velocity.



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
	rA="31.29"	rB="23.87" 	eO="-175.41"	eC="54"/>
	</frame>
</trajectories>
```

## plain-text
The other format of the trajectory file is `plain-text`

A sample trajectory in the plain format is as follows:

```xml
				#description: simulation
				#framerate: 16
				#geometry: /home/sim/corridor.xml
				#ID: the agent ID
				#FR: the current frame
				#X,Y,Z: the agents coordinates in metres
								
				#ID	FR	X		Y		Z
				1	0	28.21	131.57	0.00
				2	0	38.41	133.42	0.00
				1	1	28.21	131.57	0.00
				2	1	38.41	133.42	0.00
				1	2	28.24	131.57	0.00
				2	2	38.44	133.42	0.00
				1	3	28.29	131.57	0.00
				2	3	38.49	133.42	0.00
				1	4	28.36	131.57	0.00
				2	4	38.56	133.42	0.00
				1	5	28.44	131.57	0.00
				2	5	38.64	133.42	0.00
				1	6	28.54	131.57	0.00
				2	6	38.74	133.42	0.00
				1	7	28.65	131.57	0.00
				2	7	38.85	133.42	0.00
				1	8	28.77	131.57	0.00
```				

