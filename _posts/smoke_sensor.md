**Smoke Sensor**

The smoke sensor is an input channel providing information in terms of smoke spread to the cognitive map of each pedestrian. For that purpose, CFD data from NIST's Fire Dynamics Simulator [(FDS)](http://firemodels.github.io/fds-smv/) can be processed. In the following section, a small tutorial is given:

**File Structure**

A demonstration case with pre-calculated smoke factor grids can be found in the ```demos``` directory of JPScore. However, if you want to conduct individual studies, please use the ```JPSfire``` framework. 
The ```JPSfire``` framework comprises a directory ```A_smoke_sensor``` with two subdirectories ```FDS``` and ```JuPedSim```. The first contains the fire simulation data and the latter the pedestrian simulation data. Inside ```FDS```, a subdirectory ```smoke_sensor```contains all relevant scripts for the data processing:

Script | Basic Functionality
------------- | -------------
0_slice2ascii.py | Automation of NIST's fds2ascii. Conversion of FDS-data (slicefiles) to ascii format
1_meshgrid.py | Reshape of the ascii data to the spatial extend of each FDS mesh
2_consolidate_meshes.py | Consolidation of multiple meshes to one mesh
3_process_collect.py | Calculation of the smoke factor grids that are provided to the cognitive map router
4_sfgrids.py | Visualisation of the smoke factor grids

Please note that this is an exemplary file structure. If you want to use another structure, it might be necessary to modify the path definitions in the scripts.

**Workflow**

**Pre-Processing**

Assuming the file structure of the previous section, the ```FDS``` directory contains the fire simulation data. Inside that directory, the directory ```smoke_sensor``` contains the above-mentioned scripts. All relevant information for the data processing can be specified in the head of the ```0_slice2ascii.py``` script:

```
#===================CHANGE PARAMETERS AS NECEASSARY=============================
# location of your local fds2ascii binary
fds2ascii_path = '/Applications/FDS/FDS6/bin/fds2ascii'
# Path pointing to the fire simulation directory
fds_path = os.path.join(os.getcwd()+'/../')
# Path pointing to the JuPedSim simulation directory
jps_path = os.path.join('../../JuPedSim/')

### FDS CHID:
chid = 'smoke_sensor'

### SLICE QUANTITY:
quantity = 'SOOT OPTICAL DENSITY'

### tuple containing the dimension and the location of the slicefile 
specified_location = ('z'.upper(), 2.25)

# Grid resolution in x, y and z
dx=0.25
dy=0.25
dz=0.25

# Parameters to be tunneled to fds2ascii:
# types: (slice = 2)
data_type=2
# data extend: (all = 1)
extend=1
# domain size: (not limited ='n')
domain_size="n"
# Interpolation times sequences; adjust arange values and/or window
t_start=0
t_stop=120
t_step=20
t_window=1            # interpolation duration prompted by fds2ascii

# Do you want to have plots produced? May be computaionally intensive depending
# on your FDS simulation extend!
plots = True

#===============================================================================
```

After all relevant information are specified, there are two different ways to conduct the pre-processing. Either you execute each script separately via ```python script.py``` or you execute the shell script ```sh smoke_sensor.sh```. 

Once all scripts have been executed successfully, the pre-processing is completed. Inside the ```FDS``` directory the following subdirectories have been created:

Directory | Content
------------- | -------------
0_slice2ascii | raw ascii data derived from fds2ascii
1_meshgrid | Reshaped ascii data of each FDS mesh
2_consolidated | One consolidated mesh for each time step. PDF plots of the slicefile and the line of sights from a specified point (See Debugging) (only if ```plots=True```) 
3_sfgrids | Smoke factor grids that are provided to the cognitive map router. Subdirectories: location of slicefile + resolution of the smoke factor grid + location of all processed crossings resp. transitions. PDF plots of the smoke factor grid (only if ```plots=True```)
slicefiles | PDF plots of the specified slicefiles for every single mesh (only if ```plots=True```) 

**Pedestrian Simulation**

Once the pre-processing is completed, the converted data needs to be incorporated to JPScore. The relevant information need to be specified in the JuPedSim project file. Please not that the following snippets do not represent the complete project file. Please use ```smoke_sensor_ini.xml```.

```
<agents_distribution>
    <group group_id="1" agent_parameter_id="1" room_id="0" 
    subroom_id="0" number="30" router_id="7"         
    pre_movement_mean="100" pre_movement_sigma="0" 
    risk_tolerance_mean="0." risk_tolerance_sigma="0."/>
</agents_distribution>

```

Within the ```<agents_distribution>``` section, the mean value and the standard deviation of the risk tolerance can be specified. The risk tolerance is supposed to consider the uncertainty about human behaviour when evaluating a certain state of smoke spread. This is experimental up to now - work in progress. In order to investigate the dynamic interaction between the smoke spread and the evacuation process, it may be appropriate to adjust the pre-movement times.

```
<router router_id="7" description="cognitive_map">
   <sensors>
       <sensor sensor_id="2" description="Smoke" 
          smoke_factor_grids="../FDS/3_sfgrids/Z_2.25/dx_1.00/" update_time="20" 
          final_time="120" status="activated"/>
       <sensor sensor_id="3" description="LastDestination" status="activated"/>
      </sensors>
   <cognitive_map status="complete" />
</router>
```

Within the ```<router>``` section, the smoke sensor gets initialised. The path specified as attribute of ```smoke_factor_grids``` points to the smoke factor grids calculated during the pre-processing. The tag ```update_time``` specifies the time interval for JPScore to import data from the pre-processing. The tag ```final_time``` specifies the time when no further data import will be conducted. Please note that the specified time magnitudes need to be compliant with the data generated during the pre-processing. 

**Debugging**

When encountering problems with the observed routing patterns, it is recommended to set ```plots=True``` in order to have a closer look to the results of the pre-processing. Having that, one can check if the locations of the crossings and transitions have been determined correctly. Additionally, one can specify a fixed agent position and check the course of the line of sight within ```3_process_collect.py ```. The resulting plots may look like:


**Limitations and Pitfalls**

FDS mesh resolution: Up to now, only unique mesh resolutions can be analysed.

Multiple floors: Up to now, the analyses can solely be carried out for single-floor geometries - work in progress!
 
Geometry file: It is assumed that the geometry file is located in the specified JuPedSim directory and that the filename contains the substring 'eo'.