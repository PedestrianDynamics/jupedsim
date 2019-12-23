---
title:  Method A
keywords: analysis
tags: [jpsreport, measurement]
sidebar: jupedsim_sidebar
folder: jpsreport
summary: This method calculates the mean value of flow and density over time. 
permalink: jpsreport_method_A.html
last_updated: Dec 23, 2019
---

![Method A: Illustration of the measurement line.]({{ site.baseurl }}/images/jpsreport_Method_A.png)

A reference line is taken and studied over a
fixed period of time $$\Delta {t}$$.  
Using this method we can obtain the pedestrian flow $$J$$ and the 
velocity $$v_i$$ of each pedestrian passing the reference line directly. 
Thus, the flow over time $$\langle J \rangle_{\Delta t}$$ and the time mean 
velocity $$\langle v \rangle_{\Delta t}$$ 
can be calculated as

$$\langle J \rangle_{\Delta t}=\frac{N^{\Delta t}}{t_N^{\Delta t} - t_1^{\Delta t}}\qquad \text{and} \qquad \langle v \rangle_{\Delta t}=\frac{1}{N^{\Delta t}}\sum_{i=1}^{N^{\Delta t}} v_i(t),$$

where $$N^{\Delta t}$$ is the number of persons passing the 
reference line during the time interval  $$\Delta {t}$$. 

$$t_N^{\Delta {t}}$$ and  $$t_1^{\Delta {t}}$$ are the times when the first and last pedestrians pass the location in $$\Delta {t}$$.

{%include note.html content="Note: this time period can be different from  $$\Delta {t}$$."%}

The time mean velocity  $$\langle v \rangle_{\Delta t}$$  is defined as the mean value of the instantaneous velocities  $$N^{\Delta t}$$ pedestrians. 

$$v_i(t)$$ is calculated by use of the displacement of pedestrian  $$i$$ in a small time interval  $$\Delta t^\prime$$ around  $$t$$:

$$v_i(t)=\frac{\vec{x_i}(t+\Delta t^\prime/2)-\vec{x_i}(t-\Delta t^\prime/2))}{\Delta t^\prime}.$$


