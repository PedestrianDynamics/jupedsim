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

- `id`: id of the source
- `caption`: caption
- `frequency`: time in seconds of generation of pedestrians (default = 1).
- `N_create`: How many agents to create at once (default = 1).
- `percent`: percent of `N_create` to generate (default = 1).
- `rate`: rate of generation of agents (in seconds).
- `time_min`, `time_max`: Time lifespan of the source.
- `agents_max`: maximal number of agents produced by that source.
- `group_id`: group id of the agents. This `id` **should match** a predefined group in the section [Agents_distribution](jpscore_inifile.html#agents_distribution).
- `time`: time of appearance of agent with id `agent_id`. Here `agents_max=1`.
- `startX`, `startY`: Distribute one agent at a fix position.
- `x_min`, `x_max`, `y_min`, `y_max`: Bounding box for generation of agents.
- `greedy` (default `false`): returns a Voronoi vertex randomly with respect to weights proportional to squared distances.
   For vertexes $$v_i$$ and distances $$d_i$$ to their surrounding seeds
   calculate the probabilities $$p_i$$ as

   $$p_i= \frac{d_i^2}{\sum_j^n d_j^2}.$$

   If this attribute is set to `true`, the greedy approach is used.
   That means new agents will be placed on the vertex with the biggest distance to the surrounding seeds.
- `file`: a file containing further sources. See [sources.xml](jpscore_sources.html)


## Sample

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
