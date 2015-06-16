# Change Log
All notable changes to this project will be documented in this file.


## v0.7.0 [Unreleased]
### Added
- Changelog file
- Rimea testcases
- Boost testcases for geometry functions
- risk tolerance factor (value in [0 1]) for pedestrian. Pedestrians with high values are likely to take more risks.
- Sources for generating agents at runtime. Parameter are frequency(agents per seconds) and maximum number
- Option to color the pedestrians by group,spotlight,velocity,group,knowledge,router,final_goal,intermediate_goal. Usage: ( <trajectories format="xml-plain" fps="8" color_mode="group"> )
- More control over the triangulation specially to avoid skinny triangles. Usage: <navigation_mesh method="triangulation" minimum_distance_between_edges="0.5" minimum_angle_in_triangles="20" use_for_local_planning="true" />
- Improved statistics Flow curve for the different exits.

#### JPSVIS
- Display the geometry structure individual room/subroom
- Now build on OSX 

### Changed
-  
-  

### Fixed
- Visiblity in 3D
- Numerous geometrical operations

### Fixed
- 

## v0.6.0 - 2015-01-31
### Added
- Steering the simulation with predefined events (closing or opening doors during the simulation)
- Information sharing between the pedestrians. The agents now share their knowledge about closed doors.
- Pre evacuation time for groups of agents.
- Adjustable velocities on stairs and even terrain for group of agents.
- Stability and performance improvement. The simulation is approx 40% faster for larger scenarios and you will notice it
- New route choice model, cognitive map, giving agents the possibility to explore the environment and discover doors for instance.
- Different sensors for improving the navigation of pedestrians (smoke/jam sensor).
- New verification and validation tests.
- General statistics over the evacuation (for instance areas egress time and door usage)
- Support for Visual Studio and Xcode compilers.


### Changed
- refactor NumCPU and ExitCrossingStrategy tags to num_threads and exit_crossing_strategy

### Fixed
-

## v0.5.0 - 2014-08-05