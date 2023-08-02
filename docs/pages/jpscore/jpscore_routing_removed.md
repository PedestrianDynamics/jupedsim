---
title: Way finding
keywords: simulation
tags: [jpscore, model, removed]
sidebar: jupedsim_sidebar
folder: jpscore
permalink: jpscore_routing_removed.html
summary: Here the removed routing algorithms should be documented
last_updated: Dec 21, 2019
---


{%include note.html content="this page is unpublished and is related to [routing models](jpscore_routing.html)"%}






## AI router: Introduction and reference
The idea behind the AI Router is to provide an alternative to conventional routing algorithms such as shortest path algorithms by considering human way-finding abilities in a more detailed manner. For this purpose, the AI router introduces way-finding strategies and tools such as the cognitive map, the usage of signs, the consideration of generalized knowledge and search strategies.


The underlying concepts and algorithms have been published in this [paper](https://www.tandfonline.com/doi/abs/10.1080/23249935.2018.1432717)

## Basic usage

All files (ini and geometry, etc.) used for this tutorial are linked [HERE](https://fz-juelich.sciebo.de/s/KsSsp5UXb3TSm8u).

For the sake of simplicity let us assume the following situation: 
There is a single pedestrian in a very simple fictional building trying to find its way out. 
The building only consists of a room and a connected corridor with two exits on the left and on the right sides.

![Geometry]({{ site.baseurl }}/images/ai_router1.png)

Jon Snow from Game of Thrones probably knows every way leading out of Castle Winterfell. However, for the sake of this example, let us assume that our pedestrian is a first time visitor in our example building. He/she absolutely forgot about the way he/she came in. So he/she has no clue where to find an exit. There are no signs in the building. So there is nothing left to do than searching a way by deciding randomly at each decision point. All we need to cover this scenario is to set the following settings at the route choice models - section at the very end of the ini-file:

```xml
<route_choice_models>
 <router router_id="7" description="AI">
   <cognitive_map status="empty">
 </router>
</route_choice_models>
```

with this configuration we can let the pedestrian search randomly for an exit. 

![Geometry]({{ site.baseurl }}/images/ai_router2.png)

## Example 2 - Be fair / give a hint
For the second example, let us assume that our pedestrian now can remember something of the way he/she came in. He/she just remembers that the exit he/she used to enter the building must be somewhere on the left side (when facing the crossing to the corridor).

Thus, we provide the pedestrian with a cognitive map containing one single element that represents the information about the location of the used entrance/exit in his/her mind. For this purpose, we set up a new separate xml-file holding the information in the pedestrian‘s cognitive map. For every pedestrian group in the ini-file it is possible to set up an individual cognitive map file. We only have a single pedestrian group. The group with the ID 0. 
So we name the file as `cognitive_map0.xml`. 
The number before the postfix of the file refers to the ID of the pedestrian group. (Accordingly, it would be cogntive_map1.xml if we had another pedestrian group (with ID 1) and would like to set up a cognitive map file for this group.) Within the cognitive map file, we need the following information (see Fig. ).

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cognitiveMap version="0.81" caption = "cogMap" xmlns:xsi = "http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://xsd.jupedsim.org/jps_geometry.xsd" unit="m">
	<regions>
		<region id="0" caption="Region0" px="0.0" py="0.0" a="40.0" b="40.0">
			<landmarks>
				<landmark id="0" caption="Landmark0" type="main" pxreal="0.5" pyreal="1.5"
 						px="0.5" py="1.5" a="2.0" b="5.0">
					<associations/>
				</landmark>
			</landmarks>
			<connections/>
		</region>
	</regions>
</cognitiveMap>
```

Besides the header we specify a region of landmarks and destinations in the cognitive map file. The specifications of the region are not relevant in our case and can be arbitrarily. So, for example, we can say that the region has its center at position (0,0) and an expansion of ca. 40 times 40 (a and b equal 40) meters. So it incorporates all parts of our geometry. As an item of this region we specifiy a landmark that is supposed to represent the remembered exit. The type has to be "main" as the remembered exit is to be regarded as a final (not an intermediate) destination. Real position of the landmark (pxreal and pyreal) equal the real position of the exit. The position of the landmark in pedestrain's mind (px and py) should equal the (real) center of the exit approximately but can deviate from that position. The greater the deviation the "fuzzier" the memory in the pedestrian‘s mind. The expansion of the landmark (a and b) can be set arbitrarily. Again, the greater the expansion the fuzzier the memory. For now, let us specify an expansion of 2 times 5 meters. There are neither associations nor connections between landmarks, so these specifications can remain empty. 

```xml
<route_choice_models>
 <router router_id="7" description="AI">
   <cognitive_map files="./cognitive_map.xml"/>
 </router>
</route_choice_models>
```

The last thing we need to do now is to specify the name of our cognitive map file in the route choice model – section of the ini-file (see Fig. 4). Here, we leave out the number that refers to the pedestrian group. No matter how many cognitive map files for pedestrian groups exist we only have to mention the name (without the number) of the files once. Make sure that the cognitive map files are stored in the same folder as the ini-file. Otherwise, provide with the names with their complete paths.

## Example 3 – Déjà vu – Stuck in a dead end
As you might know déjà vus are often errors in the matrix which can occur when the machines change something. So just like it happened to Neo and his friends we assume now that our pedestrian sees twice a black cat walking by and suddenly one of the exits turnes into a wall:

![Geometry]({{ site.baseurl }}/images/ai_router3.png)

Without any knowledge or rather cognitive map our pedestrian either (with the probability of 50 %) goes to the left and finds the exit directly or turns to the right side of the corridor, finds himself stuck in a dead end, turns around and proceeds to the left side of the corridor where he finds the exit eventually.

For this purpose, we create a new file called signage.xml and fill in the following information:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<signage version="0.81" caption="signage" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
xsi:noNamespaceSchemaLocation="http://xsd.jupedsim.org/jps_geometry.xsd" unit="m">
    <signs>
        <sign id="0" caption="Sign0" room_id="0" px="4.5" py="6.25" alphaPointing="180.0" alpha="270.0" />
    </signs>
</signage>
```

Within the signs section we specify a sign (id 0 and caption Sign0) at the position (4.5, 6.25) (global coordinates of the geometry). This position is in the corridor on the opposite side of the crossing the  room where the pedestrian starts. We only have one room (consisting of two subrooms). So the sign is in the room with id 0. The sign faces to this crossing (downwards in the floor plan) as the parameter alpha is set to 270 degrees. 0 degree is the direction that is parallel to the global x-axis, 90 degrees is the direction that is equal to the global y-axis. The arrow on the sign that shows the way to the outside points to the left side of the corridor as alphaPointing is set to 180 degrees.

Finally, we need to mention the signage file in the router section of the ini-file:

```xml
<route_choice_models>
  <router router_id="7" description="AI">
    <cognitive_map status = "empty"/>
    <signage file="signage.xml"/>
  </router>
</route_choice_models>
```

Please note that is not guaranteed that the pedestrian percepts the sign (probability has been set to round about 70% following a study in a subway station.


## Parameter
### Inifile - router section:
- `router`
   - `router_id` = arbitrary (unique) integer
   - `description` = "AI"
- `cognitive_map` 
   - `status` = "empty" 
   - `files`= arbitrary string
- `signage`
   - `file`= arbitrary string

### Cognitive map files:
- `regions`
   - `region`
      - `id` = arbitrary (unique) integer
      - `caption` = arbitrary string
      - `px` = arbitrary double (Center of region (x))
      - `py` = arbitrary double (Center of region (y))
      - `a` = arbitrary double (Expansion of region in x-direction)
      - `b` = arbitrary double (Expansion of region in y-direction)
- `landmarks`
   - `landmark`
      - `id` = arbitrary (unique) integer
      - `caption` = arbitrary string
      - `type` = "main" (main destination OR "landmark" (normal landmark)
      - `pxreal` = arbitrary double (real position of landmark (x))
      - `pyreal` = arbitrary double (real position of landmark (y))
      - `px` = arbitrary double (position of landmark in cognitive map (x))
      - `py` = arbitrary double (position of landmark in cognitive map (y))
      - `a` = arbitrary double (Expansion of landmark in x-direction)
      - `b` = arbitrary double (Expansion of region in y-direction)
- `connections`
   - `connection`
      - `id` = arbitrary (unique) integer
      - `caption` = arbitrary string
      - `landmark1_id` = ID of existing landmark
      - `landmark2_id` = ID of existing landmark

### Signage file:
- `signs`
   - `sign`
      - `id` = arbitrary (unique) integer
      - `caption` = arbitrary string
      - `room_id` = ID of existing room
      - `px` = arbitrary double (Position of sign (x))
      - `py` = arbitrary double (Position of sign (y))
      - `alphaPointing` = double \[0.0, 360.0\] (direction to which the arrow on the sign points)
      - `alpha` = double \[0.0, 360.0\] (direction to which the readable side of the sign heads)
