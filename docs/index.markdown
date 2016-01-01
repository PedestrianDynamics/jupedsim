# JPScore

Core module for simulation and computing of trajectories.
<!-- <img alt="JuPedSim" title="Logo by Nadine" src="img/logo.png" width="100px"  align="right"> -->

[![build status](https://travis-ci.org/JuPedSim/jpscore.svg?branch=v0.8)](https://travis-ci.org/JuPedSim/jpscore)
[![codecov.io](https://codecov.io/github/JuPedSim/jpscore/coverage.svg?branch=v0.8)](https://codecov.io/github/JuPedSim/jpscore?branch=v0.8)
___
**Note**: When reporting bugs, please make sure to use the development branch v0.8.
___

JuPedSim comes with four loosely coupled modules: 

1. JPScore: The simulation core
2. [JPSvis](https://cst.version.fz-juelich.de/jupedsim/jpsvis): Geometry and trajectories visualization tool
3. [JPSreport](https://cst.version.fz-juelich.de/jupedsim/jpsreport): Implementation of different measurement methods to evaluate out of trajectories the density, the velocity and the flow.
3. [JPSreditor](https://cst.version.fz-juelich.de/jupedsim/jpsreditor): Geometry editor.





## Usage
`jpscore` is called  with initialization files (inifiles):

```
 jpscore config.xml
```

with `config.xml` a structured `xml` project file containing parameter definitions, agents distributions and other 
important configurations to run a simulation.

<!-- <p> -->
<!-- <h1 style="position: absolute; -->
<!--     width: 1px; -->
<!--     height: 1px; -->
<!--     padding: 0; -->
<!--     margin: -1px; -->
<!--     overflow: hidden; -->
<!--     clip: rect(0,0,0,0); -->
<!--     border: 0;">JuPedSim</h1> -->
<!-- <a href="http://www.youtube.com/watch?v=t_9MC-o_aGM"> -->
<!-- <img alt="Django REST Framework" title="Logo by Nadine" src="http://img.youtube.com/vi/t_9MC-o_aGM/0.jpg" width="400px" style="display: block; margin: 0 auto 0 auto"> -->
<!-- </a> -->
<!-- </p> -->
Everybody is welcome to contribute to the project, but first have a look at

