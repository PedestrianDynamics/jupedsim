# libfloorfield: design notes

> Status: <!-- draft / reviewed --> · Last updated: <!-- YYYY-MM-DD --> · Authors: <!-- names -->

Static and dynamic floor fields for JuPedSim routing. The library is a Rust crate (`floorfield`),
built as a static library and called from C++ through a `cxx` bridge (`floorfield_cxx`).
In the simulator, `FloorfieldRoutingEngine` (`libsimulator/src/floorfield_routing_engine.*`)
is its only consumer.

---

## 1. Scope

**In scope**

- static floorfields that encode wall avoidance by decresing the speed field for cells within a certain radius to a wall segment
- ingesting the region grap from JuPedSim, a grpah that contains the regions with their polygons as well as their relation to another
- Upon contruction of the MultiFloorfield object the library tests that all walkable grid cells in the graph are reachable. If not an exception is issued.
- Destinatios can be registered as point destinations or polygon destinations, also a set of polygons per region can be registered.
- Travel-time fields to registered destinations (points or areas) over a graph of walkable regions are computed lazily.
- Per-agent queries: walking direction, travel time, whether a point can be routed.
- Field queries: travel time.

**Out of scope (for now)**

- Dynamic fields driven by the simulation itself. Agent density does not feed into the speed fields; only the user changes them (see 7.3).
- Other solvers than FSM are currently not implemented. When reevaluating dnymic fields and hence parallel copmutation other eikonal solver implementations should be considered
- Higher resolution update kernels, currently a 4 neighbor update stencil is used. This inherently leads to in accuracies for a wave that expands diagonally. 
- Waves that can only propogate via diagonal neighbor, for example due to the geometry can not be reached by the current setup, leading to an error. Another stencila can partially help, moving to meshed might be a better solution, for this issue.

---

## 2. Architecture at a glance

| Module | Responsibility |
|---|---|
| `lib.rs` | `cxx` bridge: plain input structs shared with C++, and the exposed API |
| `region_graph.rs` | Checks the unchecked C++ input once; reads seam segments and portals off the rings |
| `geometry.rs` | Grid placement on a shared lattice, speed-field rasterisation, destination cells, Sobel gradient |
| `fsm.rs` | Fast Sweeping Method eikonal solver; can be seeded and can start warm |
| `multi_region.rs` | One grid per region, propagation across seams, halo, destinations, queries |

Data flow:

```
Geometry (C++ RegionGraph2D)
   └─► to_floorfield()  ── plain structs ──►  validate_graph()
                                                   │
                                   per-region grid on shared lattice
                                   + speed field (walls slow down, portals don't)
                                                   │
        add_*_destination()  ──►  lazy solve on first query (FSM per region,
                                  re-seeded across seams until stable, then halo)
                                                   │
        direction() / travel_time() / travel_times()
```

---

## 3. Design decisions

Each decision uses the same layout: **Decision**, **Why**, **Trade-off**, and an optional **Alternatives considered**.

### 3.1 Rust with a `cxx` bridge
- **Decision:** Write the library in Rust and expose it through `cxx`, built with Corrosion in CMake.
- **Why:** 
- easy access to projects that offer functionality that would blow up the repo and package.
- Potential move towards parallel execution or GPU execution, easily fesible by importing the necessary creates: `rayon` for CPU parallel, `wgpu` for GPU parallel.
- The intent is to make it a default routing library reused in other simulators as well. Other simulators are written in Java for example. Rust offers good FFI connectivity through specific crates.
- Potential new contributors might be more enthousiastic towards Rust than C++.
- Well established toolchain, including testing CI and packaging.
- **Trade-off:** 
- Contributors need a Rust toolchain (`rust-version = 1.88`). Errors have to cross the FFI as `Result` and come out as `rust::Error`, which C++ rewraps as `SimulationError`.
- Another language adds complexity to the build tests and CI.
- **Alternatives considered:** 
- we considered C++, favored Rust for this new functionality.

