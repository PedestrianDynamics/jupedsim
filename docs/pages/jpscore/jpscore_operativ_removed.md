---
title: Operational models (removed)
keywords: simulation
tags: [jpscore, model, removed]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_operativ_removed.html
summary: Thede models have been removed and should be reintegrated at some point in the future
last_updated: Dec 21, 2019
published: true
---

{%include note.html content="this page is unpublished and is related to [operational models](jpscore_operativ.html)"%}


## Gompertz model
[Gompertz Model][#gompertz] is a force-based model.

Usage:

```xml
<model operational_model_id="2" description="gompertz">
```



## Wall-avoidance model

[Wall-avoidance model][#ModelGraf2015] is a velocity-based model. The Wall-Avoidance Model focuses on valid pedestrian positions. The interaction of agents with walls takes precedence over the agent-to-agent interaction. There are two key aspects:

* In the vicinity to walls, agents take on a different behaviour, slowing them down (parameter: `slowdowndistance`)

 * Agents follow special floorfields, directing them to the targets/goals, which will have them avoid walls if possible (free space)

Valid exit strategies are {6, 8, 9}. Please see details below.


(Sample) Usage:

```xml
 <model operational_model_id="4" description="gradnav">
   <model_parameters>
    <solver>euler</solver>
    <stepsize>0.01</stepsize>
    <exit_crossing_strategy>9</exit_crossing_strategy>
    <floorfield delta_h="0.0625" wall_avoid_distance="0.4"
        use_wall_avoidance="true" />
    <linkedcells enabled="true" cell_size="4.2" />
    <force_ped nu="3" b="1.0" c="3.0" />
    <force_wall nu="1" b="0.70" c="3.0" />
    <anti_clipping slow_down_distance=".2" />
   </model_parameters>
   <agent_parameters agent_parameter_id="0">
    <v0 mu="1.5" sigma="0.0" />
    <bmax mu="0.25" sigma="0.001" />
    <bmin mu="0.20" sigma="0.001" />
    <amin mu="0.18" sigma="0.001" />
    <tau mu="0.5" sigma="0.001" />
    <atau mu="0.23" sigma="0.001" />
  </agent_parameters>
 </model>
```

### Parameters

- `<exit_crossing_strategy>[6, 8, 9]</exit_crossing_strategy>`
  The strategies 6, 8 and 9 differ only in the way the floorfield is created:
  - 6: one floorfield over all geometry (building); only in 2D geometries; directing every agent to the closest exit
  - 8: multiple floorfield-objects (one for every __room__); each object can create a floor field on the fly to a target line (or vector of lines) within the room; working in multi-floor-buildings; requires a router that provides intermediate targets in the same room
  - 9: (__recommended__) multiple floorfield-objects (one for every __subroom__);
       each object can create a floor field on the fly to a target line (or vector of lines)
       within the same subroom; working in multi-floor-buildings;
       requires a router that provides intermediate targets in the same subroom;

- `<floorfield delta_h="0.0625" wall_avoid_distance="0.4" use_wall_avoidance="true" />`
     - The parameters define:
          - __delta_h__: discretization/stepsize of grid-points used by the floor field
          - __wall_avoid_distance__: below this wall-distance, the floor field will show a wall-repulsive character, directing agents away from the wall
          - __use_wall_avoidance__: {true, false} switch to turn on/off the enhancement of the floor field

          <img src="{{ site.baseurl }}/images/transformFF.png" width="500" height="500" />

- `<linkedcells enabled="true" cell_size="4.2" />`
  - range in which other pedestrians are considered neighbours and can influence the current agent. This value defines cell-size of the cell-grid.

## Generalized Centrifugal Force Model with lateral swaying

The [Generalized Centrifugal Force Model with lateral swaying][#Krausz] is mostly identical to the GCFM Model,
but instead of a variable semi-axis $$b$$ of the ellipse simulating the pedestrian, pedestrians perform an oscillation perpendicular to their direction of motion.
As a consequence the parameter `Bmax` is ignored.

Usage:

```xml
 <model operational_model_id="5" description="krausz">
```

Four Parameters can be passed to control the lateral swaying, for example:

`<sway ampA="-0.14" ampB="0.21" freqA="0.44" freqB="0.35" />`

- `ampA` and `ampB` determine the amplitude of the oscillation according to the linear relation
   $$A = \texttt{ampA} \cdot \| v_i \| + \texttt{ampB}$$.

- `freqA` and `freqB` determine the frequency of the oscillation according to
   $$f = \texttt{freqA} \cdot \| v_i \| + \texttt{freqB}$$.

Setting `ampA` and `ampB` to 0 disables lateral swaying. If not specified, the empirical values given in [Krausz, 2012][#Krausz] are used, that is:

- `ampA` = -0.14, `ampB` = 0.21 and
- `freqA` = 0.44, `freqB` = 0.25.



[#gompertz]: https://www.researchgate.net/profile/Gregor_Laemmel/publication/289377829_JuPedSim_an_open_framework_for_simulating_and_analyzing_the_dynamics_of_pedestrians/links/568c1ce708aeb488ea2faf1c/JuPedSim-an-open-framework-for-simulating-and-analyzing-the-dynamics-of-pedestrians.pdf?origin=publication_detail  "Kemloh Wagoum, Armel Ulrich, Mohcine Chraibi, Jun Zhang and Gregor Lämmel. JuPedSim: An Open Framework for Simulating and Analyzing the Dynamics of Pedestrians. In 3rd Conference of Transportation Research Group of India, 2015."

[#ModelGraf2015]: https://fz-juelich.sciebo.de/index.php/s/VFnUCH2gtz1mSoL "Arne, Graf Master thesis"

[#Krausz]: https://link.springer.com/chapter/10.1007/978-3-319-02447-9_61
