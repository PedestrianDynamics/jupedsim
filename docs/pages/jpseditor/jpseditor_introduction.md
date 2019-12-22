---
title: "jpseditor: editing tool for JuPedSim"
keywords: geometry
tags: [getting_started]
sidebar: jupedsim_sidebar
permalink: jpseditor_introduction.html
summary: A graphical user interface to create the geometry of a scenario simulated by jpscore. It comes with set of CAD- and further tools to simplify the creation of proper xml-files incorporating information about the scenario geometry.
last_updated: 21, December 2019
toc: true
---

{%include note.html content="jpseditor is still buggy and crashes often."%}

## Drawing a geometry from scratch

In jpseditor there are three areas: 
- Menu Bar (with red frame): available functions of jpseditor
- Tool Bar(with blue frame):  tools/elements needed for drawing (walls, doors, ...)
- canvas.(grid area)

![jpseditor layout]({{ site.base }}/images/jpseditor_surface.png)

## Tutorials 
- [Tutorial - part 1](https://youtu.be/fk4IgG1Mg4U)
- [Tutorial - part 2](https://youtu.be/B8sPswlcOT8)
- [Tutorial - part 3](https://youtu.be/wivJHtGc6Mw)

## Demos 

- [JuPedSim Geometry Editor Tutorial](https://www.youtube.com/watch?v=DR_3TX8MFxM&t=25s)
- [3D room with jpseditor](https://www.youtube.com/watch?v=wK_atSk-xWE)
- [Stairs with jpseditor](https://www.youtube.com/watch?v=DKnJHToLAXs&t=304s)
- [Stairs with jpseditor: automatic room detection](https://www.youtube.com/watch?v=Id5Rfli7UYk)


## Old geometry format 

Some old geometries can not be parsed properly with `jpseditor`.

**Example**  

of a geometry that can not be edited properly:

```xml
<polygon>
    <vertex px="-9.0" py="-1.0"/>
    <vertex px="-9.0" py=" 1.5"/>
    <vertex px="-8.0" py=" 1.5"/>
    <vertex px="-8.0" py=" 0.0"/>
    <vertex px=" 9.0" py=" 0.0"/>
    <vertex px=" 9.0" py="-1.0"/>
    <vertex px="-9.0" py="-1.0"/>
</polygon>
``` 

**Solution** 

Change the above structur by  making sure that you use the correct grammar for `jpseditor`. Meaning, every polygon should and only can have **two vertexes**, like this:

```xml
<polygon>
    <vertex px="-9.0" py="-1.0"/>
    <vertex px="-9.0" py=" 1.5"/>
</polygon>
<polygon>
    <vertex px="-9.0" py=" 1.5"/>
    <vertex px="-8.0" py=" 1.5"/>
</polygon>
<polygon>
    <vertex px="-8.0" py=" 1.5"/>
    <vertex px="-8.0" py=" 0.0"/>
</polygon>
<polygon>
    <vertex px="-8.0" py=" 0.0"/>
    <vertex px="9.0" py=" 0.0"/>
</polygon>
<polygon>
    <vertex px="9.0" py=" 0.0"/>
    <vertex px="9.0" py="-1.0"/>
</polygon>
<polygon>
    <vertex px="9.0" py="-1.0"/>
    <vertex px="-9.0" py="-1.0"/>
</polygon>
```

## Issue tracker 

`jpseditor` is instable and tend to crash more than often. If you encounte any problem, please open an issue using the [issue tracker](https://github.com/JuPedSim/jpseditor/issues)

{%include tip.html content="jpseditor produces logs. Please upload these too along with your issue."%}

{% include links.html %}