### 3.2 Input is plain structs, checked once
- **Decision:** 
- `RegionGraph`, `Region`, `Ring`, `Seam` and `AreaPiece` are plain shared structs that mirror `RegionGraph2D`. C++ only copies the data into them. `region_graph::validate_graph` checks everything before anything is allocated.
- **Why:** 
- There is a single place for validation, and a `MultiRegionFloorfield` can only exist for a graph that passed. The field keeps no reference to the graph, so one graph can back many fields.
- **Trade-off:** 
- The geometry is copied once at construction.

### 3.3 A single polygon is just a one-region graph
- **Decision:** There is no separate single-region type. One region with no seams is the special case.
- **Why:** There is one code path to test and maintain.

### 3.4 One grid per region on a shared lattice
- **Decision:** Every region gets its own rectangular grid, and all grids sit on one `GridSpec`. This is a single lattice with one origin (the lower-left corner of all outer rings) and one `cell_size`.
- **How a grid is placed:** `GridSpec::cover` takes the region's bounding box, padded by `HALO_PAD = 2` cells on every side, and snaps it outward to lattice lines. Each grid stores its `offset`, the lattice cell of its own cell (0, 0). Converting between local and lattice indices is integer arithmetic:

  ```
  lattice = local + offset_R          local in region R = lattice - offset_R
  ```

  The same world point therefore lands in the same lattice cell in every grid. A grid's cells are walkable only where their centre lies inside that region's polygon. Everything else, including the part of the grid that overhangs into a neighbouring region, is an obstacle (speed 0).
- **What happens at a seam** (figure 1, one row of cells, `cell_size = 1`):

  ```
  Region A: x in [0, 4]        Region B: x in [4, 7]        seam: edge x = 4
  Lattice origin: x = 0        HALO_PAD = 2

                                                  seam (portal)
                                                       ┃
  lattice column          -2   -1    0    1    2    3  ┃  4    5    6    7    8    9
                                                       ┃
  grid A (offset -2)       ▒    ▒    ·    ·    ·    a  ┃  h    ▒    ▒
  local column in A        0    1    2    3    4    5  ┃  6    7    8
                                                       ┃
  grid B (offset  2)                           ▒    h  ┃  b    ·    ·    ▒    ▒    ▒
  local column in B                            0    1  ┃  2    3    4    5    6    7

  ·  walkable cell of the region          ▒  obstacle: padding, or outside the polygon
  a  b  a facing pair from seam_pairs     h  halo cell: an obstacle in its own grid that holds
                                             the neighbour's travel time once solved
  ```
_Figure 1: two regions joined at x = 4. Each grid is padded two cells past its bounding box. Facing cells `a` and `b` exchange seeds during propagation, and after solving each side writes its value into the other grid's halo cell `h`._
  - **Pairing:** `seam_pairs` samples the seam at most half a cell apart. At each sample it takes the nearest walkable cell strictly on the `from` side (`a`) and the nearest strictly on the `to` side (`b`), searching within two cells. Each pair stores a crossing cost: the distance between the two cell centres divided by their mean speed. Every seam is given in both directions, so A→B and B→A each get their own pairs. -> see details below.
  - **Propagation:** once A is solved, `T_A(a) + cost` becomes a seed for `b` in B (see 3.5). Both are whole cells on the same lattice, so the value is copied cell to cell without interpolation.
  - **Halo:** after solving, A's travel time at `a` is written into B's grid at the same lattice cell, the `h` left of the seam, and vice versa. That cell exists in B's grid only because of the padding. This lets B's Sobel gradient look across the seam (see 3.7).
  - **Seams off the lattice lines:** these work the same way. Sampling and the side-of-line test do not assume the seam lines up with the grid. See 3.4.1 for a worked example.
