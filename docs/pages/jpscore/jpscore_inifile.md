---
title: jpscore inifile
keywords: simulation
tags: [jpscore, file]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_inifile.html
summary:  The project file where the settings for a jpscore simulation are defined. Within this file properties of pedestrians, model parameters, etc can be given.
last_updated: Dec 31, 2019
---

## Header
The header comprises the following elements:

-   `<seed>seed</seed>`

    Set the  `seed` value of the random number generator to `s`. If missing the
    current time (`time(NULL)`), is used i.e. random initial conditions.

-   `<max_sim_time>t</max_sim_time>`
    the maximal simulation time in seconds.

-   `<num_threads>n</num_threads>` the number of used cores.

-   `<geometry>geometry.xml</geometry>`
    The name and location of the geometry file. All file locations are relative
    to the actual location of the project file. See [specification](jpscore_geometry.html) of the geometry format.
    
-   `<events_file>events.xml</events_file>`
    The name and location of the event file. Events can be used to open or close doors at a certain point of time. 
    See [event_file](jpscore_events.html).
    
-   `<schedule_file>schedule.xml</schedule_file>`
    The name and location of the schedule file. Schedules can be used to group doors and open or close this 
    groups of door at certain points of time. See [schedule_file](jpscore_schedule.html)

-   `<show_statistics>true</show_statistics>` Show different aggregate statistics e.g. the usage of the doors. (default: false)


-  The trajectory file

```bash
 <trajectories format="xml-plain" fps="8" color_mode="velocity">
    <file location="trajectories.xml" />   
 </trajectories>
```

The options for the format are

