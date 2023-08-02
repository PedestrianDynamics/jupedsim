---
title: Juelich
keywords: simulation unit test
tags: [jpscore, test]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_tests_juelich.html
summary: These tests are largely similar to the RiMEA tests. They are extendend by verification tests to verify the validy of some algorithms implemented in JuPedSim and some validation tests based on experimental data.
last_updated: Feb 17, 2020
---

## Verification
- **Test 1**: : Free flow movement in a corridor

    A pedestrian that starts in the middle of a corridor (i.e. is not influenced by the walls)
    should move with its free flow velocity towards the  exit. 


- **Test 2**: One pedestrian moving in a corridor

    Rotating the same geometry as in test 1 around the $$z-$$axis by an arbitrary angle e.g. $$45^\circ$$ should lead to the evacuation time of 10 s.

- **Test 3**: One pedestrian moving in a corridor with a desired direction

    A pedestrian is started from a random position in a holding area.
    This test should be repeated with different initial positions.

    Expected result: The pedestrians should be able to reach the marked goal in all repetitions of the test.

- **Test 4**: Single pedestrian moving in a corridor with an obstacle

    Two pedestrians are aligned in the same room. The second pedestrian from left is standing and will not move during the test.

    Expected result: Pedestrian left should be able to overtake the standing pedestrian

- **Test 5**: Single pedestrian moving in a very narrow corridor with an obstacle

    This test is Similar to test 4. Two pedestrians are aligned in the same room. The second pedestrian from left is standing and will not move during the test. The corridor is narrow and does not allow passing of two pedestians without serious overlapping.

    Expected result: Pedestrian left should stop without overlapping with the standing pedestrian.

- **Test 6**: single pedestrian moving in a corridor with more that one target

    A pedestrian is moving in a corridor with several intermediate goals.

    Expected result: The pedestrian should move through the different targets without a substantial change in its velocity i.e. with a desired speed of 1 m/s the distance of 10 m should be covered in 10 s.

- **Test 7**: route choice with different exits

    In this section the correct behavior of the implemented routing algorithms as well as the correct router-assignment are tested. The investigated geometry has three different exists.

    - Two different groups of pedestrians are randomly distributed in the inner room, such that the first group is nearer to $$E_1$$ than to  $$E_2$$. The number of pedestrians in both groups is relatively small but is equal $$(N_{\text{group 1}}=N_{\text{group 2}}=10$$). The router strategy is `local shortest`.
    - One group of pedestrians $$N = 50$$ is randomly distributed in the inner room. The router strategy is `local shortest`.
    - One group of pedestrians $$N = 50$$ is randomly distributed in the inner room. The router strategy is `global shortest`.

    Expected result: The pedestrians should be able to choose between the local shortest as well as the global shortest route.

- **Test 8**: visibility and obstacle
    The position of one pedestrian is initialized such that it has no direct view to the exit.

    Expected result: The pedestrian should avoid the obstacle and exit the room without overlapping with the obstacle.

- **Test 9**: runtime optimization using parallelism

    Implementations that make use of parallel paradigms e.g. OpenMP are tested  as follows:

    1. Distribute randomly 100 pedestrians in the gray area.
    2. Measure the flow through the bottleneck of the evacuation time i.e. the time necessary for the last pedestrian to leave the system.
    3. Repeat points 1 and 2 several times to get a certain statistical significance of the results.

    Expected results:
    - The simulation results (flow, evacuation time, ...) should be invariant with respect to the number of cores used.
    - The run time should scale with the number of cores.


- **Test 11**: Test the room/subroom construct [^note1]

    The same geometry is constructed differently
    The whole geometry is designed as a `rooms` (i.e. utility space)
    The geometry is designed by dividing the utility space in connected `subrooms`

    Distribute randomly pedestrians in all sub-rooms of the geometry and repeat the simulation to get a certain statistical significance.


    Expected results:
    The mean value of the evacuation times calculated from both cases should not differ.



- **Test 12**: Obstructed visibility

    Four pedestrians being simulated in a bottleneck. Pedestrians 0 and 1 have zero desired speed i.e. they will not move during the simulation whereas pedestrians 2 and 3 are heading towards the exit.

    The visibility between pedestrians 2 resp. 3 and 0 resp. 2 is obstructed by a wall resp. an obstacle.

    Expected results: Pedestrians 2 and 3 should not deviate from the horizontal dashed line.

- **Test 13**: Test if flow through bottleneck with respect to exit width is  comparable to empirical values.

- **Test 14**: Uniform distribution of initial positions

    The initial distribution of the pedestrian should be uniform. In a square room $$(100\times 100\,$$ $$m^2$$) 2000 pedestrians are randomly distributed. The test is repeated 1000 times.
    Divide the room equidistantly in 10 regions with respect to $$x$$- and $$y-$$ axis and count the number of pedestrians in each square.
    This count should be roughly the same in all squares:

```python
import numpy as np
import scipy, scipy.stats
import matplotlib.pyplot as plt

filename = "./path/to/file.txt"
data = np.loadtxt(filename)

x = data[:,2]
y = data[:,3]

nx = plt.hist(x, bins=10)[0]
ny = plt.hist(y, bins=10)[0]

px = scipy.stats.chisquare(nx)[1]
py = scipy.stats.chisquare(ny)[1]
```

   Expected result: The mean value of the 1000 p-values of the $$\chi^2$$-test should be around 0.5.



## Validation

- **Test 1**: 1D movement with periodical boundary

    The shape of the fundamental diagram $$(\rho, v$$) should be realistic (decreasing velocity with increasing density).

- **Test 2**: 2D unidirectional flow in corridor with periodical boundary

     The shape of the fundamental diagram $$(\rho, v$$) should be realistic (decreasing velocity with increasing density).

- **Test 3**: Unidirectional flow in corridor with open boundary

    The shape of the fundamental diagram $$(\rho, v$$) should be realistic (decreasing velocity with increasing density).

- **Test 4**: Unidirectional flow around a Corner

    The fundamental diagram is measured in two different locations. Before the corner and after the corner.
    The shape of the fundamental diagram $$(\rho, v$$) should be realistic (decreasing velocity with increasing density).

- **Test 5**: Flow through bottleneck

    The flow  $$J = N / \Delta t$$, with $$N$$ is the total number of participants in the run and $$\Delta t$$ is the time interval between the first and the last pedestrian entering the bottleneck, should increase linearly with increasing width of the bottleneck.

- **Test 6**: Merging flow in T-junction

    The fundamental diagram is measured in three different locations.
    Right and left of the T-junction and after the merging of flows.

    The shape of the fundamental diagram $$(\rho, v$$) should be realistic (decreasing velocity with increasing density).


- **Test 7** - Bidirectional flow in corridor

    The shape of the fundamental diagram $$(\rho, v$$) should be realistic (decreasing velocity with increasing density).

***

[^note1]: This test should be removed after refactoring the geometry-class. There will be not `subroom` anymore. 
