---
title: Way finding
keywords: simulation
tags: [jpscore, model]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_routing.html
summary: In order to navigate in (complex) buildings a router is needed to assign pedestrians exits. Different algorithms are implemented and explained briefly in this page
last_updated: Dec 21, 2019
---

## Floorfield Router

The floorfield-router uses floorfields
to calculate the distances among the doors of the same `subroom`.
The major difference to any other router is, that it does __not__
need convex subrooms/rooms any longer. There is no need for adding
helplines.

It fills an adjacency matrix and calculates global-shortest paths via the
Floyd-Warshall algorithm.

The floorfield-router will give intermediate targets within the`subroom`
of each agent. It works in combination with exit strategies 8 and 9.[^str_8_9]


{%include warning.html  content="<strong>3D geometries:  </strong> To use it successfully in multi-storage buildings, the user must provide a geometry file, where stair-cases (or any other structur), which connects two floors/levels **must** be a separate room. Further, that room **must** only connect two levels. Rooms stretching over more than 2 levels are not valid."%}


If there are two points with the same ($$x, y$$)-coordinates, which differ
only in the $$z$$-coordinate, the router will face problems, thus we defined
the restriction above. That should avoid any such cases.

The floorfield router provides one mode: ```ff_global_shortest```


{%include important.html  content="If you use a router, which allows non-convex subrooms/rooms, you should use an exit-strategy,
which also allows non-convex subrooms/rooms. Exit-strategies 8 and 9 will work best with the floorfield router."%}


Following snippet is a definition example of the routing information:

```xml
<route_choice_models>
  <router router_id="1" description="ff_global_shortest">
    <parameters>
        <write_VTK_files>true</write_VTK_files>
    </parameters>
  </router>
</route_choice_models>
```

## Global shortest path

At the beginning of the simulation, the Dijkstra algorithm is used to
build a network which is then cached and used through the simulation
life time.

Detailed information about the aforementioned models
are presented in: [KemlohWagoum2012a][#KemlohWagoum2012a]

Following snippet is a definition example of the routing information:

```xml
<route_choice_models>
  <router router_id="1" description="global_shortest">
    <parameters>
      <navigation_lines file="routing.xml" />
    </parameters>
  </router>
</route_choice_models>
```
## Global quickest path

Route choice based on the estimated travel time.
This router takes traffic into consideration.

```xml
<route_choice_models>
  <router router_id="1" description="quickest">
    <parameters>
    </parameters>
  </router>
</route_choice_models>
```



[^str_8_9]: If convex subrooms are provided, any exit strategy will work.
      In these special cases, global router will be faster in computation time.


[#KemlohWagoum2012a]: dx.doi.org/10.1142%2FS0219525912500294 "Kemloh et al "Modeling the dynamic route choice of pedestrians to assess the criticality of building evacuation. Advances in Complex Systems. 2012"
