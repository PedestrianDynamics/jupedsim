---
title: Event file
keywords: simulation
tags: [jpscore, files]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_events.html
summary: These brief instructions will help you get started quickly with the theme. The other topics in this help provide additional information and detail about working with other aspects of this theme and Jekyll.
last_updated: Dec 20, 2019
---

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

- `time`: time of event
- `state` can be close or open
- `id`: unique id of the specific door (transition) as defined in the geometry file. See [geometry](2016-11-02-geometry.html).
