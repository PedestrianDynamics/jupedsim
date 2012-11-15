Usage:

Use left click to draw a vertex, and right click to finish a polygon.

In the left toolbar, there are ten icons.

The upper three icons are Minimum Convex Decomposition, Minimum Weight Triangulation, and Visibility Polygon. When you finished the polygon, you can click these icons to check the result.

The middle three icons are Animating, Clear and Show Vertex. You can click Animating icon to see the animating of the result (The animating of MCD is not finished), click Clear icon to remove the polygon, and click Show Vertex icon to see the vertex(The vertics are CW).

The bottom four icons are not implemented yet.



Structure:
AlgorithmBase: base class for AlgorithmMCD, AlgorithmMWT, and AlgorithmVP.
ALgorithmMCD: compute and draw minimum convex decomposition
AlgorithmMWT: compute and draw minimum weight triangulation
AlgorithmVP:  compute and draw visibility polygon

CGeomBase:      base class for CGeomHomog, CGeomPairDeque, CGeomPoly, CGeomVector.
CGeomHomog:     Homog related computing
CGeomPairDeque: implementing the pair deque.
CGeomPoly:      polygon and polyline related computing
CGeomVector:    vector related computing

CDrawBase: base class for CDrawPoly.
CDrawPoly: draw the polygon.