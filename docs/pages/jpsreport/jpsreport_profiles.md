---
title:  Density profiles
keywords: analysis
tags: [jpsreport, jpsreport_tutorial]
sidebar: jupedsim_sidebar
folder: jpsreport
summary: This page shows how to get profiles of density, velocity and specific flow.
permalink: jpsreport_profiles.html
last_updated: Dec 21, 2019
published: false
---

{%include note.html content="this tutorial needs links to data"%}
 
Here is an example extracted from a T-Junction experiment: 

![T-junction]({{ site.baseurl }}/images/Figue4-4-3.png)

## Run jpsreport

In order to calculate the profiles it is mandatory to use [method D](jpsreport_method_D.html) or [method I](jpsreport_method_I.html).  
M
oreover, Set the parameter `enabled` of profiles to `true`.  
Set the resolution of the profile by initializing the  two parameters `grid_size_x`and `grid_size_y`, e.g.:  

```xml
 <method_D enabled="true"> 
   <profiles enabled="true" grid_size_x="0.20" grid_size_y="0.20"/> 
 </method_D> 
```

## (optional) Steady state

Determine the steady state of the experiment in the whole measurement region. 
In the folder script there is a script to fulfill this task semi-manually.  

## Produce the profiles

Run the python script `_Plot_profiles.py`, which is in the scripts folder in `jpsreport`.
