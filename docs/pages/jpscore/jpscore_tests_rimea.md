---
title: RiMEA
keywords: simulation  validation
tags: [jpscore, test]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_tests_rimea.html
summary: Guideline for Microscopic Evacuation Analysis according to RiMEA. The notes give hints about specific implementation in JuPedSim.

last_updated: Dec 31, 2019
---




All these tests are described in more details in  [Guidlines v3.0](https://rimeaweb.files.wordpress.com/2016/06/rimea_richtlinie_3-0-0_-_d-e.pdf).


## Test 1
One pedestrian is moving along a corridor.
Test if pedestrian can maintain a constant speed  in a straight corridor.

## Test 2
One pedestrian moving on a 10m long stair.
It should be shown that the pedestrian can maintain its speed constant.

## Test 3
One pedestrian moving on a 10m long stair downstairs.
It should be shown that the pedestrian can maintain its speed constant.

## Test 4
Show that the model can produce the shape of the fundamental diagram (density-velocity relation)
In a simplified one-dimentional corridor with closed boundary conditions.

## Test 5
Distribute 10 pedestrians with 10 different reaction times.
Verify whether they start exactly at the specified times.

## Test 6
20 pedestrians going around a corner.
Pedestrians should not cross walls.

## Test 7
Distribute pedestrian's speed according to the Table in Page 6. Four different groups are distributed according to their velocity:
1. \(v<30\)
2. \(30<v<50\)
3. \(v>50\)
4. handicapped.   
Verify whether the speed values are within the specified range.

## Test 8
A 3D building is simulated and the influence of parametes e.g. speed is investigated.
It should be shown how the evacuation time behaves with respect
to the investigated parameter. [^rimeaNote1]

## Test 9
1000 pedestrians are distributed in a room with 4 exits.
- scenario 1: All 4 exits are open
- scenario 2: 2 exits are closed. The remaining 2 are still open

The flow should nearly be doubled in scenario 1. [^rimeaNote2]

## Test 10
Pedestrians are distributed in 12 different rooms. 
The building has two exits. The Pedestrians have exactly assigned exit numbers and should evacuate through these. [^rimeaNote3]

## Test 11
300 pedestrians are distributed in a room with two exits.
The pedestrians should prefer the nearest exit, but some should (spontaneously) choose the second exit.

## Test 12
Two bottlenecks are connected with a long corridor.
At the last exit there should be no jam. [^rimeaNote4]

## Test 13
Pedestrian coming out from a bottleneck along a corridor. At the end of the corridor is a stair.
Since pedestrians have to reduce their speed on the stair, a jam should be observed at the beginning of the stair.
In case of jam, flow at the beginning of the stair should be smaller than the flow in the corridor.[^rimeaNote5] 

Therefore, we choose for $$v^0_{upstairs}$$ a Gauss-distribution with $$\mu = 0.675$$ and $$\sigma = 0.04$$.

| Handbook | Speed Stair Up |
|----------|----------------|
| PM       | 0.63 m/s       |
| WM       | 0.61 m/s       |
| NM       | 0.8 m/s        |
| FM       | 0.55 m/s       |


## Test 14
Pedestrian's evacuation to an exit. Pedestrians have two possible routes:
- short
- and long
This test has no concrete condition to check for.It should be documented whether pedestrians take a long detour or not.
There are 4 stats that should be documented:
    
1. "kurz" (*short*)
2. "lang"  (*long*)
3. "gemischt" (*mixed*)
4. "konfigurierbar" (*configurable*)

***

## References

[^rimeaNote1]: This is not a fail criterion. It is just for documentation purposes.

[^rimeaNote2]: For simplicity in JuPedSim we simulate two identical rooms:  
          - room left: with 4 exits. All of them are open  
          - room right with 4 exits. two of them are closed  
          - We write the trajectory in plain txt-format, to avoid a long lasting xml-parsing

[^rimeaNote3]: Pedestrian are assigned  to two different groups. We verify if pedestrians in the two groups pass the exits.
    In the simulation pedestrians disappear once they are outside therefore we check if a pedestrian goes through line  `exit - displacement`.

[^rimeaNote4]: The condition of this test is not clear enough...
    In the last exit there should be no jam, which means $$J_{bottleneck} >= J_{last}.$$
    However, this condition is not enough to quantify a jam.

[^rimeaNote5]: The reduced speed on stairs (up) is according to Tab 1 in [Burghardt2014][#Burghardt2014].



[#Burghardt2014]: http://link.springer.com/chapter/10.1007%2F978-3-319-02447-9_27 "Burghardt, Sebastian and Seyfried, Armin and Klingsch, Wolfram. Fundamental diagram of stairs: Critical review and topographical measurements. Pedestrian and Evacuation Dynamics 2012"
