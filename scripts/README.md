## dxf-file constraints for using the parse_dxf script
* outer and inner polygon are defined on different layers
* the definition of the inner polygon (holes) can be distributed on several layers
* the definition of the outer polygon must be on one layer // not necessary but less confusing
* a hole is defined with **one** polyline or **one** circle
  * there are no holes, which are composed of multiple lines/polylines
  * keep in mind that a circle is considered a polygon with many corners. this might make the triangulation too accurate and slow down the simulation unnecessarily.
  * circles touching other elements might be approximated different than expected
* Holes may overlap
* Holes may be defined outside the outer polygon
  * this does not change the structure of the polygon
* the outer polygon must be defined with one closed polyline
* if there are multiple polygons on the outer polygon a MultiPolygon will be parsed

The polygon should end up looking like the structure in the dxf file:

* There must be no gaps in the outer polygon or in a hole
* There must be no double lines
* all areas must be wide enough for the agents
* all polygons must be simple