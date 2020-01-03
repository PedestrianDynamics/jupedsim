---
title: Operational models
keywords: simulation
tags: [jpscore, model]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_operativ.html
summary: Several operational models are implemented in jpscore. An operational model defines how pedestrians interact with each other and with their environment.
last_updated: Dec 31, 2019
---

## Introduction
In the definition of agent's properties it is mandatory to precise the number of the model to be used e.g.:

```xml
 <agents operational_model_id="n">
```
The definition of any model parameter is composed of two different
sections:

- **model_parameters**: Model specific parameter. See below in the different model sections.
- **agent_parameters**: These parameter are mainly specific for the shape of pedestrians
    or other pedestrian properties like desired speed, reaction time etc.

## Model parameters (in general)
- `<solver>euler</solver>`
     - The solver for the ODE. Only *Euler*. No other options.
- `<stepsize>0.001</stepsize>`:
     - The time step for the solver. This should be choosed with care. For force-based model it is recommended to take a value between $$ 10^{-2} $$ and $$10^{-3}$$ s.
       For first-order models, a value of 0.05 s should be OK.
       A larger time step leads to faster simulations, however it is too risky and can lead to
numerical instabilities, collisions and overlapping among pedestrians.
     - Unit: s

- `<exit_crossing_strategy>3</exit_crossing_strategy>`
     - Positive values in $$[1, 9]$$. See [Direction strategies](jpscore_direction.html) for the definition of the strategies.

- `<linkedcells enabled="true" cell_size="2"/>`
     - Defines the size of the cells. This is important to get the neighbors of a pedestrians, which
       are all pedestrians within the eight neighboring cells. Larger cells, lead so slower simulations, since
       more pedestrian-pedestrian interactions need to be calculated.
     - Unit: m


## Agent's parameter (in general)
The *agent parameters* are mostly identical for all models. Exceptions will be mentioned explicitly.

The parameters that can be specified in this section are Gauss distributed (default value are given).

### Desired speed
- `<v0 mu="1.2" sigma="0.0" />`
    - Desired speed
    - Unit: m/s
- `<v0_upstairs mu="0.6" sigma="0.167" />`
    - Desired speed upstairs
    - Unit: m/s
- `<v0_downstairs mu="0.6" sigma="0.188" />`
    - Desired speed downstairs
    - Unit: m/s
- `<v0_idle_escalator_upstairs mu="0.6" sigma="0.0" />`
    - Speed of idle escalators upstairs
    - Unit: m/s
- `<v0_idle_escalator_downstairs mu="0.6" sigma="0.0" />`
    - Speed of idle escalators downstairs
    - Unit: m/s