- **Why:**
  - Cells of neighbouring regions coincide exactly, so seeds and halo values move between regions without resampling. The multi-region result equals what a single grid over the whole surface would give.
  - Regions whose bounding boxes overlap (an L next to its neighbour) or that share the same x/y (stacked floors joined by stairs) still get separate fields. Each grid only knows its own polygon, so one region's walkable cells never leak into another's.
  - Each region is solved independently by the plain single-grid FSM (`fsm.rs`). Seams only show up as seeds.
  - A regular grid allows a simple direction function, a Sobel kernel over the 8 neighbors gives a smooth trajectory.
  - With a regualr grid, no graph is needed in the solver to determine the neighbors, the solver can directly traverse the array, as its neighbors are encoded by the index.
- **Trade-off:**
  - *Memory:* one `f32` array per region per solved destination, over the whole padded bounding box (see 7.5). Cells outside an L-shaped or slanted polygon are still stored, and where grids overlap, the same lattice cell is stored once per grid. A grid is capped at `MAX_CELLS_PER_REGION = 1e8` cells, which is about 400 MB per region per destination.
  - *One resolution everywhere:* the lattice forces one `cell_size` on every region, so one region cannot be refined on its own. This matters for passages narrower than a cell (see 6.6).
  - *Seams narrower than a cell* may produce no pairs (`seam_pair_count == 0`). The regions are then disconnected, and construction fails the reachability check.
- **Alternatives considered:**
  - *One global grid:* this cannot represent overlapping or stacked regions, because two regions would claim the same cell.
  - *Independent grids, each with its own origin or cell size:* seeds and halo values would have to be interpolated across every seam, so the result would no longer match the single-grid one.
  - mesh-based discretization: here the quality of the mesh and avoiding obstuse triangles is very important. The geo create in Rust offers a rmeshing function, however it only garanties a min angle of 30 °. However, 


#### 3.4.1 Seam pairing in detail: a seam that cuts through cells

Figure 1 is the easy case, where the seam lies on a lattice line and every cell is clearly on one side. In general the seam runs at an angle and cuts through cells. Then three questions decide what happens: which region owns each cut cell, which cells face each other, and what it costs to cross.

**Example.** `cell_size = 1`, lattice origin (0, 0). Region A is `(0,0) (4.3,0) (5.3,3) (0,3)`, region B is `(4.3,0) (8,0) (8,3) (5.3,3)`. They are joined along the slanted edge from `(4.3, 0)` to `(5.3, 3)`. Seen from A, the seam runs from `a = (4.3, 0)` to `b = (5.3, 3)`, with A on its left.

```
        y=3  +-------+-------+-------+-/-----+-------+
             |       |       |       | /     |       |
       row 2 |   A   |   A   |   A   |/  B   |   B   |
             |       |       |       /       |       |
        y=2  +-------+-------+-------+-------+-------+
             |       |       |      /|       |       |
       row 1 |   A   |   A   |   A / |   B   |   B   |
             |       |       |     / |       |       |
        y=1  +-------+-------+----/--+-------+-------+
             |       |       |   /   |       |       |
       row 0 |   A   |   A   |   / B |   B   |   B   |
             |       |       |  /    |       |       |
        y=0  +-------+-------+-/-----+-------+-------+
            x=2      3       4       5       6       7
                col 2   col 3   col 4   col 5   col 6

  A / B  the region whose polygon holds the cell centre: walkable in that region's grid,
         an obstacle in the other
  /      the seam
```

_Figure 2: a slanted seam cuts through column 4. Each cut cell belongs to the region that holds its centre, so cell (4, 0) is in B but cells (4, 1) and (4, 2) are in A._

**Step 1: ownership is decided by cell centres, not by the seam.** A cell is walkable in a region only if its centre lies strictly inside that region's polygon (`speed_field_on_grid`). Cell (4, 0) has its centre at x = 4.5, and the seam crosses that row's centre height at x ≈ 4.47, so the cell belongs to B. In row 1 the seam is at x = 4.8, so cell (4, 1), centre at 4.5, belongs to A. The walkable cells on the two sides therefore meet along a staircase that follows the seam to within half a cell. No cell is walkable in both grids.

