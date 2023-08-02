---
title:  Method B
keywords: analysis
tags: [jpsreport, measurement]
sidebar: jupedsim_sidebar
folder: jpsreport
summary: This method measures the mean value of velocity and density over space and time.
permalink: jpsreport_method_B.html
last_updated: Dec 23, 2019
---


 The spatial mean velocity and density are
calculated by taking a segment  $$\Delta x$$ in a corridor as the
measurement area. 

![Method B: Illustration of the measurement interval.]({{ site.baseurl }}/images/jpsreport_Method_B.png)


The velocity  $$\langle v \rangle_i$$ of each person
is defined as the length  $$\Delta x$$ of the measurement area divided
by the time he or she needs to cross the area:

$$\langle v \rangle_i=\frac{\Delta x}{t_\text{out}-t_\text{in}},$$

where  $$t_\text{in}$$ and  $$t_\text{out}$$ are the times a person enters 
and exits the measurement area, respectively.

The density  $$\rho_i$$ for each person $$i$$ is calculated as:

$$\langle \rho \rangle_i=\frac{1}{t_\text{out}-t_\text{in}}\cdot\int_{t_\text{in}}^{t_\text{out}} \frac{N^\prime(t)}{b_\text{cor}\cdot\Delta x}dt,$$

where  $$b_\text{cor}$$ is the width of the measurement area while  $$N^\prime(t)$$ is the number of person in this area at a time  $$t$$.