The reduced speed on stairs (up) is according to Tab 1 in [Burghardt2014][#Burghardt2014].

| Handbook | Speed Stair Up |
|----------|----------------|
| PM       | 0.63 m/s       |
| WM       | 0.61 m/s       |
| NM       | 0.8 m/s        |
| FM       | 0.55 m/s       |


### Shape of pedestrians
Pedestrians are modeled as ellipses with two semi-axes: $$a$$ and $$b$$, where

$$
a= a_{min} + a_{\tau}v,
$$

and

$$
b = b_{max} - (b_{max}-b_{min})\frac{v}{v^0}.
$$

$$v$$ is the peed of a pedestrian.

- `<bmax mu="0.15" sigma="0.0" />`
    - Maximal length of the shoulder semi-axis
    - Unit: m
- `<bmin mu="0.15" sigma="0.0" />`
    - Minimal length of the shoulder semi-axis
    - Unit: m
- `<amin mu="0.15" sigma="0.0" />`
    - Minimal length of the movement semi-axis. This is the case when $$v=0$$.
    - Unit: m
- `<atau mu="0." sigma="0.0" />`
    - (Linear) speed-dependency of the movement semi-axis
    - Unit: s


## Generalized Centrifugal Force Model
[Generalized Centrifugal Force Model][#GCFM] [(preprint)][#GCFM_PREPRINT] is a force-based model.

Usage:

```xml
 <model operational_model_id="1" description="gcfm">
```



## Collision-free Speed Model
[Collision-free speed model][#Tordeux2015] is a velocity-based model. See also this [talk](https://www.dropbox.com/s/fj1xud5ap2aq59o/Tordeux2015_Talk.pdf) for more details about the model.

Usage:

```xml
 <model operational_model_id="3" description="Tordeux2015">
```

### Model parameters (Tordeux2015)
Besides the options defined in [Model parameters](#model-parameters-in-general) the following options are necessary for this model:

- `<force_ped  a="5" D="0.2"/>`
     - The influence of other pedestrians is triggered by $$a$$ and $$D$$ where $$a$$ is the strength if the interaction and $$D$$ gives its range. The naming may be misleading, since the model is **not** force-based, but velocity-based.
     - Unit: m
- `<force_wall a="5" D="0.02"/>`:
     - The influence of  walls is triggered by $$a$$ and $$D$$ where $$a$$ is the strength if the interaction and $$D$$ gives its range. A larger value of $$D$$ may lead to blockades, especially when passing narrow bottlenecks.
     - Unit: m

The names of the aforementioned parameters might be misleading, since the model is *not* force-based. The naming will be changed in the future.

### Agent parameters (Tordeux2015)
Actually, this model assumes circular pedestrian's shape, therefore the parameter for the semi-axes should be chosen, such that circles with constant radius can be obtained.
For example:

```xml
 <bmax mu="0.15" sigma="0.0" />
 <bmin mu="0.15" sigma="0.0" />
 <amin mu="0.15" sigma="0.0" />
 <atau mu="0." sigma="0.0" />
```
This defines circles with radius 15 cm.

Other parameters in this section are:

- `<tau mu="0.5" sigma="0.0" />`
    - Reaction time. This constant is used in the driving force of the force-based forces. Small $$\rightarrow$$ instantaneous acceleration.
    - Unit: s
- `<T mu="1" sigma="0.0" />`
    - Specific parameter for model 3 (Tordeux2015). Defines the slope of the speed function.


In summary the relevant section for this model could look like:

```xml
 <model operational_model_id="3" description="Tordeux2015">
    <model_parameters>
        <solver>euler</solver>
        <stepsize>0.05</stepsize>
        <exit_crossing_strategy>3</exit_crossing_strategy>
        <linkedcells enabled="true" cell_size="2"/>
        <force_ped  a="5" D="0.2"/>
        <force_wall a="5" D="0.02"/>
    </model_parameters>
    <agent_parameters agent_parameter_id="1">
        <v0 mu="1.34" sigma="0.0" />
        <v0_upstairs mu="0.668" sigma="0.167" />
        <v0_downstairs mu="0.750" sigma="0.188" />
        <v0_idle_escalator_upstairs mu="0.5" sigma="0.0" />
        <v0_idle_escalator_downstairs mu="0.5" sigma="0.0" />
        <bmax mu="0.15" sigma="0.0" />
        <bmin mu="0.15" sigma="0.0" />
        <amin mu="0.15" sigma="0.0" />
        <atau mu="0." sigma="0.0" />
        <tau mu="0.5" sigma="0.0" />
        <T mu="1" sigma="0.0" />
    </agent_parameters>
 </model>
```

{%include note.html content="The recommended values are by no means universal, and may/should be calibrated to fit your scenario."%}



[#GCFM]: http://journals.aps.org/pre/abstract/10.1103/PhysRevE.82.046111 "Mohcine Chraibi, Armin Seyfried, and Andreas Schadschneider Phys. Rev. E 82, 046111"
[#GCFM_PREPRINT]: https://arxiv.org/abs/1008.4297 "Preprint ArXiv"
[#Tordeux2015]: http://arxiv.org/abs/1512.05597  "Tordeux, Antoine, Chraibi, Mohcine and Seyfried, Armin, Collision-free speed model for pedestrian dynamics. In Traffic and Granular Flow  '15, to appear."

[#Burghardt2014]: http://link.springer.com/chapter/10.1007%2F978-3-319-02447-9_27 "Burghardt, Sebastian and Seyfried, Armin and Klingsch, Wolfram. Fundamental diagram of stairs: Critical review and topographical measurements. Pedestrian and Evacuation Dynamics 2012"
