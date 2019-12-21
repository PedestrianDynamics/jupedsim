---
title: Sources file
keywords: simulation
tags: [jpscore, files, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_sources.html
summary: These brief instructions will help you get started quickly with the theme. The other topics in this help provide additional information and detail about working with other aspects of this theme and Jekyll.
last_updated: Dec 20, 2019
---


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
