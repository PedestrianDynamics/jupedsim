---
title: Goals
keywords: simulation
tags: [jpscore, file, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_goals.html
summary: The desired direction of a pedestrian is defined following different algorithms.
last_updated: Dec 20, 2019
---


```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<JPScore project="JPS-Project" version="0.8">
  <goals>
    <goal id="1" final="true" caption="goal1" x_min="3.6" x_max="5.2" y_min="-4" y_max="-3">
      <polygon>
        <vertex px="3.6" py="-3.0" />
        <vertex px="3.6" py="-4.0" />
        <vertex px="5.2" py="-4.0" />
        <vertex px="5.2" py="-3.0" />
        <vertex px="3.6" py="-3.0" />
      </polygon>
    </goal>
    <goal id="4" final="true" caption="goal4" x_min="-4" x_max="-3" y_min="2.8" y_max="6.4">
      <polygon>
        <vertex px="-3.0" py="2.8" />
        <vertex px="-4.0" py="2.8" />
        <vertex px="-4.0" py="6.4" />
        <vertex px="-3.0" py="6.4" />
        <vertex px="-3.0" py="2.8" />
      </polygon>
    </goal>
    <waiting_area caption="wa1" id="5" waiting_time="20" max_peds="10" is_open="true" room_id="0" subroom_id="1" global_timer="false" >
      <polygon>
        <vertex px="11" py="1" />
        <vertex px="14" py="1" />
        <vertex px="14" py="4" />
        <vertex px="11" py="4" />
        <vertex px="11" py="1" />
      </polygon>
      <next_wa id="2" p="0.75"/>
      <next_wa id="3" p="0.25"/>
    </waiting_area>
  </goals>
</JPScore>
```
