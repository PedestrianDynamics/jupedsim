---
title:  Method J
keywords: analysis
tags: [jpsreport, measurement]
sidebar: jupedsim_sidebar
folder: jpsreport
summary: This method calculates the density based on Voronoi diagrams, which are a special kind of decomposition of a metric space determined by distances to a specified discrete set of objects in the space. At any time the positions of the pedestrians can be represented as a set of points, from which the Voronoi diagram can be generated.
permalink: jpsreport_method_J.html
last_updated: Dec 20, 2019
---

The density distribution of the space $$\rho_{xy} $$ can be defined as

$$\rho_{xy} = 1/A_i \qquad  \mbox{if} \quad (x,y) \in A_i,$$

The Voronoi density for the measurement area is defined as:

$$\langle \rho \rangle_v=\frac{\iint{\rho_{xy}dxdy}}{\Delta x \cdot \Delta y}.$$


For a given trajectory $$\vec{x_i}(t)$$, the velocity $$v_i(t)$$ is calculated by use of the displacement of pedestrian  $$i$$ in a small time interval $$\Delta t^\prime$$ around  $$t$$:

$$v_i(t)=\frac{\vec{x_i}(t+\Delta t^\prime/2)-\vec{x_i}(t-\Delta t^\prime/2))}{\Delta t^\prime}.$$

The spatial mean velocity is the average of the instantaneous velocities $$v_i(t)$$ for all pedestrians $$N$$ who have an intersection with the measurement area at the time $$t$$:

$$\langle v \rangle_{\Delta x}=\frac{1}{N_{(x,y) \in A_i}}\sum_{i=1}^{N_{(x,y) \in A_i}}{v_i(t)}.$$
