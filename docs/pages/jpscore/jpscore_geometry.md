---
title: Geometry
keywords: simulation
tags: [jpscore, file]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_geometry.html
summary: This specification of the geometry is used in all JuPedSim modules. A geometry in JuPedSim is structured in rooms and subrooms. Rooms are connected by transitions and subrooms are connected by crossings.
last_updated: Dec 21, 2019
---

## Structure of the geometry

The main structure of the geometry file is as follows

![The main components defining a geometry]({{ site.baseurl }}/images/class-diagram.png)

```xml
<geometry>
    <rooms>
        <room>
            <subroom>
                <!-- walls of the subroom -->
                <obstacle>
                    <!-- obstecals inside the subroom -->
                </obstacle>
            </subroom>
            <crossings>
                <!-- doors between two subrooms -->
            </crossings>
        </room>
    </rooms>
    <transitions>
        <!-- doors between two rooms or a room and the outside -->
    </transitions>
</geometry>
```
***

## Geometry example



![Geometry example]({{ site.baseurl }}/images/geometry_example2.png)


![Geometry example 2]({{ site.baseurl }}/images/geometry_example.png)

{% capture download_note %}xml and dwg files for the example above can be downloaded [here]({{ site.base }}/files/2_mixedusage.zip){% endcapture %}

{%include note.html content=download_note%}


***

There are two ways to structure geometries for JuPedSim:

- Rooms
- Subrooms

Following picture shows a sample geometry with one room, two subrooms and one obstacle.

![A corner defined by one room and two subrooms]({{ site.baseurl }}/images/Subrooms.png)

`subrooms` and `rooms` are two different concepts to organize a geometry.
Basically, it is up to the user to organize its geometry "roomwise" or "subroomwise".


For instance the above mentioned geometry could also be organized using two rooms as follows:

![A corner defined by two different rooms]({{ site.baseurl }}/images/Rooms.png)

***

A geometry can be produces manually (for small scenarios) or with [jpseditor](jpseditor_introduction.html).
In both cases it is recommended to visualize the geometry once finished with [jpsvis](jpsvis_introduction.html).

The main components of a geometry are:

## Rooms
The geometry contains at least one room and one transition.
Each room has a unique `id`, an optional caption and at least one `subroom`.
Two rooms are separated by either walls or transitions.

```xml
 <rooms>
     <room id="0" caption="hall" >
 </rooms>
```

## Subrooms
Subrooms define the navigation mesh, i.e the walkable areas in the geometry.
Each `subroom` is bounded by at least one `crossing`.
Here a sample:

```xml
 <subroom id="1" class="stair" A_x="−1.2" B_y="0" C="0">
     <polygon caption="wall">
         <vertex px="0.0" py="1.0"/>
         <vertex px="−5.0" py="1.0"/>
     </polygon>
     <polygon caption="wall">
         <vertex px="0.0" py="3.0"/>
         <vertex px="−5.0" py="3.0"/>
     </polygon>
     <up px="−5.0" py="2"/>
     <down px="0.0" py="2"/>
 </subroom>
```

- `id` mandatory parameter, also referred by crossings and transitions.
- `class` optional parameter defining the type of the subroom. At the moment three classes are defined:
  - `floor`
  - `stairs` take additionally
    `<up px="-5.0" py="2" />` and   `<down px="0.0" py="2"/>`, which are
    used for visualisation purposes.
  - `escalator_up` and `escalator_down` require `<up px="-5.0" py="2" />` and `<down px="0.0" py="2"/>` to initialise their directions. Used by the router.
  - `platform` needs additionally walls of type `track-n`, see also [here](jpscore_trains.html).
- $$A\_x,\; B\_y,\text{and}\; C$$ are optional parameter for the explicit plane equation of the subroom,
   for the construction of a 3D environment and should be used to describe stairs.
   The plane equation is given by:  $$Z = Ax +By + C$$.
   For instance, if the stair goes through the following points:
   $$P_1(1,0,0),\; P_2 (0,1,0)\; \text{and}\; P_3(0,0,1)$$
   then the equation is given by: $$Z= -x -y +1$$.

- `polygon` describes the walls as a sequence of *two* vertexes.

To ease navigation, it is recommended to always use convex subrooms.
In the case the subroom is not convex, additional navigation lines might be required
or the floor field router should be used.

{%include important.html content="Every room has at least one subroom."%}



## Obstacles
One or more obstacles can also be defined within a `subroom`.


{%include warning.html content="Obstacles should not intersect with other geometry elements e.g. walls, crossings or transitions."%}


Sample obstacle in a subroom

```xml
 <obstacle id="0" caption="table" height="1.0" >
    <polygon>
        <vertex px="12" py="10"/>
        <vertex px="11" py="10"/>
    </polygon>
    <polygon>
        <vertex px="11" py="10"/>
        <vertex px="11" py="9"/>
    </polygon>
    <polygon>
        <vertex px="11" py="9"/>
        <vertex px="12" py="9"/>
    </polygon>
    <polygon>
        <vertex px="12" py="9"/>
        <vertex px="12" py="10"/>
    </polygon>
 </obstacle>
```

- `id`, mandatory unique identifier for this obstacle.
- `caption`, used in the visualisation.
- `height`, optional parameter, not used at the moment
- `polygon`, describing the obstacle as a sequence of vertex.

## Crossings

A `crossing` defines the connection between two subrooms inside the same room.
Unlike `transition`, they are always open.

A sample `crossing` between two subrooms

```xml
 <!-- virtual exits between subrooms -->
 <crossing id="0" subroom1_id="0" subroom2_id="1">
    <vertex px="10.0" py="6.0"/>
    <vertex px="10.0" py="4.0"/>
 </crossing>
```

- `id`, mandatory unique identifier for this `crossing`.
- `subroom1_id`, the first `subroom`
- `subroom2_id`,  the second `subroom` sharing this transition. The order is not important.
- `vertex`: define two ending points of the `crossing`.

## Transitions
A `transition` defines the connection between two rooms and is basically a door.
It can be close or open (see ["traffic constraints"](jpscore_inifile.html#traffic-constraints)).

An example transition between two rooms

```xml
 <!-- exits between rooms or to outside (room with index = -1) -->
 <transition id="1" caption="main exit" type="emergency"
 room1_id="0" subroom1_id="1" room2_id="-1" subroom2_id="-1">
     <vertex px="15.0" py="-5.0"/>
     <vertex px="17.0" py="-5.0"/>
     <file>transitions.xml</file>
 </transition>
```

- `id`, mandatory unique identifier.
  The `id` is also used to close or open the door in the ["traffic constraints"](jpscore_inifile.html#traffic-constraints) section of the inifile.
- `caption`, optional, used in the visualisation.
- `type`, optional.
- `room1_id`, the first room sharing this transition. The order is not important.
- `subroom1_id`, the first `subroom` sharing this transition located in `room_1`.
- `room2_id`, the second room sharing this transition.
  The order is not important.
  If there is no second room (meaning this transition is connected to the outside), then use `-1`.
- `subroom2_id`, the second `subroom` sharing this transition located in `room_2`. The order is not important.
  If there is no second `subroom` (meaning this transition is connected to the outside), then use `-1`.
- `vertex`: define two ending points of the `transition`.
- `file`: file containing further transitions. See [transitions.xml](jpscore_transitions.html)


