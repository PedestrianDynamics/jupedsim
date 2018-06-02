# Change Log
All notable changes to this project will be documented in this file.

## v0.8.3 [2018-05-15]

### Added
- show name and id of rooms/subrooms in "geometry structure"
- Rename directory to `JPSvis_Files`: 94ed0a55

### Fixed
- SegFault issue #55
- Dealing with big room id numbers. #54
- Ctrl-Q functionality #51

## v0.8.0 [2016-02-18]
### Added
- option to load  vtk files. Need to add the line ``` <gradient_field filename="floorfield.vtk"> 
``` in the header of the trajectory file. Alternatively drag and drop a vtk file on JPSvis.
 
### Fixed 
- Error displaying the total number of frames when using the scroolbar

## v0.7.0 [2015-07-15]
### Added
- Display the geometry structure individual room/subroom.
- Now build on OSX/Linux/Windows 

### Fixed
- Visiblity in 3D
- Numerous geometrical operations