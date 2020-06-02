---
title: Trains
keywords: simulation
tags: [jpscore, file, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_trains.html
summary: The interaction with trains is modeled with an event-based deleting and creating of doors. When a train arrives on the platform, doors are created and again deleted when the train departs.
last_updated: Jun 02, 2020
---


## Train constraints
Information regarding trains are organized in two different files:
- [Train timetable](#train-timetable): This file defines arrival and departure times of trains.
- [Train types](#train-types): In this file types of trains are defined.

## Geometry definition
For using train in the simulation, tracks need to be defined in the geometry. 
A track is a consecutive line sequence, marked by the type `type="track"`.
For each track, one point needs to marked as starting point with `start="true"`. 

### Example
```xml
<room id="1" caption="floor">
    <subroom id="2" caption="Room 2" A_x="0" B_y="0" C_z="0">
        ...
        <polygon caption="wall" type="track" track_id="1">
            <vertex px="-10" py="10" start="true" />
            <vertex px="-5" py="10" />
        </polygon>
        <polygon caption="wall" type="track" track_id="1">
            <vertex px="-5" py="10" />
            <vertex px="0" py="10" />
        </polygon>
        ...
    </subroom>
</room>
```

## Train types
### Definition
A train is defined through the following information: 

- type (string): Unique key to identify train
- length (int): length of the train
- agents_max (int): maximal number of passengers
- door: 
    - id (int): id of the train door
    - distance (float): distance to train start
    - width (float): width of the door
    
{%include note.html content="The parameter `length` is not used yet. In future it will be used for sanity checks and for avoiding overlapping trains."%}

### Example
```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<train_type>
    <train type="RE" agents_max="15" length="9">
        <door id="1" distance="1" width="1"/>
        <door id="3" distance="5" width="1"/>
    </train>

    <train type="ICE" agents_max="20" length="23">
        <door id="1" distance="3" width="2"/>
        <door id="2" distance="10" width="3"/>
        <door id="3" distance="18" width="2"/>
    </train>
</train_type>
```

![Schematic overview of train type definition.]({{ site.baseurl }}/images/traintype.png)

### Capacity of a train

The number of agents in a train is calculated every time step as the sum of
all agents passing through the train's doors.

When this number exceeds the `agents_max` parameter, all train's doors are closed.

## Train timetable
### Definition 
A train is defined through the following information: 
- id (int): id of the train
- type (string): identifier of the train defined with the train types
- track_id (int): id of the track the train arrives
- train_offset (float): offset of the train to the track start
- from_end (bool): false: train starts at track start, true: train starts at track end
- arrival_time (float): time the train arrives
- departure_time (float): time the train departs

### Example

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<train_time_table>
    <train id="1" type="RE"
           track_id="2"
           train_offset="0"
           arrival_time="5" departure_time="15">
    </train>

    <train id="2" type="ICE"
           track_id="3"
           train_offset="0" from_end="true"
           arrival_time="20" departure_time="30">
    </train>
</train_time_table>
```

## Geometry changes due to trains
For realizing trains in the simulation, doors need to be added or removed dynamically during the simulation. 
 
### Calculation of train door positions
The following accounts when calculating the train door coordinates:
- Distance between train start and door start is calculated along the lines
- Distance between door start and door end is calculated as direct connection
- Distance and width are persevered 

### Splitting of the track walls
#### Case 1: Door on one single wall element
![Door on one single wall element.]({{ site.baseurl }}/images/trainCase1.png)

#### Case 2: Door on two neighboring wall elements
![Door on two neighboring wall elements.]({{ site.baseurl }}/images/trainCase2.png)

#### Case 3: Door on more than two wall elements
![Door on more than two wall elements.]({{ site.baseurl }}/images/trainCase3.png)

{%include note.html content="Please be aware that depending on the curvature of the platform edges a significant area of
 platform might be not accessible. Pedestrian inside this area will be removed from the simulation and report as 
 erroneous."%}
