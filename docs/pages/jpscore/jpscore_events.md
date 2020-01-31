---
title: Events
keywords: simulation
tags: [jpscore, file]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_events.html
summary: Definition of the event's file. Events occure in time and trigger certain actions on doors and crossings.
last_updated: Jan 25, 2020
---

## Definition

Following properties define an `event`:

- `id` (int): unique id of the specific door (transition) as defined in the geometry file. See [geometry](jpscore_geometry.html).
- `time` (double): time of an event
- `state` can be `close`, `temp_close`, `open` or `reset`: defines what event will be applied to the door


## Sample

Example of an event file: 

```xml
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <JPScore project="JPS-Project" version="0.8">
    <events update_frequency="2" update_radius="100" agents_color_by_knowledge="true">
        <event time="30" state="close" id="5" />
        <event time="50" state="close" id="4" />
        <event time="70" state="open" id="4" />
        <event time="71" state="close" id="2" />
        <event time="80" state="open" id="5" />
    </events>
</JPScore>
```