-   `xml-plain`: the default xml format. It can lead to large files. See section [xml-plain](jpscore_trajectory#xml).

-   `plain`: simple text format. See section [plain](jpscore_trajectory#txt).
    - `optional_output`: possibility to give additional output. Set any of these values to `TRUE` (not case sensitive) to get the according value. `FALSE`, any other value and ignoring the option will lead to no output.
        - `speed`: speed of the pedestrian
        - `velocity`: x,y components of the pedestrians's velocity
        - `final_goal`: id of the final goal the pedestrian is heading to
        - `intermediate_goal`: id of the current goal the pedestrian is heading to (usually a door)
        - `desired_direction`: x,y components of the pedestrians's desired directions
        - `group`: group of the pedestrian
        - `router`: router used by the pedestrian (id accoriding to ini-file)
        - `spotlight`: pedestrian is highlighted.
        
        ```xml
            <trajectories format="plain"  fps="8">
              <file location="bottleneck_traj.txt" />
              <optional_output speed="FALSE" velocity="TRUE" final_goal="NoOutputWrongValue"  intermediate_goal="false" desired_direction="true" group="TRUE" router="TRUE" spotlight="TRUE"/>
            </trajectories>
        ```
        
{%include warning.html content="xml trajectories tend to become large and hence slow down parsing and visualisation." %}

- The value `fps` defines the frame rate per second for the trajectories.

    - `color_mode`: coloring agents in the trajectories. Options are:
       - `velocity` (default): color is proportional to speed (slow --> red).
       - `spotlight`
       - `group`: color by group
       - `knowledge`
       - `router`
       - `final_goal`
       - `intermediate_goal`

    - `file location` defines the location of the trajectories.
       All paths are relative to the location of the project file.


## Train constraints

In the inifile the following section should be defined, where two files are specified:
- [Train timetable](jpscore_trains.html#train-timetable)
- [Train types](jpscore_trains.html#train-types)

```
 <train_constraints>
   <train_time_table>ttt.xml</train_time_table>
   <train_types>train_types.xml</train_types>
 </train_constraints>
```

For detailed information see [here](jpscore_trains.html).

## Traffic constraints

This section defines constraints related to doors.

```xml
 <traffic_constraints>
    <doors>
        <file>traffic.xml</file>
    </doors>
 </traffic_constraints>
```

- `file` file containing further constraints. See [traffic.xml](jpscore_traffic.html)

{%include note.html content="although all traffic constraints can be written in the inifile, we recommend to use a seperated file and linked it in the inifile as shown above."%}

## Routing

The routing comprises additional goals, which might be defined **inside** or **outside** the geometry.     

```xml
 <routing>
    <goals>
        <goal id="0" final="false" caption="goal 1">
            <polygon>
                <vertex px="-5.0" py="-5.0" />
                <vertex px="-5.0" py="-2.0" />
                <vertex px="-3.0" py="-2.0" />
                <vertex px="-3.0" py="-5.0" />
                <vertex px="-5.0" py="-5.0" />
            </polygon>
        </goal>
	<waiting_area caption="wa1" id="1" waiting_time="20" min_peds="5" max_peds="10" is_open="true" room_id="0" subroom_id="1" global_timer="false" transition_id="1">
            <polygon>
                <vertex px="11" py="1" />
                <vertex px="14" py="1" />
                <vertex px="14" py="4" />
                <vertex px="11" py="4" />
                <vertex px="11" py="1" />
            </polygon>
            <next_wa id="2" p="0.75"/>
            <next_wa id="3" p="0.25"/>
	</waiting_area>
        <file>goals.xml</file>
    </goals>
 </routing>
```

## Goals

Additional goals, which are defined **outside** the geometry.
{%include note.html content="Goals should _NOT_ overlap with any walls or be inside rooms."%}

It is recommended to position them near the exits.
- Goals are defined with close polygons, with  *the last vertex is equal to the first one*.
- `file` file containing further goals. See [goals.xml](jpscore_goals.html)

## Waiting Area

Addional goals, which are defined **inside** the geometry.

{%include note.html content="Waiting areas should _NOT_ overlap with any walls or be outside rooms."%}

Here are some use cases:
- Waiting: Designated waiting areas where pedestrians wait for a specific time or till an specific door opens. After waiting is over the move to one of the specified next goal (decided individually for each ped).
- Manual routing: Goals which should be passed before leaving the building without waiting (`waiting_time="0"`). Can be used to reduce jam in front of bottlenecks or ensuring pedestrian walk through certain paths. 
{%include warning.html content="Unexpected behaviour might be observed if the shortest distance to the next goal does not match the desired route. In such a case consider placing the waiting area at an other place. "%}

- Waiting areas are defined as closed polygons, with  *the last vertex is equal to the first one*.

- `file`: file containing further goals/waiting areas. See [goals.xml](jpscore_goal.html)
- `waiting_time`: the time pedestrians wait inside the waiting area
- `min_peds`: the number of pedestrians needed in the waiting area to start the timer (if `global_timer` = false)
- `max_peds`: the max number of pedestrians allowed inside the waiting area. 
{%include note.html content="to avoid undefined behaviour `max_peds` should not exceed the number of pedestrians heading for an other waiting area. Hence `max_peds(WA1)` <= `max_peds(WA2)`."%}
- `is_open`: defines whether the waiting area is open for pedestrians
- `room_id`: ID of room containg waiting area
- `subroom_id`: ID of subroom containing waiting area
- `global_timer`: If `true` timer starts with start of the simulation, else timer starts when `min_peds` pedestrians are inside waiting area
- `transition_id`: waits till the specific door opens. **Important:** `waiting_time` is neglected in this case!

- `next_wa`: Next waiting area or goal where pedestrians are heading for
  - `id`: ID of next waiting area/goal, -1 for next exit
  - `p`: probability of pedestrians being led to the specific next waiting area. During simulation if `max_ped` of the particular waiting is reached it will not be considered.

## Agents

There are two ways to distribute agents for a simulation:

- [random distribution](#agents_distribution) in a specific area *before* the simulation starts.
- distribution by means of [sources](#sources) *during* the simulation.

```xml
 <agents>
    <agents_distribution>
        <group group_id="1" room_id="0" number="10" />
        <group group_id="2" room_id="0" subroom_id="0" number="10" goal_id="" router_id="1" />
    </agents_distribution>
    <agents_sources>
        <source id="1" frequency="2" agents_max="10" group_id="1" caption="caption" greedy="true"/>
        <source id="2" time="10" agent_id="50" group_id="1" caption="caption" greedy="true"/>
        <source id="10" caption="new-source" time_min="5" time_max="30" frequency="5" N_create="10" agents_max="300" group_id="0"  x_min="0" x_max="3" y_min="0" y_max="3" percent="0.5" rate="2"  greedy="true"/>
    <file>sources.xml</file>
    </agents_sources>	 
 </agents>
```

### Agents_distribution
Above is an example how to define agent's characteristics with different number of 
attributes, which are as follows:

- `group_id`: mandatory parameter defining the unique id of that group.

- `number`: mandatory parameter defining the number of agents to distribute.

- `room_id`: mandatory parameter defining the room where the agents should be
randomly distributed.

- `subroom_id`: defines the id of the subroom where the agents should be distributed.
  If omitted then the agents are homogeneously distributed in the room.

- `goal_id`: should be one of the `id`s defined in the section  [goals](#goals).
   If omitted or is `-1` then the shortest exit to the outside is chosen by the agent.

- `router_id`: defines the route choice model to be used. See [routing documentation](jpscore_routing.html) of available routers.

- `agent_parameter_id`: choose a set of parameters for the [operational models](jpscore_operativ.html).

- `x_min`, `x_max`, `y_min` and `y_max`: define a bounding box where agents should be distributed.

- `startX`, `startY`: define the initial coordinate of the agents. This might be useful for testing/debugging.
Note that these two options are only considered if `number=1`.

- `pre_movement_mean` and `pre_movement_sigma`: premovement time is Gauss-distributed $$\mathcal{N}(\mu, \sigma^2)$$.

- Risk tolerance can be Gauss-distributed, or beta-distributed.
  If not specified then it is defined as $$\mathcal{N}(1, 0)$$:

  - `risk_tolerance_mean` and `risk_tolerance_sigma`: $$\mathcal{N}(\mu, \sigma^2)$$.

  - `risk_tolerance_alpha` and `risk_tolerance_beta`: $$Beta(\alpha, \beta)$$.

- `patience`:  this parameter influences the route choice behavior when using the quickest path router.
  It basically defines how long a pedestrian stays in jams before attempting a rerouting.

- `age`: not yet used by any of the [operational](jpscore_operativ.html) models.

- `gender`: not yet used.

- `height`: not yet used.

### Sources
Besides distributing agents randomly before the simulation starts, it is
possible to define sources in order to "inject"
new agents in the system during the simulation.
The parameter of the sources defined [here](jpscore_sources.html).

An example of usage:
- Busses are coming every 10 min (600 seconds).
- Every bus transports 100 pedestrians.
- When the bus stops, every 2 seconds 10 pedestrians leave the bus.
- 3 Buses at max.

```xml
 <source id="10" frequency="600" N_create="100" agents_max="300"
   percent="0.1" rate="2"/>
```

## Operational models
One of the available [operational models](jpscore_operativ.html) should be defined.

## Router
One of the available [routers](jpscore_routing.html) should be defined.