**Step 2: sample the seam.** The seam has length √10 ≈ 3.16, so `samples = ceil(len / (cell_size / 2)) = 7`. The samples sit at the midpoints of 7 equal steps, `t = (k + 0.5) / 7`. For each sample `p`, `nearest_walkable` searches the 5 × 5 block of cells around `p`'s cell. On each side it picks the walkable cell nearest to `p` whose centre lies strictly on that side of the infinite line through the seam: the left side in A, the right side in B.

| k | sample `p` | cell holding `p` | nearest A cell (left) | nearest B cell (right) | pair |
|---|---|---|---|---|---|
| 0 | (4.37, 0.21) | (4, 0) | (3, 0) | (4, 0) | new |
| 1 | (4.51, 0.64) | (4, 0) | (4, 1) | (4, 0) | new |
| 2 | (4.66, 1.07) | (4, 1) | (4, 1) | (4, 0) | duplicate, skipped |
| 3 | (4.80, 1.50) | (4, 1) | (4, 1) | (5, 1) | new |
| 4 | (4.94, 1.93) | (4, 1) | (4, 1) | (5, 1) | duplicate, skipped |
| 5 | (5.09, 2.36) | (5, 2) | (4, 2) | (5, 2) | new |
| 6 | (5.23, 2.79) | (5, 2) | (4, 2) | (5, 2) | duplicate, skipped |

These are the pairs `seam_pairs` actually returns for this geometry, listed as (column, row) lattice cells. The 7 samples give **4 pairs** from A to B:

```
(3,0) → (4,0)    (4,1) → (4,0)    (4,1) → (5,1)    (4,2) → (5,2)
```

The B→A direction is the same set mirrored, because it is sampled from B's own edge.

**Step 3: the crossing cost.** Each pair stores `|centre_a − centre_b| / mean(speed_a, speed_b)`. Here all four pairs are unit neighbours and every cell has speed 1, so each costs 1.0. In general a pair can be diagonal (√2 · `cell_size`) or one or two cells apart, and the cost is the actual distance between the two centres. The seam point itself does not enter the cost.

**What the example shows:**
- **Pairs are not only horizontal.** (4, 1) → (4, 0) crosses the seam vertically, because the staircase puts A's cell directly above B's. Pairs follow whichever cells actually face each other across the staircase.
- **A cell can appear in several pairs.** B's (4, 0) is seeded from both (3, 0) and (4, 1), and it keeps the lower arrival time. A's (4, 1) feeds both (4, 0) and (5, 1). This is how a seam that is steep relative to the grid still carries the wave in every direction it can cross.
- **Sampling every half cell means no cell along the seam is missed.** Consecutive samples often find the same pair, and the `seen` set drops the duplicates.
- **The side test keeps the two sides apart.** In a non-convex region, a walkable cell of A could lie close to the sample but beyond the seam line, behind a corner for example. The strict test against the seam line rejects it, so a pair always crosses the seam.

**Edge case: a seam through cell centres.** If the seam is the vertical line x = 4.5, the centres of column 4 lie on the boundary of both polygons, strictly inside neither. Originally column 4 was then an obstacle in both grids: pairing skipped over it (`(3, r) → (5, r)`, cost 2.0), and the halo was written at (5, r), so column 4 stayed infinite on both sides and the gradient flattened against the seam. Since `49f41a20a` the region with the lower id claims cell centres lying on a seam (`speed_field_on_grid`, `claimed`), so column 4 is walkable in exactly one region and the seam behaves like any other (see 7.4).

**When pairing yields nothing.** If, within two cells of a sample, there is no walkable cell on one side, that sample is skipped. A seam that is short compared with `cell_size`, or that sits in a gap narrower than a cell, can end up with `seam_pair_count == 0`. The regions are then disconnected, and `new` fails the reachability check (see 3.11 and 6.6).

