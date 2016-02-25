# JPSreport v0.8



## Added

- A switch is added in the infile for `method_D` to turn off plotting Voronoi diagrams. Now it is possible to only output data for the diagram but not plot figures.

- Switches for plotting time series of density and velocity are added for `method_C` and `method_D` in inifile.

- A switch for plotting N-t diagram is added for `method_A` in inifile.

- An option for analyzing one dimensional trajectory data is added in `method_D`.

- Issue a warning when the voronoi cell cannot be calculated.

- A warning will will be given and the program stops if trajectory for a given pedestrian ID is not continuous. 


## Changed

- Scripts "_Plot_cell_rho.py" and "_Plot_cell_v.py" are modified. Now the geometry is also plotted when plotting voronoi cells.

- The indicator for velocity component can be specified in trajectory files now (.TXT and .XML)

- Scripts "_Plot_FD.py" is modified!

## Fixed

- Output data file "Folw\_NT\_xxxx.dat" is closed before calling script for plotting N-t diagram.

- A bug relating to transformation of units in `method_B` is fixed.

- Fixed error where all trajectories were colinear.

- A bug for legend in the script "_Plot_timeseries_rho_v.py" is fixed.

- The case that frame ID and Ped ID in trajectory file are not coutinuous can also be analyzed correctly.

- Now when the given file paths in inifile include blank, it still works on windows system.

- when path of trajectory is not given absolutely, the default location is the same folder with the inifile




