# Change Log
All notable changes to `jpsreport` will be documented in this file.

## v0.8.5 [08.06.2020]

### Added

### Changed
- Use common Logger which is defined in /libshared 
- File with logging data can be created by piping the console output to a file

### Deprecated
- Definition of logfile in .ini file is not used anymore

### Removed
- Removed OutputHandler since it was used for logging only
- Removed read in and processing of logfile options

### Fixed


## v0.8.5 [04.11.2019]

### Added
- Method J: calculation of Voronoi density and spatial mean velocity

### Changed
- Update demo-files
    - bottleneck
    - corridor
    - corridor_with_obstacle
    - crossing
    - T-Junction

### Deprecated

### Removed
- Removed unused code
- Removed option to plot timeseries for Methods A, B, D and I
- Removed option to plot Voronoi diagrams with index

### Fixed
- Cmake for macOS 10.15 (Catalina)
- A bug calling `method_B` is fixed.

## v0.8.4 [10.09.2019]
### Added
- Option brew install ([for more information](https://github.com/JuPedSim/homebrew-jps/blob/master/README.md))
- Option --version or -version
- Option to define a customized output directory
  ```xml
     <output location="path/to/directory"/>
  ```
- Python script for trajectory correction (`correct_trajectories.py`)
- Python detected by cmake
- Python interpreter and version to log
- Version information: definitions for interpreter and its version
- Method for calculation for time-series analysis of individual data (see [Method_I](http://www.jupedsim.org/jpsreport/2016-11-01-inifile.html#method-i)) and output data format.
- Implementation of Travis CI
- Qualitative Utest for methods A-D and I


### Changed
- Add elapsed time and reduce print logs per frame
- deactivate static linking
- use c++17 for filesystem functionality.
- Modification of calculation of Voronoi Density (see [Method_D](http://www.jupedsim.org/jpsreport/2016-11-01-inifile.html#method-d)) and output data format.


### Deprecated

### Removed

### Fixed
- Fix for VisualStudio (Windows)
- Better handling negative frame id
- Fix for polygon output considering interior and exterior rings
- Fix if no measurement area is selected
- Fix if no measurement method is selected
-


## v0.8.3 [16.05.2018]
### Added
- Option to output log in a file instead of the screen fe66fa49
  ```
  <logfile>log.txt</logfile>
  ```
- Output useful debug information like date, git and compiler versions. !6 and discussion in #79
- Option to plot Voronoi diagrams with index instead of little blue circles `plot_index`. Use as:
  ```xml
  <output_voronoi_cells enabled="true" plot_graphs="true" plot_index="true"/>
  ```
- new format of returned polygons `"index | polygon"` 6fa459ad9ffe5a07699c05b655bcf90f114ed635
- Exit if `framerate` is not found. c1308ef8

### Changed
- Pass Matrix by reference bff89e48
- Better fonts in plot scripts. 56d6a8f7


### Fixed
- Ignore empty line in traj file. 3a3ae04e
- Fixes for profile plots. a8a1414c
- Fix parsing of framerate. 2ad0b01d

## v0.8.2 [06.11.2017]

### Added

- Use the effective distance in method B in case `length_in_movement_direction` is not defined.  (2c321cef)
- Added an error warning when the number of agents in the trajectory is not corresponding to total ids or the ped ids are not continuous in the first frame.

### Changed
- Code does not come with Boost anymore. User should install Boost before using jpsreport. (2c0c69f3)
- use boost matrix instead of double pointers (9ff5c978)
- Use own index numbers (9a0d8ec8)
- Use Python3 in scripts.

### Fixed
- Fix SegFault due to reading files from different OS. (9a42c9dd)

## v0.8.1 [11.10.2016]

### Added

- Two options `startframe` and `stopframe` are added for each measurement area for method D to assign the time periods for analysis.

- Individual density based on Voronoi method is added for one dimensional case in the output file (Individual headway is moved to the 5th column).

- z-position of each measurement area can be assigned in inifile so that the trajectories in geometries with several floors can be analyzed.

- The option `plot_time_series` is available for each measurement area.

- The option `frame_interval` for method A now can have different values for different measurement area.

- The option `ignore_backward_movement` and `set_movement_direction` are added to indicate the movement direction for velocity calculation.

### Changed

- The switch for calculating Individual FD is arraged for each measurement area.

- The setting for velocity calculation is changed in inifile. Now velocity can be calculated by projecting to any direction by setting the parameter `set_movement_direction`. The backward movement against the target direction can be considered or removed by setting the parameter `ignore_backward_movement`.

- The way for reading .txt format trajectory file is changed. Now the order of each column in trajectory file is not so important. The trajectory file from JPScore can be analyzed directly.

- The algorithm for loading the '.txt' format trajectory file is modified. Now the order of each column in the file is not so important. JPSreport will search for meaning of each column from the comments (for example `#ID FR X Y Z VD`).

### Fixed

- The script for plot Voronoi cells is modified so that it work when the trajectory files are not the in the same location as the inifile.

- The bug related to #54 is fixed.

- The bug regarding to #43 is fixed.

## v0.8


### Added

- A switch is added in the infile for `method_D` to turn off plotting Voronoi diagrams. Now it is possible to only output data for the diagram but not plot figures.

- Switches for plotting time series of density and velocity are added for `method_C` and `method_D` in inifile.

- A switch for plotting N-t diagram is added for `method_A` in inifile.

- An option for analyzing one dimensional trajectory data is added in `method_D`.

- Issue a warning when the voronoi cell cannot be calculated.

- A warning will will be given and the program stops if trajectory for a given pedestrian ID is not continuous.


### Changed

- Scripts "_Plot_cell_rho.py" and "_Plot_cell_v.py" are modified. Now the geometry is also plotted when plotting voronoi cells.

- The indicator for velocity component can be specified in trajectory files now (.TXT and .XML)

- Scripts "_Plot_FD.py" is modified!

### Fixed

- Output data file "Folw\_NT\_xxxx.dat" is closed before calling script for plotting N-t diagram.

- A bug relating to transformation of units in `method_B` is fixed.

- Fixed error where all trajectories were co-linear.

- A bug for legend in the script "_Plot_timeseries_rho_v.py" is fixed.

- The case that frame ID and Ped ID in trajectory file are not coutinuous can also be analyzed correctly.

- Now when the given file paths in inifile include blank, it still works on windows system.

- when path of trajectory is not given absolutely, the default location is the same folder with the inifile


## v0.7

### Added

- Added four demos as examples for using JPSreport
- Added the option for specifying the location of scripts in configuration file.
- Embedded python scripts (**\_Plot_N\_t.py**, **\_Plot_timeseries\_rho_v.py**) for plotting N-t diagram (Method A), time series of density/velocity diagram (Method C and D) and Voronoi diagrams (Method D).
- Added python script (**SteadyState.py**) for automatically detecting steady state of pedestrian flow based on time series of density and velocity. When plotting fundamental diagrams normally only data under steady state are used due to its generality.
- Added python script (**\_Plot_FD.py**) for plotting fundamenatl diagram based on the detected steady state.

### Changed

- Changed name of some variables in configuration file:

    **measurementAreas**                --->  **measurement_areas**

    **Length_in_movement_direction**	---> **length_in_movement_direction**

    **useXComponent**                   ---> **use_x_component**

    **useYComponent**                   ---> **use_y_component**

    **halfFrameNumberToUse**            ---> **frame_step**

    **timeInterval**                    ---> **frame_interval**

    **measurementArea**                 ---> **measurement_area**

    **outputGraph**                     ---> **output_graph**

    **individualFDdata**                ---> **individual_FD**

    **cutByCircle**                     ---> **cut_by_circle**

    **getProfile**                      ---> **profiles**

    **scale_x**                         ---> **grid_size_x**

    **scale_y**                         ---> **grid_size_y**
- Changed the data type of frame rate (fps) from integer to float

- Changed the way for dealing with pedestrian outside geometry. In old version JPSreport stops when some pedestrians are outside geometry but now it continue working by
removing these pedestrians from the list.

- More than one sub rooms in one geometry can be analysed independently.

### Fixed

- Fixed bug for dealing with obstacles inside geometry.