### 3.5 Seams as portals, propagation by re-seeding
- **Decision:** Seam edges are portals and every other edge is a wall. Travel times cross a seam by seeding the facing cells with the real arrival time. Regions are re-solved from a min-heap (smallest seed first) until no seed improves by more than `EPS` (1 ms since the switch to `f32`, see 7.5). A budget turns endless back-and-forth into `NotConverged`.
- **Why:** The result matches a single grid over the whole surface, and it settles on cyclic region graphs too.
- **Trade-off:** 
- In the worst case a region is solved several times per destination. 
- We model an all knowing agent, that knows about all regions it traverses to its destination and can find the globally shortest path.

### 3.6 Fast Sweeping Method with warm starts
- **Decision:** Use FSM (Zhao 2005) with a first-order 4-neighbour Godunov stencil. Values only ever decrease, so a previous solution is a valid upper bound for a warm start. 

- **Why:** The method is simple and has no priority queue. It costs O(k·N), and a re-solve after a seed improves only relaxes the cells that change. The warm result is equal to a cold one, which a test checks.
- **Trade-off:** It is first order. Diagonal distances are overestimated by about 7%. Around a point source the gradient's direction is off by at most about 11° and about 2.6° on average (tested bounds). k grows with how often the shortest paths turn, so a maze takes many rounds.
- **Alternatives considered:** 
- none at this point.
- viable alterntives are FMM, FIM (especially for GPU)
- 8 neighbor stencils could help as well but come with performance downgrade

### 3.7 Halo cells past each portal
- **Decision:** After solving, the neighbour's travel times are written into a one-cell halo just past each portal. These cells are obstacles in the region's own grid, padded by `HALO_PAD = 2`.
- **Why:** Without the halo, the gradient flattens out against a seam instead of pointing across it.
- **Trade-off:** None in memory: the plain field is the same array with the halo cells set to infinity, so no second copy is kept.

### 3.8 Speed field: walls slow agents down, portals don't
- **Decision:** Speed is `clamp(dist_to_nearest_wall / wall_influence_radius, MIN_SPEED, 1)` inside the polygon and 0 outside, with `MIN_SPEED = 0.01 m/s`. A cell that a wall runs through is not walkable (see 7.4). The nearest wall is found with an R-tree (`rstar`). This is the region's *default* speed field; users can replace it (see 7.3).
- **Why:** It keeps routes off walls without blocking narrow doors, and a door is not treated as a wall.
- **Parameters:** `cell_size` (default 0.2 m) and `wall_influence_radius` (default 0.5 m), set per simulation through `FloorfieldOptions`.

### 3.9 Destinations are registered up front and solved lazily
- **Decision:** `add_point_destination` snaps to the nearest walkable cell within two cells. `add_area_destination` takes one or more pieces, each inside one region, and agents head for the nearest piece. A destination is solved on first query and then kept.
- **Why:** Nothing is paid for destinations nobody walks to. A solved field is kept until a speed field it uses changes; then it is dropped and solved again on its next query (see 7.3).
- **Trade-off:** The first query for a destination pays the full solve inside a simulation step.

### 3.10 Query semantics of `direction()`
- Returns a unit vector downhill along the Sobel gradient. If the point's cell is an obstacle, which happens next to a slanted wall, the nearest walkable cell within two cells is used instead.
- Inside an area destination it returns zero. Inside a point destination's cell it points towards the point, and returns zero exactly on it.
- On a ridge, where the gradient vanishes between two equally near exits, the steepest-descent neighbour decides.
- `is_routable` tests the exact polygon, not the grid, so points in holes or walls are rejected.

