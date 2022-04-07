---
title: jpscore inifile
keywords: simulation
tags: [jpscore, file]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_inifile.html
summary:  The project file where the settings for a jpscore simulation are defined. Within this file properties of pedestrians, model parameters, etc can be given.
last_updated: April 6, 2022
---

# Basic Structure

The basic structure of the inifile is as follows:

```xml

<JuPedSim project="ProjectTitle">
    <header>
        <!-- General simulation information -->
    </header>

    <agents>
        <!-- Agents' distribution and sources -->
    </agents>

    <route_choice_models>
        <!-- Definition of routing strategies -->
    </route_choice_models>

    <operational_models>
        <!-- Interaction of agents and walls -->
    </operational_models>

    <!-- OPTIONAL settings -->

    <routing>
        <!-- Parameters to adapt routing via goals-->
    </routing>

    <traffic_constraints>
        <!-- Traffic information to adapt the flow -->
    </traffic_constraints>

    <train_constraints>
        <!-- Parameters for simulation of train arrival/departure -->
    </train_constraints>
</JuPedSim>
```

# Header

## Required Header Settings

The following elements must be defined in the header:

- `<seed>seed</seed>`

  Set the  `seed` value of the random number generator to `s`. If the tag is empty or missing, the current
  time (`time(NULL)`) is used i.e. random initial conditions.

- `<max_sim_time>t</max_sim_time>`
  the maximal simulation time in seconds.

- `<geometry>geometry.xml</geometry>`
  The name and location of the geometry file. All file locations are relative to the actual location of the project
  file. See [specification](jpscore_geometry.html) of the geometry format.

- `<output path="output_directory" />`
  The name and location where the results of the simulation should be stored. The program creates a folder with the
  given name and copies all needed files of the simulation to this folder. Additionally the file names in the ini and
  geometry file are updated. Resulting in a folder with the results and everything to reproduce a specific simulation.
  If no output path is given the folder is named `results`.

