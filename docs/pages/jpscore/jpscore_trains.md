---
title: Interface to trains in jpscore
keywords: simulation
tags: [jpscore, files, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_trains.html
summary: These brief instructions will help you get started quickly with the theme. The other topics in this help provide additional information and detail about working with other aspects of this theme and Jekyll.
last_updated: Dec 20, 2019
---


## Train constraints

In the inifile the following section should be defined, where two files are specified:
- [Train timetable](#train-timetable)
- [Train types](#train-types)

```
 <train_constraints>
   <train_time_table>ttt.xml</train_time_table>
   <train_types>train_types.xml</train_types>
 </train_constraints>
```

## Geometry definition

Following constraints are to be respected when creating the geometry of a platform:

- Define a subroom of *class*  `Platform`
- A *Platform* can have more than one track.
- A *Platform* is closed: Meaning it has whether transitions nor crossings.
- `tracks` can be defined with walls of type `track-n`, where $$n$$ is the number of the track.

<div class="alert alert-info">
  <strong>Important! </strong>Note: Every track *should* be within one single subroom.
</div>



Here is an example of a platform with two tracks

```
 <room id="0" caption="station">
    <subroom id="1" caption="dummy" class="Platform" A_x="0" B_y="0" C_z="0">
      <polygon caption="wall" type="track-2">
        <vertex px="0" py="0"/>
        <vertex px="0" py="3"/>
      </polygon>
      <polygon caption="wall" type="track-2">
        <vertex px="10" py="0"/>
        <vertex px="10" py="3"/>
      </polygon>
      <polygon caption="wall" type="track-2">
        <vertex px="0" py="0"/>
        <vertex px="10" py="0"/>
      </polygon>
      <polygon caption="wall" type="track-1">
        <vertex px="0" py="3"/>
        <vertex px="2" py="4"/>
      </polygon>
      <polygon caption="wall" type="track-1">
        <vertex px="7" py="4"/>
        <vertex px="8.00377" py="3.79962"/>
      </polygon>
      <polygon caption="wall" type="track-1">
        <vertex px="8.00377" py="3.79962"/>
        <vertex px="9.00063" py="3.53957"/>
      </polygon>
    </subroom>
    <crossings/>
 </room>
```

## Train timetable

Here, train's coordinates are **relative** to the point `track_start`.

```
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <train_time_table>
  <train id="1" type="RE" room_id="0"
      track_start_x="-10" track_start_y="10" track_end_x="0" track_end_y="10"
      train_start_x="1" train_start_y="3"
      train_end_x="7" train_end_y="3"
      arrival_time="5" departure_time="15">
  </train>
  <train id="2" type="ICE" room_id="0"
      track_start_x="0" track_start_y="10" track_end_x="10" track_end_y="10"
      train_start_x="2" train_start_y="3"
      train_end_x="12" train_end_y="3"
      arrival_time="30" departure_time="50">
  </train>
 </train_time_table>
```
## Train types

Door's coordinates are **relative** to the point `train_start` as defined in the [Train timetable](#train-timetable) file.

![coordinates]({{ site.baseurl }}/img/coordinates.png)

So the global coordinates of the train's door `[A,B]` will be translated as follows

$$
A_{global} = track\_start + train\_start + A_{local},
$$

$$
B_{global} = track\_start + train\_start + B_{local}
$$



The number of agents in a train are calculated every time step as the sum of
all agents passing through the train's door.

When this number exceeds the `agents_max` parameter, all train's doors are closed.

Note: The parameter `length` is not used yet. Maybe it will be helpful for sanity checks.
Actually, the length of the train is internally calculated from `train_start` and `train_end`.

```
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <train_type>
  <train type="RE" agents_max="10" length="5">
   <door id="1">
      <vertex px="0.5" py="0.0" />
      <vertex px="2" py="0.0" />
    </door>
    <door id="2" outflow="3">
      <vertex px="4" py="0.0" />
      <vertex px="5" py="0.0" />
    </door>
  </train>
  <train type="ICE" agents_max="800" length="4">
    <door id="1">
      <vertex px="1" py="0" />
      <vertex px="2" py="0" />
    </door>
    <door id="2">
      <vertex px="4" py="0" />
      <vertex px="5" py="0" />
    </door>
    </train>
 </train_type>
```

## Algorithm

Given a `platform` with at least one `track`, the geometry will be changes every time a train arrives/leaves.

This will be done by projecting the train's doors on the track. With "projection" we mean along the orthogonal direction to the door.

![platform]({{ site.baseurl }}/img/platform.png)

Every projection point is mapped to the corresponding wall on the track.

```
 std::pair<Point, Wall >
```

Since every train's door `[A, B]`, corresponds to two intersection points `[T1, T2]` with pairs `<T1, W1>` and `<T2, W2>`, we have three cases:

- case 1: `W1 == W2`
- case 2: W1 and W2 share one point
- case 3: W1 and W2 are disjoint, meaning several track's walls are between.

To remove the walls between the points T1 and T2, following actions are performed according to each case:
- Remove from the `building` walls with end points between T1 and T2
- Add to the `building` a new transition  [T1, T2]. This transition will have an id starting from 10000. This number is incremented every time a new train transition is added.

- Add  to the `building` new walls to close the gaps.

![trainIntersection]({{ site.baseurl }}/img/trainIntersection.png)