### 3.11 Fail loudly at construction, never panic across the FFI
- **Decision:** `new` floods the field and fails if any walkable cell is unreachable. This catches disconnected geometry and passages narrower than a cell. This test also catches the constraints of the implementation itself directly at construction.
Settings, grid sizes and query coordinates (no NaN or infinity) are all checked. 
Broken internal invariants come back as an `Internal` error instead of a panic, because a panic would abort the C++ caller.
- **Why:** A geometry error surfaces once, at setup, instead of as an agent stuck mid-simulation.


---

## 4. What works well

- **Clear module boundaries.** Each module above does one job, and the solver knows nothing about regions.
- **Exactness where it matters.** The multi-region result equals the single-grid result, a warm start equals a cold start, and the point-in-region test is exact.
- **Validation at the edges.** Bad input is rejected before allocation, and nothing panics across the FFI.
- **Tests encode the numerics.** 69 Rust tests cover the expected first-order error (distance and gradient angle), seeding, warm starts, walls with gaps and seams with holes, speed fields and their invalidation, seam-cell ownership and thin walls. C++ coverage is in `libsimulator/test/test_floorfield.cpp`.
- **Doc comments state contracts, not mechanics.** Preconditions, error cases and conventions (row-major, row 0 at the lowest y, a cell belongs to a polygon if its centre is strictly inside) are written down.

<!-- Add or adjust from team experience. -->

---

## 5. Known limitations

| Limitation | Effect | Where |
|---|---|---|
| First-order stencil | ~7% diagonal overestimate, up to ~11° direction error near sources | `fsm.rs` |
| Passages narrower than two cells | May close: cells a wall runs through are not walkable, so only a passage at least two cells wide is sure to stay open. A closed one fails construction as unreachable | `speed_field_on_grid`, `check_reachability` |
| Lazy solve on first query | A latency spike in the first step that uses a destination, and again after each speed field change | `MultiRegionFloorfield::solution` |
| One `f32` array per region per destination | Memory grows with destinations × area | `multi_region.rs` |
| Direction taken per cell | Directions change in steps at cell borders | `direction()` |

---

## 6. Next areas to extend

### 6.1 Better accuracy
Options include a second-order FSM, an 8-neighbour stencil, or bilinear interpolation of the gradient between cells for smoother headings.
- **How to measure:** Tighten the bounds in `gradient_direction_open_space` and `gradient_direction_behind_wall_corner`, and compare trajectories with `run_scenarios.py`.

### 6.2 Eager and/or parallel solving
Optionally solve all destinations at setup, in parallel across destinations (they are independent) and possibly across regions in the propagation queue.
- **Open questions:** Should solving be eager by default? Which threading model fits the C++ side?

### 6.3 Memory footprint
Travel times and speeds are `f32` now (see 7.5), and regions a destination never reaches keep no field. What is left: not storing obstacle cells alongside walkable ones in a regular array. CSR or similar formats could be considered.

### 6.4 Richer speed fields
Per-region and per-stage speed fields set by the user are in place (see 7.3), so slow zones, smoke or stairs can be modelled by changing cell values. What is left: anisotropic or direction-dependent costs, and convenience helpers (for example setting a speed for a polygon instead of a raster).

### 6.5 Dynamic floor fields
Invalidation is in place: changing a speed field drops the solutions that use it (see 7.3). What is left: a density-dependent component computed by the simulation itself, and warm-starting a re-solve from the previous field instead of solving from scratch after every change.

### 6.6 Narrow passages
Allow the use of a finer grid in places, where it is needed potentially using a quad-tree based method.

### 6.7 Tooling and visibility
- Add benchmarks (solve time against area, number of regions and maze depth) under `performancetest/`.

<!-- Add items as they come up. -->

---

## 7. Implemented

Items that were on the list above and are now done, with how they were done.

### 7.1 Field parameters exposed (#1702, `a41e3c374`)
`cell_size` and `wall_influence_radius` were constants in `floorfield_routing_engine.cpp`. They are now a `FloorfieldOptions` struct (defaults 0.2 m and 0.5 m) passed to `Simulation`, and in Python as `jps.Simulation(..., floorfield_options=jps.FloorfieldOptions(cell_size=..., wall_influence_radius=...))`. The options apply to the whole simulation, not per region. The only check is the existing one in `validate_settings`: both must be positive and finite.

