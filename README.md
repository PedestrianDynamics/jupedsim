
JPSreport v0.7
================

Running
=======

>> ./JPSreport.exe ./xxx/xxx/ini_bottleneck.xml"

From the command line trajectory files mentioned in the inifile will be analysed by using the methods and parameters in the inifile.
The output results will be saved in the sub-folder of the folder where the inifile exists.

### Added

- Added four demos as examples for using JPSreport
- Added the option for specifying the location of scripts in configuration file.
- Embedded python scripts for plotting N-t diagram (Method A), time series of density/velocity diagram (Method C and D) and Voronoi diagrams (Method D).

### Changed

- Changed name of some variables in configuration file.
	measurementAreas   --->  measurement_areas
	Length_in_movement_direction	---> length_in_movement_direction
	useXComponent		---> use_x_component
	useYComponent		---> use_y_component
	halfFrameNumberToUse  ---> frame_step
	timeInterval	---> frame_interval
	measurementArea	---> measurement_area
	outputGraph	---> output_graph
	individualFDdata	---> individual_FD
	cutByCircle 	---> cut_by_circle
	getProfile 		---> profiles
	scale_x			---> grid_size_x
	scale_y			---> grid_size_y
- Changed the data type of frame rate (fps) from integer to float

- Changed the way for dealing with pedestrian outside geometry. In old version JPSreport stops when some pedestrians are outside geometry but now it continue working by 
removing these pedestrians from the list.

- More than one sub rooms in one geometry can be analysed independently.
	
### Fixed
	
- Fixed bug for dealing with obstacles inside geometry.










  
