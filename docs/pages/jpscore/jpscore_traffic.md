---
title: Traffic constraints and door statuses
keywords: simulation
tags: [jpscore, file, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_traffic.html
summary: Properties of doors can be defined before the start of a simulation. Doors have different statuses, which may change during the simulation.
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

## Sample

```xml
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <JPScore project="JPS-Project" version="0.8">
  <traffic_constraints>
    <doors>
      <door trans_id="2" state="open" />
      <door trans_id="3" state="open" />
      <door trans_id="4" state="open" />
      <door trans_id="5" state="open" />
      <door trans_id="0" state="open" dn="10" outflow="2" max_agents="200"/>
    </doors>
  </traffic_constraints>
 </JPScore>
```

## Changes of door's status

Doors have three statuses:

* _open_
* _close_
* _temp_close_

They can all be set and changed by an event. 

Here are the rules governing the changes from one status to another:

* Events have priority before the door-flow regulation.
* When max_agents are reached the door status is _close_.
* Doors that are closed by max_agents can be opened again by the event "reset".
* If the door status is _temp_close_ the agents should wait in front of the door until it opens again.
* If a door status is _close_, the agents should look for another door and **not** wait in front of the closed door. And if there is no other open door, the agents should wait inside the room.


![Changes of door's status]({{ site.baseurl }}/images/door_states.png)