### 7.2 Travel times and directions exposed to Python (`428715586`)
The grids go through C++, not straight from Rust to Python. The bridge got `region_directions` next to `region_travel_times` and `region_grid`; `FloorfieldRoutingEngine::travel_times` builds a `TravelTimeGrid` from them, and `Simulation.travel_times(stage_id)` returns one per region (`origin`, `width`, `height`, `cell_size`, `extent`, `values`, `directions`). `directions()` works on cells directly instead of calling the point-based `direction()` once per cell: the solution and the destination's cells are looked up once, and both share `direction_at_cell`, so the results are identical. `Hdf5TrajectoryWriter(write_travel_times=True)` stores all grids in `/travel_times` on close, `Hdf5TrajectoryLoader.load_travel_times()` reads them back in the same form, and the trame viewer draws them (`d44e903a0`, `babca8b5f`).

### 7.3 User-defined speed fields (`0d9323ab7`)
Every region has a *default* speed field (the wall-distance rule of 3.8), and every destination can *override* it per region. In Python:
- `default_speed_fields()` / `set_default_speed_field(region_id, values)`: affects every stage without an override in that region.
- `custom_speed_fields(stage_id)` / `set_custom_speed_field(stage_id, region_id, values)` / `clear_custom_speed_field(stage_id, region_id)`: one stage only.

Values are a `(height, width)` raster on the region's grid. `checked_speed` requires one value per cell, and on walkable cells a finite speed of at least `MIN_SPEED = 0.01 m/s`: lower speeds would make travel times overflow, and a zero would change which cells are walkable, which is the geometry's to decide. Values on non-walkable cells are ignored and come out as 0. Setting or clearing a field invalidates exactly the solutions that use it (`d.solution = None`), and they are solved again from scratch on their next query. The speeds only change routes, not how fast agents walk.

### 7.4 Seam cells and thin walls (`49f41a20a`)
- *Cell centres on a seam* used to be walkable in neither region, which flattened the gradient there (3.4.1). Now the region with the lower id claims them: `speed_field_on_grid` gets the seam segments the region owns (`claimed`) and treats a centre on one of them as inside.
- *Walls thinner than a cell* could hold no cell centre, so the cells on either side became neighbours and the field leaked through the wall. Now a cell that a wall segment runs through is not walkable (`cut_by_wall`, a Liang-Barsky clip against the cell shrunk by a hair, so walls along a cell edge and portals do not count). As a consequence, only a passage at least two cells wide is sure to stay open (see 5 and 6.6).

### 7.5 `f32` storage (`75fdf5f43`)
The fast sweeping solver, speed fields, seam seeds and solved travel times are `f32`, halving the memory per destination and region. Geometry stays `f64` (coordinates, cell size, seam distances, wall tolerances), and gradients are taken in `f64` from the `f32` values. The cxx bridge keeps its `f64` signatures and converts at the boundary, so C++ and Python are unchanged. The seam propagation tolerance `EPS` went from `1e-9` to 1 ms, since an `f32` step at realistic travel times is far larger than `1e-9`. On the 5-floor office example, travel times differ from `f64` by at most 0.26 ms (relative 4e-6).

---

## 8. Open questions

- Should we remove point destinations already at this stage? currnetly no outside consumer for it.


## 9. References

- H. Zhao, *A fast sweeping method for Eikonal equations*, Math. Comp. 74 (2005).
- Crate sources: `libfloorfield/src/*.rs`. Bridge consumer: `libsimulator/src/floorfield_routing_engine.cpp`.
- Related PRs and commits: #1702 (`a41e3c374`), `428715586`, `0d9323ab7`, `49f41a20a`, `75fdf5f43`.
