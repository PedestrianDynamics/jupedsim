---
title:  Method D
keywords: analysis
tags: [jpsreport, measurement]
sidebar: jupedsim_sidebar
folder: jpsreport
summary: this method calculates the density based on Voronoi diagrams, which are a special kind of decomposition of a metric space determined by distances to a specified discrete set of objects in the space. 
permalink: jpsreport_method_D.html
last_updated: Dec 23, 2019
---

At any time the positions of the pedestrians
can be represented as a set of points, from which the Voronoi
diagram can be generated. 

The Voronoi cell area,  $$A_i$$, for each person  $$i$$ can be obtained. 

![Method D: Illustration of the Voronoi diagrams]({{ site.baseurl }}/images/jpsreport_Method_D.png)


Then, the density and velocity distribution of the space $$\rho_{xy} $$  and  $$v_{xy}$$ can be defined as

$$\rho_{xy} = 1/A_i \quad \text{and} \quad v_{xy}={v_i(t)}\qquad \mbox{if} (x,y) \in A_i,$$

where  $$v_i(t)$$ is the instantaneous velocity of each person. 

The Voronoi density for the
measurement area is defined as:

$$\langle \rho \rangle_v=\frac{\iint{\rho_{xy}dxdy}}{b_\text{cor}\cdot\Delta x}.$$

Similarly, the Voronoi velocity is defined as:

$$\langle v \rangle_v=\frac{\iint{v_{xy}dxdy}}{b_\text{cor}\cdot\Delta x}.$$
