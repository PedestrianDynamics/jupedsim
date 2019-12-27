---
title: Sample for a file with traffic constraints
keywords: simulation
tags: [jpscore, file, simulation]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_traffic.html
summary: These brief instructions will help you get started quickly with the theme. The other topics in this help provide additional information and detail about working with other aspects of this theme and Jekyll.
last_updated: Dec 20, 2019
---



```xml
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <JPScore project="JPS-Project" version="0.8">
  <traffic_constraints>
    <doors> <!-- doors states are: close or open -->
      <door trans_id="2" caption="NaN" state="open" />
      <door trans_id="3" caption="NaN" state="open" />
      <door trans_id="4" caption="NaN" state="open" />
      <door trans_id="5" caption="NaN" state="open" />
      <door trans_id="0" caption="NaN" state="open" dn="10" outflow="2" max_agents="200"/>
    </doors>
  </traffic_constraints>
 </JPScore>
```