- The `trajectory` file with its location to configure output as described [here](jpscore_trajectory#txt). The settings
  are listed below.

### Trajectory settings

- `fps`: defines the frame rate per second for the trajectories.
- `precision`: define the precision of the trajectories' points. Value can be between 1 and 6. (default: 2).
- `color_mode`: coloring agents in the trajectories. Options are:
  - `velocity` (default): color is proportional to speed (slow --> red).
  - `spotlight`
  - `group`: color by group
  - `router`
  - `final_goal`
  - `intermediate_goal`
- `file location` defines the location of the trajectories. All paths are relative to the location of the project file.
- `optional_output`: possibility to give additional output. Set any of these values to `TRUE` (not case sensitive) to
  get the according value. `FALSE`, any other value and ignoring the option will lead to no output.
  - `speed`: speed of the pedestrian
  - `velocity`: x,y components of the pedestrians's velocity
  - `final_goal`: id of the final goal the pedestrian is heading to
  - `intermediate_goal`: id of the current goal the pedestrian is heading to (usually a door)
  - `desired_direction`: x,y components of the pedestrians's desired directions - `group`: group of the pedestrian
  - `router`: router used by the pedestrian (id accoriding to ini-file)
  - `spotlight`: pedestrian is highlighted.

```xml

<trajectories fps="8" precision="2" color_mode="velocity">
    <file location="trajectories.txt"/>
    <optional_output speed="FALSE" velocity="TRUE" final_goal="NoOutputWrongValue" intermediate_goal="false"
                     desired_direction="true" group="TRUE" router="TRUE" spotlight="TRUE"/>
</trajectories>
```

## Optional Header Settings

The following elements can be defined optionally in the header.

- `<show_statistics>true</show_statistics>` Show different aggregate statistics e.g. the usage of the doors. (default:
  false)
- `<events_file>events.xml</events_file>` The name and location of the event file. Events can be used to open or close
  doors at a certain point of time. For details see below.
-
- `<schedule_file>schedule.xml</schedule_file>`
  The name and location of the schedule file. Schedules can be used to group doors and open or close this groups of door
  at certain points of time. For details see below.

### Events

Events occur in time and trigger certain actions on doors and crossings. Following properties define an `event`:

- `id` (int): unique id of the specific door (transition) as defined in the geometry file.
  See [geometry](jpscore_geometry.html).
- `time` (double): time of an event
- `state` can be `close`, `temp_close`, `open` or `reset`: defines what event will be applied to the door

Example of an event file:

```xml
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<JPScore project="JPS-Project" version="0.8">
    <events update_frequency="2" update_radius="100" agents_color_by_knowledge="true">
        <event time="30" state="close" id="5"/>
        <event time="50" state="close" id="4"/>
        <event time="70" state="open" id="4"/>
        <event time="71" state="close" id="2"/>
        <event time="80" state="open" id="5"/>
    </events>
</JPScore>
```

### Schedule

Schedules are events that can be used to group doors and trigger events groupwise at certain points of time. In general
a definition of a valid schedule is composed of two parts:

- **groups**: collection of doors
- and **times** of events that apply to the different groups.

In detail the following attributes are defined:

- `groups`: groups with all doors, which should open or close at the same time.
  - a group has an unique `id`.
  - member defined by a unique id `t_id`  of the specific door as defined in the geometry file.
    See [geometry](jpscore_geometry.html).
  - `max_agents`: sets the maximum number of agents which can pass a transition until it is closed.

- `times`: schedule times for every group.
  - `reset` (default "false"): Instead of opening the transition, the counter of passed pedestrians is reset to 0 and
    the door opens again at time `t`.
  - `group_id`: unique id of the specific group of doors as defined above in `groups`.
  - `closing_time`: time after which the doors of the group will be closed after the doors were opened.
  - `t`: time when the doors of the group will be open again. The door will be closed again at `t + closing_time` or
    when max_agents is reached.

In this example a schedule defining four different groups

```xml
 <?xml version="1.0" encoding="UTF-8" ?>
<JPScore project="JPS-Project" version="0.6">
    <groups>
        <group id="0">
            <member t_id="0"/>
            <member t_id="2"/>
            <member t_id="4"/>
        </group>
        <group id="1">
            <member t_id="1"/>
            <member t_id="3"/>
            <member t_id="5"/>
        </group>
        <group id="2" max_agents="20">
            <member t_id="7"/>
            <member t_id="9"/>
        </group>
        <group id="3" max_agents="25">
            <member t_id="6"/>
            <member t_id="8"/>
        </group>
    </groups>

    <times>
        <time group_id="0" closing_time="20">
            <t t="30"/>
            <t t="80"/>
        </time>
        <time group_id="1" closing_time="20">
            <t t="35"/>
            <t t="85"/>
        </time>
        <time group_id="2" closing_time="5" reset="true">
            <t t="2"/>
            <t t="50"/>
            <t t="90"/>
        </time>
        <time group_id="3" closing_time="10" reset="true">
            <t t="10"/>
            <t t="60"/>
            <t t="90"/>
        </time>
    </times>
</JPScore>
```

In this example the doors of group 3 will **open** at 10, 60 and 90

and **closed** at 20, 70 and 100.

{% include note.html content="all times are in seconds."%}

# Agents

There are two ways to distribute agents for a simulation:

- [random distribution](#agents_distribution) in a specific area *before* the simulation starts.
- distribution by means of [sources](#sources) *during* the simulation.

```xml

<agents>
    <agents_distribution>
        <group group_id="1" room_id="0" number="10"/>
        <group group_id="2" room_id="0" subroom_id="0" number="10" goal_id="" router_id="1"/>
    </agents_distribution>
    <agents_sources>
        <source id="1" frequency="2" agents_max="10" group_id="1" caption="caption" greedy="true"/>
        <source id="2" time="10" agent_id="50" group_id="1" caption="caption" greedy="true"/>
        <source id="10" caption="new-source" time_min="5" time_max="30" frequency="5" N_create="10" agents_max="300"
                group_id="0" x_min="0" x_max="3" y_min="0" y_max="3" percent="0.5" rate="2" greedy="true"/>
        <file>sources.xml</file>
    </agents_sources>
</agents>
```

## Agents_distribution

Above is an example how to define agent's characteristics with different number of attributes, which are as follows:

- `group_id`: mandatory parameter defining the unique id of that group.

- `number`: mandatory parameter defining the number of agents to distribute.

- `room_id`: mandatory parameter defining the room where the agents should be randomly distributed.

- `subroom_id`: defines the id of the subroom where the agents should be distributed. If omitted then the agents are
  homogeneously distributed in the room.

- `goal_id`: should be one of the `id`s defined in the section  [goals](#goals). If omitted or is `-1` then the shortest
  exit to the outside is chosen by the agent.

- `router_id`: defines the route choice model to be used. See [routing documentation](#routing) of available routers.

- `agent_parameter_id`: choose a set of parameters for the [operational models](#operational-models).

- `x_min`, `x_max`, `y_min` and `y_max`: define a bounding box where agents should be distributed.

- `startX`, `startY`: define the initial coordinate of the agents. This might be useful for testing/debugging. Note that
  these two options are only considered if `number=1`.

- `pre_movement_mean` and `pre_movement_sigma`: premovement time is Gauss-distributed $$\mathcal{N}(\mu, \sigma^2)$$.

- Risk tolerance can be Gauss-distributed, or beta-distributed. If not specified then it is defined as $$\mathcal{N}(1,
  0)$$:

  - `risk_tolerance_mean` and `risk_tolerance_sigma`: $$\mathcal{N}(\mu, \sigma^2)$$.

  - `risk_tolerance_alpha` and `risk_tolerance_beta`: $$Beta(\alpha, \beta)$$.

- `patience`:  this parameter influences the route choice behavior when using the quickest path router. It basically
  defines how long a pedestrian stays in jams before attempting a rerouting.

- `age`: not yet used by any of the [operational](#operational-models) models.

- `gender`: not yet used.

- `height`: not yet used.
-

## Sources

Besides distributing agents randomly before the simulation starts, it is possible to define sources in order to "inject"
new agents in the system during the simulation.

An example of usage:

- Busses are coming every 10 min (600 seconds).
- Every bus transports 100 pedestrians.
- When the bus stops, every 2 seconds 10 pedestrians leave the bus.
- 3 Buses at max.

```xml

<source id="10" frequency="600" N_create="100" agents_max="300"
        percent="0.1" rate="2"/>
```

### Parameters

- `id` (int): id of the source
- `caption` (str): caption (default: "no caption")
- `frequency` (int): time in seconds of generation of pedestrians (default: 1).
- `N_create` (int): how many agents to create at once (default: 1).
- `percent` (float): percent of `N_create` to generate (default: 1.0). A number between 0 and 1.

{%include note.html content="percent should be at least equal to $$\lceil \frac{\text{frequency}}{\text{rate}}\rceil$$
in order to generate `N_create` agents."%}

- `rate` (int): rate of generation of `percent`$$\times$$`N_create` agents. (default: `frequency`)
- `time_min` (float), `time_max`: Time lifespan of the source.
- `agents_max` (int): maximal number of agents produced by that source. (default: 10)
- `group_id`: group id of the agents. This `id` **should match** a predefined group in the
  section [Agents_distribution](#agents_distribution).
- `time` (float): time of appearance of agent. Should be used together with `agent_id`. If used then `agents_max=1`
  and `frequency=1`.
- `startX`, `startY` (float, float): Distribute one agent at a fix position.
- `x_min`, `x_max`, `y_min`, `y_max` (float, float, float, float): Bounding box for generation of agents.
- `greedy` (bool): returns a Voronoi vertex randomly with respect to weights proportional to squared distances. (
  default: `false`). For vertexes $$v_i$$ and distances $$d_i$$ to their surrounding seeds calculate the probabilities
  $$p_i$$ as

  $$p_i= \frac{d_i^2}{\sum_j^n d_j^2}.$$

  If this attribute is set to `true`, the greedy approach is used. That means new agents will be placed on the vertex
  with the biggest distance to the surrounding seeds.
- `file`: a file containing further sources. See [sample](#file-sample)

### Example 1

Starting from time 2 seconds, $$percent\times N\_create=2$$ are generated with a rate of 4 seconds.

```xml

<source id="1" group_id="1"
        time_min="2"
        time_max="30"
        frequency="10"
        rate="4"
        percent="0.2"
        N_create="10"
        agents_max="300"/>
```

Note that a cycle of generation starts at every `frequency` mark (red ticks). Therefore, if `percent` is too low, the
chances are that the number of created agents in one cycle is smaller than `N_create`.

![Generation of agents with percent=0.2.]({{ site.baseurl }}/images/sources_p02.gif)

### Example 2

Starting from time 2 seconds, $$percent\times N\_create=5$$ are generated with a rate of 4 seconds.

```xml

<source id="1" group_id="1"
        time_min="2"
        time_max="30"
        frequency="10"
        rate="4"
        percent="0.5"
        N_create="10"
        agents_max="300"/>
```

Here, `N_create` is generated in every cycle, although in comparison to example 1 only 2 generation steps are needed (
instead of 3).

![Generation of agents with percent=0.2.]({{ site.baseurl }}/images/sources_p05.gif)

### File sample

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<JPScore project="JPS-Project" version="0.8">
    <agents_sources><!-- frequency in persons/seconds -->
        <source id="11" caption="new-source 2" time_min="5" time_max="30" frequency="7" N_create="10" agents_max="300"
                group_id="1" x_min="6" x_max="9" y_min="6" y_max="9" percent="0.5" rate="2" greedy="true"/>
        <source id="12" caption="new-source 3" time_min="5" time_max="30" frequency="9" N_create="10" agents_max="300"
                group_id="2" x_min="6" x_max="9" y_min="0" y_max="3" percent="0.5" rate="2" greedy="true"/>
        <source id="13" caption="new-source 4" time_min="5" time_max="30" frequency="11" N_create="10" agents_max="300"
                group_id="3" x_min="0" x_max="3" y_min="6" y_max="9" percent="0.5" rate="2" greedy="true"/>
        <source id="14" caption="nlow-source 5" time_min="5" time_max="30" frequency="11" N_create="10" agents_max="300"
                group_id="4" x_min="3.5" x_max="5.5" y_min="3.5" y_max="5.5" percent="0.5" rate="2" greedy="true"/>
    </agents_sources>
</JPScore>
```

# Router

In order to navigate in complex geometries a router is required to assign pedestrians and their goals. Different
algorithms are implemented and explained in the following.

## Floorfield Router

The floorfield-router uses floorfields to calculate the distances among the doors of the same `subroom`. The major
difference to any other router is, that it does __not__ need convex subrooms/rooms any longer. There is no need for
adding helplines.

It fills an adjacency matrix and calculates global-shortest paths via the Floyd-Warshall algorithm.

The floorfield-router will give intermediate targets within the`subroom`
of each agent. It works in combination with exit strategies 8 and 9.[^str_8_9]

{%include warning.html content="<strong>3D geometries:  </strong> To use it successfully in multi-storage buildings, the
user must provide a geometry file, where stair-cases (or any other structur), which connects two floors/levels **must**
be a separate room. Further, that room **must** only connect two levels. Rooms stretching over more than 2 levels are
not valid."%}

If there are two points with the same ($$x, y$$)-coordinates, which differ only in the $$z$$-coordinate, the router will
face problems, thus we defined the restriction above. That should avoid any such cases.

The floorfield router provides one mode: ```ff_global_shortest```

{%include important.html content="If you use a router, which allows non-convex subrooms/rooms, you should use an
exit-strategy, which also allows non-convex subrooms/rooms. Exit-strategies 8 and 9 will work best with the floorfield
router."%}

Following snippet is a definition example of the routing information:

```xml

<route_choice_models>
    <router router_id="1" description="ff_global_shortest">
        <parameters>
            <write_VTK_files>true</write_VTK_files>
        </parameters>
    </router>
</route_choice_models>
```

## Global shortest path

At the beginning of the simulation, the Dijkstra algorithm is used to build a network which is then cached and used
through the simulation life time.

Detailed information about the aforementioned models are presented in [KemlohWagoum2012a][#KemlohWagoum2012a]

Following snippet is a definition example of the routing information:

```xml

<route_choice_models>
    <router router_id="1" description="global_shortest">
        <parameters>
            <navigation_lines file="routing.xml"/>
        </parameters>
    </router>
</route_choice_models>
```

## Global quickest path

Route choice based on the estimated travel time. This router takes traffic into consideration.

```xml

<route_choice_models>
    <router router_id="1" description="quickest">
        <parameters>
        </parameters>
    </router>
</route_choice_models>
```

[^str_8_9]: If convex subrooms are provided, any exit strategy will work. In these special cases, global router will be
faster in computation time.

[#KemlohWagoum2012a]: https://doi.org/10.1142/S0219525912500294 "Kemloh et al "Modeling the dynamic route choice of
pedestrians to assess the criticality of building evacuation. Advances in Complex Systems. 2012"

# Operational models

Several operational models are implemented in jpscore. An operational model defines how pedestrians interact with each
other and with their environment.

In the definition of agent's properties it is mandatory to precise the number of the model to be used e.g.:

```xml

<agents operational_model_id="n">
```

The definition of any model parameter is composed of two different sections:

- **model_parameters**: Model specific parameter. See below in the different model sections.
- **agent_parameters**: These parameter are mainly specific for the shape of pedestrians or other pedestrian properties
  like desired speed, reaction time etc.

## Model parameters (in general)

- `<stepsize>0.001</stepsize>`:
  - The time step for the solver. This should be choosed with care. For force-based model it is recommended to take a
    value between $$ 10^{-2} $$ and $$10^{-3}$$ s. For first-order models, a value of 0.05 s should be OK. A larger
    time step leads to faster simulations, however it is too risky and can lead to numerical instabilities, collisions
    and overlapping among pedestrians.
  - Unit: s

- `<exit_crossing_strategy>3</exit_crossing_strategy>`
  - Positive values in $$[1, 9]$$. See below.

- `<linkedcells enabled="true" cell_size="2"/>`
  - Defines the size of the cells. This is important to get the neighbors of a pedestrians, which are all pedestrians
    within the eight neighboring cells. Larger cells, lead to slower simulations, since more pedestrian-pedestrian
    interactions need to be calculated.
  - Unit: m

### Direction strategies

The chosen model the direction strategy should be specified in the [inifile](jpscore_inifile.html) as follows

```xml

<exit_crossing_strategy>num</exit_crossing_strategy>
```

with *num* a positive integer.

The majority of the strategies define how a pedestrian crosses a line $$L = [P_1, P_2]$$. Possible values are:

#### Strategy 1

The direction of the pedestrian is towards the middle of $$L$$ ($$\frac{P_1+P2}{2}$$)
![Strategy 1: Goal towards the center of the door]({{ site.baseurl }}/images/strategy1.png)

#### Strategy 2

Choose the nearest point on the *line* $$L$$. If the nearest point of the pedestrian on the line $$L$$ is outside the
segment, then chose the middle point as target. Otherwise the nearest point is chosen.
![Strategy 2: Goal towards the nearest point on the door.]({{ site.baseurl }}/images/strategy3.png)

#### Strategy 3

The direction is given by the nearest point on $$L$$ to the position of the pedestrian. $$L$$ is shorten by the shoulder
width of the pedestrians (+10 cm).
![Strategy 3: Goal towards the center if pedestrian is out of range of the door.]({{ site.baseurl
}}/images/strategy2.png)

#### Strategy 8

{% include note.html content="For this to work properly the [floor field router](#floorfield-router)
has to be used"%}

For more details see this [talk](https://fz-juelich.sciebo.de/index.php/s/s1ORGTUssCsHDHC) and the
corresponding [thesis](https://fz-juelich.sciebo.de/index.php/s/VFnUCH2gtz1mSoL).

This strategy uses a floor field collection for each room.

The following parameters of an enhanced floor field can be changed:

- `delta_h`: discretization/stepsize of grid-points used by the floor field
- `wall_avoid_distance`: below this wall-distance, the floor field will show a wall-repulsive character, directing
  agents away from the wall
- `use_wall_avoidance`: {true, false} switch to turn on/off the enhancement of the floor field

{%include tip.html content="It's recommended to choose a reasonable value of the `wall_avoid_distance` (shoulder width
of an average pedestrian) in order to not steer pedestrians too close to walls"%}

![Floorfield: Changing the minimal distance to the walls to guarantee a "safe" route]({{ site.baseurl
}}/images/transformFF.png)

Usage example:

```xml

<model_parameters>
    <exit_crossing_strategy>8</exit_crossing_strategy>
    <delta_h>0.0625</delta_h>
    <wall_avoid_distance>0.8</wall_avoid_distance>
    <use_wall_avoidance>true</use_wall_avoidance>
</model_parameters>
```

Here the floor fields are smaller but cannot steer to targets in a different room. The router **must** provide
intermediate targets for every agent, the target being in the same room.

{%include warning.html content="The projection of the room onto the ($$x,\, y$$)-plane must be non-overlapping!"%}
{%include warning.html content="do not use in multi-storage buildings!"%}

#### Strategy 9

This strategy uses a floor field collection for each subroom. (__broken__)

{% include note.html content="For this to work properly the [floor field router](#floorfield-router)
has to be used"%}

Thus the floor fields are again smaller but cannot steer to targets in a different subroom. The router **must** provide
intermediate targets for every agent, that target being in the same subroom.

{%include warning.html content="The projection of the room onto the ($$x,\, y$$)-plane must be non-overlapping!"%}
{%include warning.html content="do not use in multi-storage buildings!"%}

---

[#Chraibi2011]: http://aimsciences.org/journals/displayPaper.jsp?paperID=6440 "Chraibi el al. Force-based models of pedestrian dynamics.  Pages: 425 - 442, Volume 6, Issue 3, September 2011"

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
- `<escalator_upstairs mu="0.6" sigma="0.0" />`
  - Desired speed of agents on escalators upstairs
  - Unit: m/s
- `<escalator_downstairs mu="0.6" sigma="0.0" />`
  - Speed of agents on escalators downstairs
  - Unit: m/s

{%include important.html content="The desired speed changes *smoothly* from one plane to another. See
this [documentation](jpscore_desired_speed.html) for more details."%}

The reduced speed on stairs (up) is according to Tab 1 in [Burghardt2014][#Burghardt2014].

| Handbook | Speed Stair Up |
|----------|----------------|
| PM       | 0.63 m/s       |
| WM       | 0.61 m/s       |
| NM       | 0.8 m/s        |
| FM       | 0.55 m/s       |

### Shape of pedestrians

Pedestrians are modeled as ellipses with two semi-axes: $$a$$ and $$b$$, where

$$ a= a_{min} + a_{\tau}v, $$

and

$$ b = b_{max} - (b_{max}-b_{min})\frac{v}{v^0}. $$

$$v$$ is the speed of a pedestrian.

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

### Model parameters (GCFM)

- `<force_ped nu="0.6" dist_max="3" disteff_max="2" interpolation_width="0.1" />`
  The repulsive force between two agents. See [Fig. 7](https://arxiv.org/pdf/1008.4297.pdf).
  - `nu` is the strength of the force ($$\nu$$ in Eq. (19)).
  - `dist_max` is the maximum force at contact ($$f_m$$)
  - `disteff_max`: cut-off radius ($$r_c$$). Note this value should be smaller than `cell_size` of the linkedcells.
    See [Model parameters (in general)](#model-parameters-in-general).
  - `interpolation_width` ($$r_{eps}$$)
- `<force_wall nu="0.1" dist_max="1" disteff_max="2" interpolation_width="0.1" />`
  The parameters for the repulsive force between a wall and an agent are defined in analogy to the agent-agent repulsive
  force.

A definition of this model could look like:

```xml

<model operational_model_id="1" description="gcfm">
    <model_parameters>
        <stepsize>0.01</stepsize>
        <exit_crossing_strategy>3</exit_crossing_strategy>
        <linkedcells enabled="true" cell_size="2.2"/>
        <force_ped nu="0.6" dist_max="3" disteff_max="2" interpolation_width="0.1"/>
        <force_wall nu="0.1" dist_max="1" disteff_max="2" interpolation_width="0.1"/>
    </model_parameters>
    <agent_parameters agent_parameter_id="1">
        <v0 mu="1.0" sigma="0.0"/>
        <bmax mu="0.15" sigma="0.001"/>
        <bmin mu="0.15" sigma="0.001"/>
        <amin mu="0.15" sigma="0.001"/>
        <tau mu="0.5" sigma="0.001"/>
        <atau mu="0.0" sigma="0.000"/>
    </agent_parameters>
</model>
 ```

## Collision-free Speed Model

[Collision-free speed model][#Tordeux2015] is a velocity-based model. See also
this [talk](https://www.dropbox.com/s/fj1xud5ap2aq59o/Tordeux2015_Talk.pdf) for more details about the model.

Usage:

```xml

<model operational_model_id="3" description="Tordeux2015">
```

### Model parameters (Tordeux2015)

Besides the options defined in [Model parameters](#model-parameters-in-general) the following options are necessary for
this model:

- `<force_ped  a="5" D="0.2"/>`
  - The influence of other pedestrians is triggered by $$a$$ and $$D$$ where $$a$$ is the strength of the interaction
    and $$D$$ gives its range. The naming may be misleading, since the model is **not** force-based, but
    velocity-based.
  - Unit: m
- `<force_wall a="5" D="0.02"/>`:
  - The influence of walls is triggered by $$a$$ and $$D$$ where $$a$$ is the strength of the interaction and $$D$$
    gives its range. A larger value of $$D$$ may lead to blockades, especially when passing narrow bottlenecks.
  - Unit: m

The names of the aforementioned parameters might be misleading, since the model is *not* force-based. The naming will be
changed in the future.

### Agent parameters (Tordeux2015)

Actually, this model assumes circular pedestrian's shape, therefore the parameter for the semi-axes should be chosen,
such that circles with constant radius can be obtained. For example:

```xml

<bmax mu="0.15" sigma="0.0"/>
<bmin mu="0.15" sigma="0.0"/>
<amin mu="0.15" sigma="0.0"/>
<atau mu="0." sigma="0.0"/>
```

This defines circles with radius 15 cm.

Other parameters in this section are:

- `<T mu="1" sigma="0.0" />`
  - Specific parameter for model 3 (Tordeux2015). Defines the slope of the speed function.

In summary the relevant section for this model could look like:

```xml

<model operational_model_id="3" description="Tordeux2015">
    <model_parameters>
        <stepsize>0.05</stepsize>
        <exit_crossing_strategy>3</exit_crossing_strategy>
        <linkedcells enabled="true" cell_size="2"/>
        <force_ped a="5" D="0.2"/>
        <force_wall a="5" D="0.02"/>
    </model_parameters>
    <agent_parameters agent_parameter_id="1">
        <v0 mu="1.34" sigma="0.0"/>
        <v0_upstairs mu="0.668" sigma="0.167"/>
        <v0_downstairs mu="0.750" sigma="0.188"/>
        <escalator_upstairs mu="0.5" sigma="0.0"/>
        <escalator_downstairs mu="0.5" sigma="0.0"/>
        <bmax mu="0.15" sigma="0.0"/>
        <bmin mu="0.15" sigma="0.0"/>
        <amin mu="0.15" sigma="0.0"/>
        <atau mu="0." sigma="0.0"/>
        <T mu="1" sigma="0.0"/>
    </agent_parameters>
</model>
```

{%include note.html content="The recommended values are by no means universal, and may/should be calibrated to fit your
scenario.

Moreover, some parameter values, for instance $$\nu$$ in the GCFM or $$a$$ in Tordeux2015, have to be chosen wisely.
Otherwise, it is possible that the agents overlap excessively, since no explicit collision-detection algorithms are
implemented in these models. In case of excessive overlapping we recommend to perform the simulation again with
different values.
"%}

[#GCFM]: http://journals.aps.org/pre/abstract/10.1103/PhysRevE.82.046111 "Mohcine Chraibi, Armin Seyfried, and Andreas Schadschneider Phys. Rev. E 82, 046111"

[#GCFM_PREPRINT]: https://arxiv.org/abs/1008.4297 "Preprint ArXiv"

[#Tordeux2015]: http://arxiv.org/abs/1512.05597  "Tordeux, Antoine, Chraibi, Mohcine and Seyfried, Armin, Collision-free speed model for pedestrian dynamics. In Traffic and Granular Flow  '15, to appear."

[#Burghardt2014]: http://link.springer.com/chapter/10.1007%2F978-3-319-02447-9_27 "Burghardt, Sebastian and Seyfried, Armin and Klingsch, Wolfram. Fundamental diagram of stairs: Critical review and topographical measurements. Pedestrian and Evacuation Dynamics 2012"

# Optional Settings for an Advanced Simulation

## Routing

The routing can be adapted by the definition of additional intermediate goals **inside** or final goals **outside** the
geometry.

```xml

<routing>
    <goals>
        <goal id="0" final="false" caption="goal 1">
            <polygon>
                <vertex px="-5.0" py="-5.0"/>
                <vertex px="-5.0" py="-2.0"/>
                <vertex px="-3.0" py="-2.0"/>
                <vertex px="-3.0" py="-5.0"/>
                <vertex px="-5.0" py="-5.0"/>
            </polygon>
        </goal>
        <waiting_area caption="wa1" id="1" waiting_time="20" min_peds="5" max_peds="10" is_open="true" room_id="0"
                      subroom_id="1" global_timer="false" transition_id="1">
            <polygon>
                <vertex px="11" py="1"/>
                <vertex px="14" py="1"/>
                <vertex px="14" py="4"/>
                <vertex px="11" py="4"/>
                <vertex px="11" py="1"/>
            </polygon>
            <next_wa id="2" p="0.75"/>
            <next_wa id="3" p="0.25"/>
        </waiting_area>
        <file>goals.xml</file>
    </goals>
</routing>
```

### Goals

Additional goals, which are defined **outside** the geometry. {%include note.html content="Goals should _NOT_ overlap
with any walls or be inside rooms."%}

It is recommended to position them near the exits.

- Goals are defined with close polygons, with  *the last vertex is equal to the first one*.
- `file` file containing further goals.

### Waiting Area

Addional goals, which are defined **inside** the geometry.

{%include note.html content="Waiting areas should _NOT_ overlap with any walls or be outside rooms."%}

Here are some use cases:

- Waiting: Designated waiting areas where pedestrians wait for a specific time or till an specific door opens. After
  waiting is over the move to one of the specified next goal (decided individually for each ped).
- Manual routing: Goals which should be passed before leaving the building without waiting (`waiting_time="0"`). Can be
  used to reduce jam in front of bottlenecks or ensuring pedestrian walk through certain paths. {%include warning.html
  content="Unexpected behaviour might be observed if the shortest distance to the next goal does not match the desired
  route. In such a case consider placing the waiting area at an other place. "%}

- Waiting areas are defined as closed polygons, with  *the last vertex is equal to the first one*.

- `file`: file containing further goals/waiting areas. See [goals.xml](jpscore_goal.html)
- `waiting_time`: the time pedestrians wait inside the waiting area
- `min_peds`: the number of pedestrians needed in the waiting area to start the timer (if `global_timer` = false)
- `max_peds`: the max number of pedestrians allowed inside the waiting area. {%include note.html content="to avoid
  undefined behaviour `max_peds` should not exceed the number of pedestrians heading for an other waiting area.
  Hence `max_peds(WA1)` <= `max_peds(WA2)`."%}
- `is_open`: defines whether the waiting area is open for pedestrians
- `room_id`: ID of room containg waiting area
- `subroom_id`: ID of subroom containing waiting area
- `global_timer`: If `true` timer starts with start of the simulation, else timer starts when `min_peds` pedestrians are
  inside waiting area
- `transition_id`: waits till the specific door opens. **Important:** `waiting_time` is neglected in this case!

- `next_wa`: Next waiting area or goal where pedestrians are heading for
  - `id`: ID of next waiting area/goal, -1 for next exit
  - `p`: probability of pedestrians being led to the specific next waiting area. During simulation if `max_ped` of the
    particular waiting is reached it will not be considered.

## Traffic constraints

This section defines constraints related to doors.

```xml

<traffic_constraints>
    <doors>
        <file>traffic.xml</file>
    </doors>
</traffic_constraints>
```

- `file` file containing further constraints.

{%include note.html content="although all traffic constraints can be written in the inifile, we recommend to use a
seperated file and linked it in the inifile as shown above."%}

### Definitions

Following properties of `transitions` can be defined before the start of the simulation:

- `trans_id`: unique id of a transition. Should match an existing id in
  the [geometry file](jpscore_geometry.html#transitions)
- `caption`: optional parameter defining the caption of the door.
- `state` defines the state of the door. Options are `close`, `temp_close` or `open`.
- `dn`: number of agents to pass the door before triggering the process of flow regulation.
- `outflow`: Max flow through door. Door's state will be changed adequately.
- `max_agents`: Max agents that can pass door. Door will be closed permanently

### Sample

```xml
 <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<JPScore project="JPS-Project" version="0.8">
    <traffic_constraints>
        <doors>
            <door trans_id="2" state="open"/>
            <door trans_id="3" state="open"/>
            <door trans_id="4" state="open"/>
            <door trans_id="5" state="open"/>
            <door trans_id="0" state="open" dn="10" outflow="2" max_agents="200"/>
        </doors>
    </traffic_constraints>
</JPScore>
```

### Changes of door's status

Doors have three statuses:

* _open_
* _close_
* _temp_close_

They can all be set and changed by an event.

Here are the rules governing the changes from one status to another:

* Events have priority before the door-flow regulation.
* When max_agents are reached the door status is _close_.
* Doors that are closed by max_agents can be opened again by the event "reset".
* If the door status is _temp_close_ the agents should wait in front of the door until it opens again.
* If a door status is _close_, the agents should look for another door and **not** wait in front of the closed door. And
  if there is no other open door, the agents should wait inside the room.

![Changes of door's status]({{ site.baseurl }}/images/door_states.png)

## Train constraints

Information regarding trains are organized in two different files:

- [Train types](#train-types): In this file types of trains are defined.
- [Train timetable](#train-timetable): This file defines arrival and departure times of trains and specifies their
  location on tracks.

```xml

<train_constraints>
    <train_time_table>ttt.xml</train_time_table>
    <train_types>train_types.xml</train_types>
</train_constraints>
```

### Train types

#### Definition

A train is defined through the following information:

- `type` (string): unique key to identify train
- `length` (float): length of the train
- `agents_max` (int): maximal number of passengers
- door:
  - `id` (int): id of the train door
  - `distance` (float): distance to train start
  - `width` (float): width of the door
  - (optional) `outflow` (float): maximum flow at the specific train door (persons per second).

{%include note.html content="The parameter `length` is not used yet. In future it will be used for sanity checks and for
avoiding overlapping trains."%}

#### Example

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<train_type>
    <train type="RE" agents_max="15" length="9">
        <door id="1" distance="1" width="1"/>
        <door id="3" distance="5" width="1" outflow="2."/>
    </train>

    <train type="ICE" agents_max="20" length="23">
        <door id="1" distance="1.5" width="2" outflow="1.5"/>
        <door id="2" distance="10" width="3" outflow="3."/>
        <door id="3" distance="18" width="2" outflow="1.5"/>
    </train>
</train_type>
```

![Schematic overview of train type definition.]({{ site.baseurl }}/images/trains/traintype.png)

#### Capacity of a train

The number of agents in a train is calculated every time step as the sum of all agents passing through the train's
doors.

When this number exceeds the `agents_max` parameter, all train's doors are closed.

### Train timetable

#### Definition

A train is defined through the following information:

- `id` (int): id of the train
- `type` (string): identifier of the train defined with the train types
- `track_id` (int): id of the track the train arrives, more information on
  tracks ([geometry defintion](#geometry-definition))
- `train_offset` (float): offset of the train to the track start
- `reversed` (bool): <br>false: train starts at track start (default), <br>true: train starts at track end
- `arrival_time` (float): time the train arrives
- `departure_time` (float): time the train departs

{%include note.html content="The parameter `reversed` adds doors starting from track end in direction of track start!"%}

#### Example

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<train_time_table>
    <train id="1" type="RE"
           track_id="2"
           train_offset="2"
           arrival_time="5" departure_time="15">
    </train>

    <train id="2" type="ICE"
           track_id="3"
           train_offset="1" reversed="true"
           arrival_time="20" departure_time="30">
    </train>
</train_time_table>
```

With the train defined as above, this would lead to the positioning of the train doors as seen here:
![Schematic overview of train arrival on platform.]({{ site.baseurl }}/images/trains/traintimetable.png)

### Geometry definition

For using trains in the simulation, tracks need to be defined in the geometry. A track is a consecutive line sequence,
marked by the type `type="track"`. For each track, one point needs to be marked as a starting point with `start="true"`,
which represents the reference point for the location of trains on tracks (defined
in [Train timetable](#train-timetable)).

#### Example

```xml

<room id="1" caption="floor">
    <subroom id="2" caption="Room 2" A_x="0" B_y="0" C_z="0">
        ...
        <polygon caption="wall" type="track" track_id="1">
            <vertex px="-10" py="10" start="true"/>
            <vertex px="-5" py="10"/>
        </polygon>
        <polygon caption="wall" type="track" track_id="1">
            <vertex px="-5" py="10"/>
            <vertex px="0" py="10"/>
        </polygon>
        ...
    </subroom>
</room>
```

### Geometry adaptation

For realizing trains in the simulation, doors need to be added or removed dynamically during the simulation when trains
are arriving or departing.

#### Calculation of train door positions

For calculating the train door coordinates, the following rules apply:

- Distance between train start and door start is calculated along the track walls
- Distance between door start and door end is calculated as direct connection (see below)

#### Splitting of the track walls

##### Case 1: Door on one single wall element

![Door on a single wall element.]({{ site.baseurl }}/images/trains/trainCase1.png)

##### Case 2: Door on two neighboring wall elements

![Door on two neighboring wall elements.]({{ site.baseurl }}/images/trains/trainCase2.png)

##### Case 3: Door on more than two wall elements

![Door on more than two wall elements.]({{ site.baseurl }}/images/trains/trainCase3.png)

{%include note.html content="Please be aware that depending on the curvature of the platform edges a significant area of
platform might be not accessible. Pedestrian inside this area will be removed from the simulation and report as
erroneous."%}
