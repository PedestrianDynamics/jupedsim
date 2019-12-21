---
title: Schedule
keywords: simulation
tags: [jpscore, files, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_schedule.html
summary: These brief instructions will help you get started quickly with the theme. The other topics in this help provide additional information and detail about working with other aspects of this theme and Jekyll.
last_updated: Dec 20, 2019
---


Example for a schedule: 

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

-  `groups` groups with all door, which shoul open or close at the same time.
-  `group id` unique id to identify the specific group.
-  `member_id` unique id of the specific door as defined in the geometry file. See [geometry](2016-11-02-geometry.html).
-  `times` schedule times for every group.
-  `group_id` unique id of the specific group of doors as defined above in `groups`.
-  `closing_time` time after which the doors of the group will be closed after the doors were opened.
-  `t` time when the doors of the group will be open again.      
    `t`+ `closing_time`= opening time
