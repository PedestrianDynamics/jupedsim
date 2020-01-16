---
title: Modeling the change of the desired speed
keywords: simulation
tags: [jpscore, model]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_desired_speed.html
summary: Pedestrians may have a different desired speed on a stair than on a horizontal plan.  Therefore, it is necessary to calculate a "smooth" transition in the desired speed, when pedestrians move on planes with a different inclination. In this way "jumpy" changes in the desired speed are avoided.
last_updated: Dec 25, 2019
---
{% include note.html content="This modelling of the desired speed in the transition area of planes and stairs is not validated, since experimental data are missing."%}


## Definitions
Assume the following scenario, with two horizontal planes and a stair, where
$$z_0<z_1$$ and the inclination of the stair $$\alpha$$.

![Speed curve in the transition area between levels and stairs]({{ site.baseurl }}/images/desired_speed.png)


The agent has a desired speed on the horizontal plane $$v^0_{\text{horizonal}}$$ and a *different* desired speed on the stair $$v^0_{\text{stair}}$$.

Given a stair connecting two hirozontal floors, we define the following functions:

$$
f(z) = \frac{2}{1 + \exp\Big(-c\cdot \alpha (z-z_1)^2)\Big)} - 1,
$$

and

$$
g(z) =  \frac{2}{1 + \exp\Big(-c\cdot \alpha ((z-z_0)^2)\Big)} - 1.
$$

![Increasing function $$f(z)$$ and decreasing function $$g(z)]({{ site.baseurl }}/images/desired_speed_f_g.png)

## Function of the desired speed

Taking the previously introduced quantities into consideration, we can define the desired speed od the agent with respect to its $$z-$$component as

$$
v^0(z) = v^0_{\text{horizonal}}\cdot\Big(1 − f(z)\cdot g(z)\Big)   + v^0_{\text{stair}}\cdot f(z)\cdot g(z),
$$


$$c$$ is a constant.

The following figure shows the changes of the desired speed with repsect to the inclination of the stair $$\alpha$$. The steepter the inclination of the stair, the faster is the change of the desired speed.

![Transition area of levels and stairs]({{ site.baseurl }}/images/desired_speed2.png)


{%include note.html content="The value of *c* should be chosen so that the function grows fast (but smooth) from 0 to 1. However, in force-based models the speed is adapted exponentially from zero to the desired speed.  Therefore, the parameter tau must be taken into consideration."%}




