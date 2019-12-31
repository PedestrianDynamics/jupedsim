---
title: Sample for a file with traffic constraints
keywords: simulation
tags: [jpscore, file, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_traffic.html
summary: Properties of doors can be defined before the start of a simulation.
last_updated: Dec 20, 2019
---

## Definitions

Following properties of `transitions` can be defined before the start of the simulation:

- `trans_id`: unique id of a transition. Should match an existing id in the [geometry file](jpscore_geometry.html#transitions)
- `caption`: optional parameter defining the caption of the door.
- `state` defines the state of the door. Options are `close`, `temp_close` or `open`.
- `dn`: number of agents to pass the door before triggering the process of flow regulation.
- `outflow`: Max flow through door. Door's state will be changed adequately.
- `max_agents`: Max agents that can pass door. Door will be closed permanently

{%include note.html content="the door's state *close* is permanent, meaning that the door is closed until the end of the simulation, whereas *temp_close* means that the door is closed temporally and may be open at some time during the simulation"%}


## Sample

```xml
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <JPScore project="JPS-Project" version="0.8">
  <traffic_constraints>
    <doors>
      <door trans_id="2" caption="NaN" state="open" />
      <door trans_id="3" caption="NaN" state="open" />
      <door trans_id="4" caption="NaN" state="open" />
      <door trans_id="5" caption="NaN" state="open" />
      <door trans_id="0" caption="NaN" state="open" dn="10" outflow="2" max_agents="200"/>
    </doors>
  </traffic_constraints>
 </JPScore>
```
