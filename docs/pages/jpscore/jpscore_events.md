---
title: Events
keywords: simulation
tags: [jpscore, file]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_events.html
summary: Definition of the event's file. Events occure in time and trigger certain actions on doors and crossings.
last_updated: Dec 31, 2019
---

## Definition

Following properties define an `event`:

- `id`: unique id of the specific door (transition) as defined in the geometry file. See [geometry](jpscore_geometry.html).
- `update_frequency`:
- `update_radius`:
- `agents_color_by_knowledge`:
- `time`: time of an event
- `state` can be `close`, `temp_close` or `open`


## Sample

Example of an event file: 

```xml
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <JPScore project="JPS-Project" version="0.8">
    <events update_frequency="2" update_radius="100" agents_color_by_knowledge="true">
        <event time="30" type="door" state="close" id="5" />
        <event time="50" type="door" state="close" id="4" />
        <event time="70" type="door" state="open" id="4" />
        <event time="71" type="door" state="close" id="2" />
        <event time="80" type="door" state="open" id="5" />
    </events>
</JPScore>
```
