---
title:  jpsreport How-to
keywords: analysis
tags: [jpsreport, jpsreport_tutorial]
sidebar: jupedsim_sidebar
folder: jpsreport
summary: Analysis of a bottleneck scenario from the demo files of jpsreport
permalink: jpsreport_howto.html
last_updated: Dec 22, 2019
---

{%include note.html content="This howto can be downloaded as a [jupyter notebook](https://github.com/JuPedSim/jpscore/blob/master/jpsreport/demos/HowTo.ipynb).
The files are from the [bottleneck demo](https://github.com/JuPedSim/jpscore/tree/master/jpsreport/demos/bottleneck)"%}



## Getting started with JPSreport


`JPSreport` needs three different files as input:
- A trajectory file (txt or xml) (see [documentation](jpscore_trajectory.html))
- A geometry file (see [documentation](jpscore_geometry.html))
- and a project file, called inifile (see [documentation](jpsreport_inifile.html))

## Bottleneck example

`JPSreport` comes with some demo files. 

For example in **demos/bottleneck** we can find the following files:


```bash
%%bash
echo "Actual directory:  $PWD"
ls bottleneck
```

    Actual directory:  /Users/chraibi/Workspace/github/jpscore/jpsreport/demos
    geo_AO_300.xml
    geometry.png
    ini_AO_300.xml
    traj_AO_300.txt


## Running jpsreport
For the rest, it is assumed that the executable file lives in `../../build/bin`.
In the following cell you may want to change the value of `dir_binary` in order to match the directory of your executable.

Calling `JPSreport` without an inifile yields:


```bash
%%bash
dir_binary="../../build/bin"
$dir_binary/jpsreport
```

    INFO: 	Trying to load the default configuration from the file <ini.xml>
    ----
    JuPedSim - JPSreport
    
    Current date   : Sun Dec 22 16:19:01 2019
    Version        : 0.8.4
    Compiler       : clang++ (11.0.0)
    Commit hash    : v0.8.4-893-gf1fa6bc5-dirty
    Commit date    : Sat Dec 14 14:36:57 2019
    Branch         : 614_documentation
    Python         : /usr/local/bin/python3 (3.7.4)
    ----
    
    INFO: 	Parsing the ini file <ini.xml>
    Usage: 
    
    ../../build/bin/jpsreport inifile.xml
    


    ERROR: 	Failed to open file
    ERROR: 	Could not parse the ini file


----

`JPSreport` rightly complains about a missing inifile. 

## Preparing the inifile

here you may want to start with a demo inifile and check the online [documentation](jpsreport_inifile.html)
for options. 

## Call JPSreport with inifile

To start the analysis run from a terminal the following:



```bash
%%bash
dir_binary="../../build/bin"
echo "Actual directory:  $PWD."
echo "dir_bin: $dir_binary"
$dir_binary/jpsreport ./bottleneck/ini_AO_300.xml
```

    Actual directory:  /Users/chraibi/Workspace/github/jpscore/jpsreport/demos.
    dir_bin: ../../build/bin
    ----
    JuPedSim - JPSreport
    
    Current date   : Sun Dec 22 16:19:36 2019
    Version        : 0.8.4
    Compiler       : clang++ (11.0.0)
    Commit hash    : v0.8.4-893-gf1fa6bc5-dirty
    Commit date    : Sat Dec 14 14:36:57 2019
    Branch         : 614_documentation
    Python         : /usr/local/bin/python3 (3.7.4)
    ----
    
    INFO: 	Parsing the ini file <./bottleneck/ini_AO_300.xml>
    INFO:	logfile </Users/chraibi/Workspace/github/jpscore/jpsreport/demos/bottleneck/log_AO_300.txt>
    lineNr 100000
    INFO:	Success with Method A using measurement area id 2
    INFO:	Success with Method A using measurement area id 4
    INFO: 	End Analysis for the file: traj_AO_300.txt
    Time elapsed:	 4.32952 [s]


The output of `JPSreport` gives some logging information on its running process.  It may inform on any errors during the compilation or misconception of the input files (inifile, geometry or trajectory file).

In the inifile you can define a logfile to direct these messages.

Uppon a succesful run, the directory **output** is created with the following content (depends on the configuration in inifile):


```bash
%%bash
tree bottleneck
```

    bottleneck
    ├── bottleneck_Output
    │   └── Fundamental_Diagram
    │       └── FlowVelocity
    │           ├── FDFlowVelocity_traj_AO_300.txt_id_2.dat
    │           ├── FDFlowVelocity_traj_AO_300.txt_id_4.dat
    │           ├── Flow_NT_traj_AO_300.txt_id_2.dat
    │           └── Flow_NT_traj_AO_300.txt_id_4.dat
    ├── geo_AO_300.xml
    ├── geometry.png
    ├── ini_AO_300.xml
    ├── log_AO_300.txt
    └── traj_AO_300.txt
    
    3 directories, 9 files



```bash
%%bash
tail bottleneck/bottleneck_Output/Fundamental_Diagram/FlowVelocity/FDFlowVelocity_traj_AO_300.txt_id_2.dat
```

    #Flow rate(1/s)		 Mean velocity(m/s)
    8.404	1.148
    7.673	0.869
    7.347	0.774
    7.216	0.723
    7.129	0.736
    6.880	0.726
    6.275	0.740
    3.918	0.748
    2.500	0.806


## Problems

In case something does not work, which may happen of course, please open an issue.

Use for this purpose our [issue-tracker](http://github.com/jupedsim/jpscore/issues). 
