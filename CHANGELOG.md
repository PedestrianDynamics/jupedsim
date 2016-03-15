# Change Log
All notable changes to this project will be documented in this file.

## Known Issues
#### JPSCORE
Please also check the Issue tracker on GitLab, if access is available.
- exit strategies using *local* - prefix can result in circular routing
- triangulation creates error-prone help-lines in certain geometries
- errors in unused parts of ini files lead to system-exit


## v0.8.0 [2016-02-18]


### Added
#### JPSCORE
- CI with travis and Gitlab CI
- Compilation checked on Visual Studio 12 2013 
- Added more validation tests
- Added statistics (calculate exit usage) for all exits
- Added voronoi based algorithm for inserting agents coming from the source or from matsim
- New option for the quickest path router. Sample options are:

 ```<parameters default_strategy="local_shortest">```
 
 ```<parameters default_strategy="global_shortest">``` 
 
 ``` <parameters cba_gain="0.15" reference_peds_selection="single" congestion_ratio="0.8" queue_vel_escaping_jam="0.2" 
       queue_vel_new_room="0.7" visibility_obstruction="4">```
- New model with the generic name `Tordeux2015` and `id=3`. For use check the ini-files in the Utest-validation tests.
- Tests are sorted in `rimea_tests`, `juelich_tests` and `validation_tests`.
- Periodic boundary conditions with the option `<periodic>1</periodic>`.Works only with model 3.  
- Added floor field to all exits, providing direction to target, direction to closest wall and cost estimates. Parameter to control wall-avoidance included.
- Added new __Exit Strategies__ using floor fields on various subdomains (rooms, subrooms). Please refer to the online documentation on [jupedsim.org](www.jupedsim.org).
   
#### JPSVIS
- Added option to load  vtk files. Need to add the line ``` <gradient_field filename="floorfield.vtk"> 
``` in the header of the trajectory file. Alternatively drag and drop a vtk file on JPSvis.
 
- Fixed error displaying the total number of frames when using the scroolbar
 
#### JPSREPORT
- Added geometry information while plotting the voronoi cells
- Added option to disable plotting
- Issue a warning when the voronoi cell cannot be calculated 
- Fixed error where all trajectories were colinear

#### JPSEDITOR
 

## v0.7.0 [2015-07-15]

### New Module
- JuPedSim: Editor for the geometry

### Added

- Risk tolerance factor (value in [0 1]) for pedestrian. Pedestrians with high values are likely to take more risks.
- Added pre-movement time of the agents. Only after this time, the concerned agents will start moving.
- Sources for generating agents at runtime. Parameter are frequency (agents per seconds) and maximum number
- Option to color the pedestrians by group, spotlight, velocity, group, knowledge, router, final\_goal, intermediate\_goal. Usage: ( 
```<trajectories format="xml-plain" fps="8" color_mode="group"> ```)
- More control over the triangulation specially to avoid skinny triangles. Usage: ```<navigation_mesh method="triangulation" minimum_distance_between_edges="0.5" minimum_angle_in_triangles="20" use_for_local_planning="true" />```
- Improved statistics. The flow curve for the different exits can be computed at runtime.
- Changelog file
- Rimea testcases
- Unit tests are now based on the Boost testing engine
#### JPSVIS
- Display the geometry structure individual room/subroom.
- Now build on OSX/Linux/Windows 

### Changed
-  
-  

### Fixed
- Visiblity in 3D
- Numerous geometrical operations


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
- refactor NumCPU and ExitCrossingStrategy tags to `num_threads and exit_crossing_strategy`



## v0.5.0 - 2014-08-05
First release of the the Juelich Pedestrian Simulator. Most noteworthy features:

- Simulate pedestrians movement in a space continuous geometry
- Forces based models for describing the pedestrians interactions
- Shortest and quickest path route choice strategies
- Loading and visualizing trajectories and geometries
- Easy to use visualization interface
- Making high quality videos directly from the visualization interface or generating png sequences
- XML based input files