# JuPedSim modernisation efforts

In this document we want to discuss how to iteratively improve jpscore as an
alternative to a rewrite from scratch. Part of this incremental strategy is to
keep as much functionality of jpscore working as possible during the rework and
only temporarily, i.e. for days at a time break core functionality. Such an
effort needs to include a discussion about which features to keep and/or fix.

## Issues with jpscore

### Hard to modify

As of now it is almost impossible to change jpscore, this can be attributed to:

* Tight coupling of unrelated functionality, e.g. trajectory writing integrated
  into simulation.
* Inheritance favoured over composition for code reuse.
* Under developed interfaces, i.e. lots of `GetXYZ()` methods and ad-hoc
  computation on the accessed data instead of methods on the objects containing
  the data.

### Instability

The application does exhibit many crash bugs. Some simulation results are not
plausible if the model is to be applied correctly, e.g. pedestrians
"disappearing" mid simulation or passing trough walls.

### Difficult data management

Input data are managed in different files, and copied on writing a trajectory
file. There is no support for higher level data management approaches, e.g. a
workspace to manage multiple trajectories and support for easy rerunning of
simulations.  

## Transform jpscore to a "simulator" like architecture - Initial idea

Ultimately the goal is to transform the existing codebase into the same target
architecture as chosen for the rewrite, i.e. "simulator"

Such a rework could potentially be done in the following broadly outlined steps:

* Remove dead code
* Remove multithreading
* Extract and separate functionality on the highest abstraction level, i.e.
  Simulator, Application, Output Writers and Input Parsers.
* Separate universal data structure from model specific data structures.
* Generalise abstractions of e.g. Rooms, Stairs, Escalators into concepts such
  as "Area with property" by hiding old implementation behind new interface.
* Separate computation according to layering chose for "simulator"

Additionally a strong emphasis should be put on creating documentation along
the way.

### Remove parallelisation with OpenMP

Right now it is very difficult to follow basic computations and the way the
code is parallelized seems to contain several "not readily addressable" race
conditions. Instead of slowly trying to address those races it might lead to
quicker results if we simply remove parallelization with openmp as a very first
step. Parallelisation should be a concern when reworking code but it should not
be reintroduced prior to other parts of the code base getting untangled first.

### Separate output generation from simulation

As of now writing trajectories is contained in the simulation class. It will
lead to a more flexible implementation if this responsibility is removed from
the simulation class. This also implies that the simulation class no longer
manages the continuation of the simulation but rather exposes an interface that
allow for running a single iteration and checking various simulation state to
either write this state to a file, i.e. trajectories. Such a change would also
allow to provide for an online visualization mid-term.

### Separate pedestrian behaviour from pedestrian data structures

Pedestrian behaviour varies by model, hence the behaviour should not be
implemented in the Pedestrian class. The Pedestrian class models data common to
all models. Models that need additional data need to create associated data
structures and the application needs to provision a mechanism to associate
these data structures with a specific pedestrian.

### Remove geometry correction

Input data should not be automatically modified by the application. If input
data is not consumable or otherwise erroneous no auto correct should be
applied. This will reduce complexity in the application and make issues more
transparent, e.g. what happens if the auto-correction is not correcting as
expected? Why is the application processing this "broken" geometry but may
error out on a different "broken" geometry? Instead the application should
treat input data as is or provide the user with a intelligible error message
that enables the user to diagnose the issue with his input data.

## Outcome discussion Oct. 27th 2021

It was decided to rework jpscore so that it will midterm follow the
architecture outlined for 'simulator'. We want to meet in regular intervals to
discuss roadblocks, progress and next steps.

First tasks:

* Remove OpenMP support and Geometry Correction.

* Extract simulation main loop from and pull it up into 'main' to enable
  removing e.g. trajectory writing, input parsing, etc. from the 'Simulation'
  class.

* Provide each model with its own distinct agent class to remove model specific
  fields from agent.

