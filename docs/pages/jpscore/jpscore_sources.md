---
title: Sources
keywords: simulation
tags: [jpscore, file, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_sources.html
summary: Sources are used to inject agents in a running simulation.
last_updated: Dec 31, 2019
---

## Parameter of a source

- `id` (int): id of the source
- `caption` (str): caption (default: "no caption")
- `frequency` (int): time in seconds of generation of pedestrians (default: 1).
- `N_create` (int): how many agents to create at once (default: 1).
- `percent` (float): percent of `N_create` to generate (default: 1.0). A number between 0 and 1.

{%include note.html content="percent should be at least equal to $$\lceil \frac{\text{frequency}}{\text{rate}}\rceil$$ in order to generate `N_create` agents."%}

- `rate` (int): rate of generation of `percent`$$\times$$`N_create` agents. (default: `frequency`)
- `time_min` (float), `time_max`: Time lifespan of the source.
- `agents_max` (int): maximal number of agents produced by that source. (default: 10)
- `group_id`: group id of the agents. This `id` **should match** a predefined group in the section [Agents_distribution](jpscore_inifile.html#agents_distribution).
- `time` (float): time of appearance of agent. Should be used together with `agent_id`. If used then `agents_max=1` and `frequency=1`.
- `startX`, `startY` (float, float): Distribute one agent at a fix position.
- `x_min`, `x_max`, `y_min`, `y_max` (float, float, float, float): Bounding box for generation of agents.
- `greedy` (bool): returns a Voronoi vertex randomly with respect to weights proportional to squared distances. (default: `false`).
   For vertexes $$v_i$$ and distances $$d_i$$ to their surrounding seeds
   calculate the probabilities $$p_i$$ as

   $$p_i= \frac{d_i^2}{\sum_j^n d_j^2}.$$

   If this attribute is set to `true`, the greedy approach is used.
   That means new agents will be placed on the vertex with the biggest distance to the surrounding seeds.
- `file`: a file containing further sources. See [sample](jpscore_sources.html#file-sample)



## Example 1

Starting from time 2 seconds, $$percent\times N\_create=2$$ are generated with a rate of 4 seconds.

```xml
<source id="1" group_id="1"
    time_min="2"
    time_max="30" 
    frequency="10"
    rate="4"
    percent="0.2" 
    N_create="10"
    agents_max="300"/>
```

Note that a cycle of generation starts at every `frequency` mark (red ticks).
Therefore, if `percent` is too low, the chances are that the number of created agents in one cycle is smaller than `N_create`.

![Generation of agents with percent=0.2.]({{ site.baseurl }}/images/sources_p02.gif)

## Example 2

Starting from time 2 seconds, $$percent\times N\_create=5$$ are generated with a rate of 4 seconds.

```xml
<source id="1" group_id="1"
    time_min="2"
    time_max="30" 
    frequency="10"
    rate="4"
    percent="0.5" 
    N_create="10"
    agents_max="300"/>
```

Here, `N_create` is generated in every cycle, although in comparison to example 1 only 2 generation steps are needed (instead of 3).


![Generation of agents with percent=0.2.]({{ site.baseurl }}/images/sources_p05.gif)

## File sample

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<JPScore project="JPS-Project" version="0.8">
  <agents_sources><!-- frequency in persons/seconds -->
    <source id="11" caption="new-source 2" time_min="5" time_max="30" frequency="7" N_create="10" agents_max="300"
            group_id="1"  x_min="6" x_max="9" y_min="6" y_max="9" percent="0.5" rate="2"  greedy="true"/>
    <source id="12" caption="new-source 3" time_min="5" time_max="30" frequency="9" N_create="10" agents_max="300"
            group_id="2"  x_min="6" x_max="9" y_min="0" y_max="3" percent="0.5" rate="2"  greedy="true"/>
    <source id="13" caption="new-source 4" time_min="5" time_max="30" frequency="11" N_create="10" agents_max="300"
            group_id="3"  x_min="0" x_max="3" y_min="6" y_max="9" percent="0.5" rate="2"  greedy="true"/>
    <source id="14" caption="nlow-source 5" time_min="5" time_max="30" frequency="11" N_create="10" agents_max="300"
            group_id="4"  x_min="3.5" x_max="5.5" y_min="3.5" y_max="5.5" percent="0.5" rate="2"  greedy="true"/>
  </agents_sources>
</JPScore>
```
