---
title: Schedules
keywords: simulation
tags: [jpscore, file, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_schedule.html
summary: Schedules are events that can be used to group doors and trigger events groupwise at certain points of time.
last_updated: Dec 31, 2019
---

## Definition

In general a definition of a valid schedule is composed of two parts:

- **groups**: collection of doors
- and **times** of events that apply to the different groups.

In detail the following attributes are defined:

-  `groups`: groups with all door, which should open or close at the same time.
   -  a group has an unique `id`.
   -  member defined by a unique id `t_id`  of the specific door as defined in the geometry file. See [geometry](jpscore_geometry.html).
   - `max_agents`: sets the maximum number of agents which can pass a transition until it is closed.

-  `times`: schedule times for every group.
   - `reset` (default "false"): Instead of opening the transition, the counter of passed pedestrians is reset to 0 and the door opens again. The closing time is not needed in this case.
   -  `group_id`: unique id of the specific group of doors as defined above in `groups`.
   -  `closing_time`: time after which the doors of the group will be closed after the doors were opened.
   -  `t`: time when the doors of the group will be open again. The door will be closed again at `t + closing_time`.

## Sample

In this example a schedule defining four different groups 

```xml
 <?xml version="1.0" encoding="UTF-8" ?>
 <JPScore project="JPS-Project" version="0.6">
    <groups>
        <group id="0">
            <member t_id="0"/>
            <member t_id="2"/>
            <member t_id="4"/>
        </group>
        <group id="1">
            <member t_id="1"/>
            <member t_id="3"/>
            <member t_id="5"/>
        </group>
        <group id="2">
            <member t_id="7"/>
            <member t_id="9"/>
        </group>
        <group id="3">
            <member t_id="6"/>
            <member t_id="8"/>
        </group>
    </groups>

    <times>
        <time group_id="0" closing_time="20">
            <t t="30"/>
            <t t="80"/>
        </time>
        <time group_id="1" closing_time="20">
            <t t="35"/>
            <t t="85"/>
        </time>
        <time group_id="2" closing_time="5">
            <t t="2"/>
            <t t="50"/>
            <t t="90"/>
        </time>
        <time group_id="3" closing_time="10">
            <t t="10"/>
            <t t="60"/>
            <t t="90"/>
        </time>
    </times>
</JPScore>
```

In this example the doors of group 3 will **open** at 10, 60 and 90

and **closed** at 30, 80 and 110.

{% include note.html content="all times are in seconds."%}
