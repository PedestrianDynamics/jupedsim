---
title: Direction strategies
keywords: simulation
tags: [jpscore, model]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_direction.html
summary: Once the targeted door is defined in a previous step by the router, the pedestrian aims at a specific point on the door. This point defines the desired direction of the pedestrian. This pages shows different methods how to define this point.
last_updated: Dec 31, 2019
---

## Introduction

The chosen model the direction strategy should be specified in the [inifile](jpscore_inifile.html) as follows

```xml
 <exit_crossing_strategy>num</exit_crossing_strategy>
```

with *num* a positive integer.

The majority of the strategies define how a pedestrian crosses a line $$L = [P_1, P_2]$$. Possible values are:

## Strategy 1
The direction of the pedestrian is towards the middle of $$L$$ ($$\frac{P_1+P2}{2}$$)
![Strategy 1: Goal towards the center of the door]({{ site.baseurl }}/images/strategy1.png)

## Strategy 2
The direction is given by the nearest point on $$L$$ to the position of the pedestrian.
   $$L$$ is shorten by the shoulder width of the pedestrians (+10 cm).
  
  
![Strategy 2: Goal towards the center if pedestrian is out of range of the door.]({{ site.baseurl }}/images/strategy2.png)

## Strategy 3 
Choose the nearest point on the *line* $$L$$. 
If the nearest point of the pedestrian on the line $$L$$ is outside the segment, then chose the middle point as target.
Otherwise the nearest point is chosen.
![Strategy 3: Goal towards the nearest point on the door.]({{ site.baseurl }}/images/strategy3.png)

## Strategy 8 
{% include note.html content="For this to work properly the [floor field router](jpscore_routing.html#floorfield-router) has to be used"%}

For more details see this [talk](https://fz-juelich.sciebo.de/index.php/s/s1ORGTUssCsHDHC) and the corresponding [thesis](https://fz-juelich.sciebo.de/index.php/s/VFnUCH2gtz1mSoL).

This strategy uses a floor field collection for each room.

The following parameters of an enhanced floor field can be changed:
- `delta_h`: discretization/stepsize of grid-points used by the floor field
- `wall_avoid_distance`: below this wall-distance, the floor field will show a wall-repulsive character, directing agents away from the wall
- `use_wall_avoidance`: {true, false} switch to turn on/off the enhancement of the floor field

{%include tip.html content="It's recommended to choose a reasonable value of the `wall_avoid_distance` (shoulder width of an average  pedestrian) in order to not steer pedestrians too close to walls"%}

![Floorfield: Changing the minimal distance to the walls to guarantee a "safe" route]({{ site.baseurl }}/images/transformFF.png)

Usage example:
```xml
<model_parameters>
    <exit_crossing_strategy>8</exit_crossing_strategy>
    <delta_h>0.0625</delta_h>
    <wall_avoid_distance>0.8</wall_avoid_distance>
    <use_wall_avoidance>true</use_wall_avoidance>
</model_parameters>
```


Here the floor fields are smaller but cannot steer to targets in a different room.
The router **must** provide intermediate targets for every agent, the target being in the same room.

{%include warning.html content="The projection of the room onto the ($$x,\, y$$)-plane must be non-overlapping!"%}
{%include warning.html content="do not use in multi-storage buildings!"%}

## Strategy 9
This strategy uses a floor field collection for each subroom. (__broken__)

{% include note.html content="For this to work properly the [floor field router](jpscore_routing.html#floorfield-router) has to be used"%}

Thus the floor fields are again smaller but cannot steer to targets in a different subroom.
The router **must** provide intermediate targets for every agent, that target being in the same subroom.

{%include warning.html content="The projection of the room onto the ($$x,\, y$$)-plane must be non-overlapping!"%}
{%include warning.html content="do not use in multi-storage buildings!"%}



---



[#Chraibi2011]: http://aimsciences.org/journals/displayPaper.jsp?paperID=6440 "Chraibi el al. Force-based models of pedestrian dynamics.  Pages: 425 - 442, Volume 6, Issue 3, September 2011"
