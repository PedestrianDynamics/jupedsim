---
title:  jpsreport How-to
keywords: analysis
tags: [jpsreport, jpsreport_tutorial]
sidebar: jupedsim_sidebar
folder: jpsreport
summary: Analysis of a bottleneck scenario from the demo files of jpsreport
permalink: jpsreport_howto.html
last_updated: Dec 21, 2019
---

{%include note.html content="This howto can be downloaded as a [jupyter notebook](https://raw.githubusercontent.com/JuPedSim/jpscore/master/jpsreport/demos/HowTo.ipynb).
The files are from the [bottleneck demo](https://raw.githubusercontent.com/JuPedSim/jpscore/master/jpsreport/demos/bottleneck)"%}

## Bottleneck example

`JPSreport` comes with some demo files. 

For example in **demos/bottleneck** we can find the following files:


```bash
%%bash
echo "Actual directory:  $PWD"
ls bottleneck
```

    Actual directory:  /Users/chraibi/Workspace/jpsreport/demos
    Output
    geo_AO_300.xml
    geometry.png
    ini_AO_300.xml
    traj_AO_300.txt


## Starting JPSreport

Calling `JPSreport` without an inifile yields:


```bash
%%bash
../bin/jpsreport
```

    INFO: 	Trying to load the default configuration from the file <ini.xml>
    ----
    JuPedSim - JPSreport
    
    Current date   : May 19 2018 09:09:49
    Version        : 0.8.3
    Compiler       : clang++ (3.5.0)
    Commit hash    : v0.8.2-76-g561ab10-dirty
    Commit date    : Fri May 18 18:40:14 2018
    Branch         : develop
    ----
    
    INFO: 	Parsing the ini file <ini.xml>
    Usage: 
    
    ../bin/jpsreport inifile.xml
    


    ERROR: 	Failed to open file
    ERROR: 	Could not parse the ini file


----

`JPSreport` rightly complains about a missing inifile. 

In the bottleneck directory, we can find the three necessary files to get us started. 

Let's visualize the geometry with [JPSvis](https://gitlab.version.fz-juelich.de/jupedsim/jpsvis)

![geometry](./bottleneck/geometry.png)


## Preparing the inifile

here you may want to start with a demo inifile and check the online [documentation](http://www.jupedsim.org/jpsreport/2016-11-01-inifile)
for options. 

## Call JPSreport

To start the analysis run from a terminal the following:



```bash
%%bash
echo "Actual directory:  $PWD"

../bin/jpsreport ./bottleneck/ini_AO_300.xml
```

    Actual directory:  /Users/chraibi/Workspace/jpsreport/demos
    ----
    JuPedSim - JPSreport
    
    Current date   : May 19 2018 09:09:49
    Version        : 0.8.3
    Compiler       : clang++ (3.5.0)
    Commit hash    : v0.8.2-76-g561ab10-dirty
    Commit date    : Fri May 18 18:40:14 2018
    Branch         : develop
    ----
    
    INFO: 	Parsing the ini file <./bottleneck/ini_AO_300.xml>
    lineNr 100000
    INFO: 	End Analysis for the file: traj_AO_300.txt
    Time elapsed:	 4.09673 [s]


The output of `JPSreport` gives some logging information on its running process.  It may inform on any errors during the compilation or misconception of the input files (inifile, geometry or trajectory file).

In the inifile you can define a logfile to direct these messages.

Uppon a succesful run, the directory **output** is created with the following content (depends on the configuration in inifile):


```bash
%%bash
tree bottleneck/Output
```

    bottleneck/Output
    └── Fundamental_Diagram
        └── FlowVelocity
            ├── FDFlowVelocity_traj_AO_300.txt_id_2.dat
            ├── FDFlowVelocity_traj_AO_300.txt_id_4.dat
            ├── Flow_NT_traj_AO_300.txt_id_2.dat
            ├── Flow_NT_traj_AO_300.txt_id_2.png
            ├── Flow_NT_traj_AO_300.txt_id_4.dat
            └── Flow_NT_traj_AO_300.txt_id_4.png
    
    2 directories, 6 files



```bash
%%bash
tail bottleneck/Output/Fundamental_Diagram/FlowVelocity/FDFlowVelocity_traj_AO_300.txt_id_2.dat
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


![result 1](bottleneck/Output/Fundamental_Diagram/FlowVelocity/Flow_NT_traj_AO_300.txt_id_2.png)

Result: 
![](bottleneck/Output/Fundamental_Diagram/FlowVelocity/Flow_NT_traj_AO_300.txt_id_2.png)

## Problems

In case something does not work, which may happen of course, please open an issue.

Use for this purpose our [issue-tracker](http://gitlab.version.fz-juelich.de/jupedsim/jpsreport/issues). 
