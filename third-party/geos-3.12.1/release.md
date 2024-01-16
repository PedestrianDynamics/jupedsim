
2023-11-11

- Fixes:
  - Remove undefined behaviour in use of null PrecisionModel (GH-931, Jeff Walton)
  - Explicitly set endianness for some tests so that output matches expected (GH-934, Paul Ramsey)
  - Fix IncrementalDelaunayTriangulator to ensure triangulation boundary is convex (GH-953, Martin Davis)
  - Improve scale handling for PrecisionModel (GH-956, Martin Davis)
  - Fix PreparedLineStringDistance for lines within envelope and polygons (GH-959, Martin Davis)
  - Fix error in CoordinateSequence::add when disallowing repeated points (GH-963, Dan Baston)
  - Fix InteriorPointPoint to handle empty elements (GH-977, Martin Davis)
  - Skip over testing empty distances for mixed collections. (GH-979, Paul Ramsey)
  - Fix TopologyPreservingSimplifier endpoint handling to avoid self-intersections (GH-986, Martin Davis)
  - Build warnings with Visual Studio (GH-929, Jeff Mckenna, Paul Ramsey)
  - Fix CMake on Windows with Visual Studio (GH-945, Aaron Barany)

