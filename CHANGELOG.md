# Change Log
All notable changes to this project will be documented in this file.

## v0.8.5 [2019-07-12]

### Added
- Added option for additional/optional output for speed, velocity(x,y), 
final goal, intermediate goal, desired direction(x,y), spotlight, router, group. Thus,
there is no need to run multiple simulation to obtain those values. Attention: File may
become significantly larger!

### Changed

### Fixed

## v0.8.4 [2019-04-12]

### Added
- Added waiting areas, waiting for certain time or till transition opens
- Improve realtime event handling, new tag in ini `<event_realtime>` with commit 292b8381
- New type of doors `{ OPEN, CLOSE, TEMP_CLOSE }` !48  #307
- Outsource some configurations from inifile to external xml files. !46   #302
- Add new properties to doors #299, #295
- Add new properties to sources !41 #298
- Added homebrew [packages](https://github.com/JuPedSim/homebrew-jps/blob/master/README.md)
- added function `correct` to fix sloppily drawn geometries !36
- use cmake option `-D JPSFIRE=true` (default false) !33
- JPSfire-module: Walking speed and toxicity analysis functional (thanks to @hein1)  !31
- Add study Fridolf2018 !29
- Make Windows executables with CPack !28
- New wiki page for AI-Router [WIKI](https://gitlab.version.fz-juelich.de/jupedsim/jpscore/wikis/AI-Router)
- New model (number 6) GCVM !30
- Distribute source agents on a fixed position. Use attributes `startX` and `startY`  6766950e
- Feature dynamic transitions, moving danger line !32

### Changed

- Show statistics for crossings too. !43 #293
- Crossing IDs don't have to be consecutive. !39
- `jpscore` uses C++17.
- Write door statistics  every 1000 frames. 284749ab2e6f3ed85dc40ad24e3579e92c7f7883  #309
- Big TXT-files will be splited in 10MB files. !45  #308
- Change `plain` format dded6083
- Agents without any exits should not be deleted 67ce14cf

### Fixed

- Continuous walls are splited. Write corrected geometry  #281
- Doors on walls are not a problem anymore #282
- Better handle wrong group id for sources
- Better handle parameter id in inifile. #303
- Floorfield router reacts better on events. #294
- Windows path compatibility for JPSfire-module d14229f7
- Several SegFaults related to agents without navigation lines (especially agents created by sources)
- C++11 support for XCode fbb3038a

## v0.8.3 [2018-05-18]
### Added
- Consider direction of escalators in ff-router !22
- Statistics about deleted agents !12
- `cmake  -DAIROUTER=true` to use the `AIrouter`  (default: false) 6fd4a1a0
- Use npy files for smokesensor. !13
- New attributes for sources `agent_id` and `time` for scheduling agent's appearance. See #258 for discussion. !14
- Router tests
- Option for printing progressbar:  `<progressbar/>` cfc65f22
- Universal Floorfield: recognition of inside and outside of rooms, gridpoints know corresponding Subroom
- Floorfield Router: fixes of "Cannot find route for pedestrian ..." bugs; b1345915
- vtk-files (ff_router) can be written with `<write_VTK_files>true</write_VTK_files>` (see wiki)
- Exit_Strategie / Direction_Strategie: fixed bug 269, where agents get stuck at doors. b1345915

### Changed
- Renaming of CogMap-router and new router (Erik). !16

### Fixed
- source thread and main thread are not synchrone. 2d1fb525
- Bug in sources leads to duplicate sgents. #158 5c81985b


## v0.8.2 [2016-10-11]

Repository moved to [new server](https://gitlab.version.fz-juelich.de/jupedsim/jpscore)
### Added
- Adding rolling flow for validation tests (a74f9ba465efc9706b7e0bbe4e51c1cae66b356f)
- New smoke sensor. Adapt speed of agents walking in smoke. See branch [jpsfire_toxicity_analysis](https://gitlab.version.fz-juelich.de/jupedsim/jpscore/commits/jpsfire_toxicity_analysis)
- New option for `groups`. It is now possible to load the first positions of pedestrians from a file.
  Needed for better comparison with experiments. Use attribute: `positions_dir="/path/to/directory/"` `unit="cm"`.
  The unit of the trajectories can be specified. Default is meter. (!3)
- New router: prototype of the floorfield based, quickest router `ff_quickest`. This router will re-asses all routes
  including agents into its routing-calculations. Jams will be avoided, if a clear route is available.

### Changed
- Using only Gitlab CI
- update to RiMEA test 3.0 [changelog_rimea.txt](https://gitlab.version.fz-juelich.de/jupedsim/jpscore/blob/mergePrototype/Utest/rimea_tests/changelog_rimea.txt). Thanks @schumacher3
- automatically make a report with RiMEA tests (!7). Thanks @zhong1
- renamed options `start_x` and `start_y` to `startX` and `startY` resp. (bf28854f)
- New Progressbar (ec7c9b0c)
- UnivFFviaFM class replaces the old (Local)FloorfieldViaFM classes
- FFRouter (`ff_global_shortest`, `ff_quickest`) will delay calculations to a point between timesteps to use all available cores.
- removed writing of VTK-files (this feature will be controllable via inifiles in the future)
- DirectionStrategy using floorfields (exit strat `8`,`9`) will respect shoulderwidth for doors.
- Boost::Geometry is used for geometric checks (isWithin)
- Direction Strategy will be matched, if FFRouter is used (auto-set to exit strat `8`)

### Fixed
- Wrong Z-component in trajectories (!4)
- Several bug fixes
- isClockwise() fixed
- isInSubroom() fixed (#238)
- agents getting stuck and oscillating fixed (#247)
- router problems with `ff_global_shortest` fixed (#245)
- parameters for direction strategy `8`,`9` will be used (`wall_avoid_distance`,`delta_h`, `use_wall_avoidance`)

## v0.8.1 [2016-10-11]
### Added
- Tag Sources: new attribute to choose between "greedy" approach and random approach: `gready="true"` (default: `false`)
- Floor field router. See [usage](http://jupedsim.github.io/jpscore/models/routing/#floorfield-router).
- New attribute for *step_size* `<step_size fix "no">0.01</step_size>`. Default is yes. See #193 for description and commit 78b634ff.
- Added new operational model `<model operational_model_id="5" description="Krausz">`. See [Wiki](https://cst.version.fz-juelich.de/jupedsim/jpscore/wikis/docs/models/operativ#generalized-centrifugal-force-model-with-lateral-swaying)
- Added new `cmake` compilation flags:
  - `-D DUSE_DUAL_ABI=ON` (default OFF): See [note in GCC 5.1 release](https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html).
  - `-D Boost_NO_SYSTEM_PATHS=true` (default false): In case a local version of boost has to be used.
     Pass this option together with `-D BOOST_ROOT=PATH_where_to_find_boost`.

### Modified
- If no seed is given, the simulation will be run with `seed=Time(NULL)` (random).
- Major refactoring of the parsing machinery. :+1:  @laemmel.

### Fixed
- Fix properly delete pedestrians in `Tordeux915` (#203).
- Fix simulations are not thread-safe (#200).
- Fix memory lacks (#202).


### Bugs
Please also check the [Issue tracker](https://cst.version.fz-juelich.de/jupedsim/jpscore/issues?assignee_id=&author_id=&label_name=&milestone_id=&scope=all&sort=created_desc&state=opened), if access is available.
- exit strategies using *local* - prefix can result in circular routing
- triangulation creates error-prone help-lines in certain geometries
- errors in unused parts of ini files lead to system-exit
- exit_strategy: exit strat 9 currently bugged
- router: ff_global_shortest might not find paths in some geometries. please use exit_strategy 8
- console output: messages of only one line will be overwritten by progress bar
- agents that leave the building might produce an error-msg (although they correctly left the simulation)

## v0.8.0 [2016-02-18]


### Added
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



## v0.7.0 [2015-07-15]

### Added
- New module `JuPeditor`: Editor for the geometry
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

### Changed
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
