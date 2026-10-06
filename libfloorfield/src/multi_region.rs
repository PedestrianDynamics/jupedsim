// SPDX-License-Identifier: LGPL-3.0-or-later
//! Floor fields over several regions joined by seams.
//!
//! Every region gets its own grid, all on one shared lattice (`GridSpec`), so
//! a cell on one side of a seam corresponds to a cell on the other side by an
//! integer offset. A destination has source cells in one or more regions; its
//! travel times are carried into the other regions across the seams: every
//! seam cell is seeded with the real arrival time from the neighbour, and
//! regions are re-solved until no seed improves. The result matches a single
//! grid over the whole surface.
//!
//! After solving, the neighbour's travel times are written into a one-cell
//! halo just past each portal (cells that are obstacles in the region itself).
//! That lets the gradient point across a seam instead of flattening out
//! against it.
//!
//! Queries are local: `direction` says which way to walk from a point in a
//! given region. Which region a moving agent is in, also after crossing a
//! seam, is the caller's to track - the geometry does that by walking its
//! surface mesh, which also keeps stacked floors apart.

use crate::fsm;
use crate::geometry::{self, bounding_box, GridParams, GridSpec};
use crate::region_graph::{self, DirectedSeam};
use crate::{AreaPiece, RegionGraph, Ring};
use geo::{Contains, Point, Polygon};
use std::cmp::Ordering;
use std::collections::{BinaryHeap, HashMap, HashSet};
use std::fmt;

/// How a field is built from a `RegionGraph`.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct FieldSettings {
    /// Grid resolution in metres, shared by every region.
    pub cell_size: f64,
    /// Distance within which walls slow agents down (portals do not).
    pub wall_influence_radius: f64,
}

#[derive(Clone, Debug, PartialEq)]
pub enum MultiRegionError {
    InvalidInput(String),
    UnknownRegion(usize),
    UnknownDestination(usize),
    NotRoutable {
        region: usize,
        x: f64,
        y: f64,
    },
    /// The destination cannot be reached from the queried point: its region,
    /// or the part of it the point lies in, has no way there.
    Unreachable {
        region: usize,
        dest: usize,
    },
    /// Propagation across seams kept improving past its iteration budget.
    NotConverged,
}

impl fmt::Display for MultiRegionError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::InvalidInput(msg) => write!(f, "invalid input: {msg}"),
            Self::UnknownRegion(r) => write!(f, "unknown region {r}"),
            Self::UnknownDestination(d) => write!(f, "unknown destination {d}"),
            Self::NotRoutable { region, x, y } => {
                write!(f, "({x}, {y}) is not walkable in region {region}")
            }
            Self::Unreachable { region, dest } => {
                write!(
                    f,
                    "destination {dest} cannot be reached from there in region {region}"
                )
            }
            Self::NotConverged => write!(f, "propagation across seams did not converge"),
        }
    }
}

impl std::error::Error for MultiRegionError {}

type Result<T> = std::result::Result<T, MultiRegionError>;

/// One region's grid on the shared lattice.
struct RegionGrid {
    grid: GridParams,
    /// Lattice cell of this grid's cell (0, 0).
    offset: [i64; 2],
    speed: Vec<f64>,
    /// The region's walkable polygon, for exact point-in-region tests: the
    /// grid only resolves it to whole cells.
    shape: Polygon<f64>,
}

impl RegionGrid {
    /// Local index of a lattice cell, if the grid covers it.
    fn local(&self, lattice: [i64; 2]) -> Option<u32> {
        // Checked: the lattice cell of a point far outside saturates near
        // i64::MAX / MIN, and such a point is simply not covered.
        let col = lattice[0].checked_sub(self.offset[0])?;
        let row = lattice[1].checked_sub(self.offset[1])?;
        let inside =
            col >= 0 && row >= 0 && col < self.grid.width as i64 && row < self.grid.height as i64;
        inside.then(|| row as u32 * self.grid.width + col as u32)
    }

    fn lattice(&self, cell: u32) -> [i64; 2] {
        [
            (cell % self.grid.width) as i64 + self.offset[0],
            (cell / self.grid.width) as i64 + self.offset[1],
        ]
    }

    fn walkable(&self, cell: u32) -> bool {
        self.speed[cell as usize] > 0.0
    }

    fn row_col(&self, cell: u32) -> (i32, i32) {
        (
            (cell / self.grid.width) as i32,
            (cell % self.grid.width) as i32,
        )
    }
}

/// A seam rasterised: pairs of cells facing each other across it. The region
/// it leaves from is implied: `out_seams[from]` lists it.
struct SeamCells {
    to: usize,
    /// (cell in `from`, cell in `to`, time to cross from one centre to the other)
    pairs: Vec<(u32, u32, f64)>,
}

/// The part of a destination lying in one region.
struct Source {
    region: usize,
    /// Sorted, walkable cells of `region`.
    cells: Vec<u32>,
    kind: SourceKind,
}

#[derive(Clone, Copy, Debug, PartialEq)]
enum SourceKind {
    /// A point destination, one cell. Inside that cell agents head for
    /// `anchor`: the registered point itself, or the centre of its cell if the
    /// point had to be snapped off an obstacle. Always walkable.
    Point { anchor: (f64, f64) },
    /// A piece of an area destination: inside its cells, agents are there.
    Area,
}

struct Destination {
    sources: Vec<Source>,
    solution: Option<Solution>,
}

impl Destination {
    /// Source cells of every region, merged; empty for regions without one.
    fn cells_per_region(&self, region_count: usize) -> Vec<Vec<u32>> {
        let mut cells = vec![Vec::new(); region_count];
        for s in &self.sources {
            cells[s.region].extend_from_slice(&s.cells);
        }
        cells.into_iter().map(|c| unique(c.into_iter())).collect()
    }
}

struct Solution {
    /// Travel time per region, `None` where the destination is unreachable,
    /// plus the halo: neighbour values written into obstacle cells just past
    /// the seams. What gradient descent reads. Halo cells are obstacles in
    /// their own region, so the plain field is this with them set to infinity
    /// - no second copy is kept.
    query: Vec<Option<Vec<f64>>>,
    /// Halo cells per region: obstacles there, holding neighbour values.
    halo: Vec<HashSet<u32>>,
}

pub struct MultiRegionFloorfield {
    spec: GridSpec,
    regions: Vec<RegionGrid>,
    seams: Vec<SeamCells>,
    out_seams: Vec<Vec<usize>>,
    destinations: Vec<Destination>,
}

const EPS: f64 = 1e-9;

/// Upper bound on cells per region grid. Every destination keeps one f64
/// array per region (`query`), so this caps one destination at
/// about 800 MB per region; it also keeps cell indices well inside `u32`.
const MAX_CELLS_PER_REGION: u64 = 100_000_000;

/// Obstacle cells around every region's grid, holding the halo. Seam cells are
/// paired within two cells of the seam (see `nearest_walkable`), so the
/// neighbour's cell always falls into this margin.
const HALO_PAD: i64 = 2;

impl MultiRegionFloorfield {
    /// Check `graph` and build the grids of all its regions. The field keeps
    /// no reference to the graph: one graph can back any number of fields.
    pub fn new(graph: &RegionGraph, settings: &FieldSettings) -> Result<Self> {
        let FieldSettings {
            cell_size,
            wall_influence_radius,
        } = *settings;
        region_graph::validate_graph(graph)?;
        validate_settings(settings)?;
        // The lattice is anchored at the lower-left corner of everything.
        let mut origin = [f64::INFINITY; 2];
        for p in graph.regions.iter().flat_map(|r| &r.outer.points) {
            origin = [origin[0].min(p.x), origin[1].min(p.y)];
        }
        let spec = GridSpec { origin, cell_size };

        // Pad by HALO_PAD cells on every side: the halo past a seam has to lie
        // inside the grid, and a plain lattice cover leaves no room on the low
        // sides (it starts at the cell holding the bbox minimum).
        let pad = HALO_PAD as f64 * cell_size;
        let boxes: Vec<([f64; 2], [f64; 2])> = graph
            .regions
            .iter()
            .map(|region| {
                let (min, max) = bounding_box(&region.outer);
                ([min[0] - pad, min[1] - pad], [max[0] + pad, max[1] + pad])
            })
            .collect();
        check_cell_counts(&spec, &boxes)?;

        let regions: Vec<RegionGrid> = graph
            .regions
            .iter()
            .zip(&boxes)
            .enumerate()
            .map(|(id, (region, &(min, max)))| {
                let (grid, offset) = spec.cover(min, max);
                let portals = region_graph::portals(graph, id);
                let speed =
                    geometry::speed_field_on_grid(region, &portals, &grid, wall_influence_radius);
                RegionGrid {
                    grid,
                    offset,
                    speed,
                    shape: geometry::polygon(&region.outer, &region.holes),
                }
            })
            .collect();

        let mut out_seams = vec![Vec::new(); regions.len()];
        let seams: Vec<SeamCells> = region_graph::directed_seams(graph)
            .iter()
            .enumerate()
            .map(|(index, seam)| {
                out_seams[seam.from].push(index);
                SeamCells {
                    to: seam.to,
                    pairs: seam_pairs(&spec, &regions[seam.from], &regions[seam.to], seam),
                }
            })
            .collect();

        Ok(Self {
            spec,
            regions,
            seams,
            out_seams,
            destinations: Vec::new(),
        })
    }

    // ── Queries ─────────────────────────────────────────────────────────────

    pub fn region_count(&self) -> usize {
        self.regions.len()
    }

    pub fn grid(&self, region: usize) -> Result<&GridParams> {
        Ok(&self.region(region)?.grid)
    }

    pub fn speed_field(&self, region: usize) -> Result<&[f64]> {
        Ok(&self.region(region)?.speed)
    }

    /// Whether an agent at (x, y) can be routed in `region`: the point lies
    /// inside the region's polygon, and a walkable cell is near enough for
    /// `direction` to take the gradient from it. The polygon test is exact,
    /// so a point inside a hole or a wall is rejected even where the grid's
    /// cells would reach it; a point inside the polygon but in a cell whose
    /// centre is not (next to a slanted wall) is accepted.
    pub fn is_routable(&self, region: usize, x: f64, y: f64) -> bool {
        let Ok(g) = self.region(region) else {
            return false;
        };
        if !(x.is_finite() && y.is_finite()) {
            return false;
        }
        g.shape.contains(&Point::new(x, y))
            && nearest_walkable(&self.spec, g, [x, y], None).is_some()
    }

    /// Number of cell pairs a seam was rasterised into (0: the seam is too
    /// narrow for the grid, or does not touch walkable cells on both sides).
    pub fn seam_pair_count(&self, seam: usize) -> usize {
        self.seams.get(seam).map_or(0, |s| s.pairs.len())
    }

    // ── Destinations ────────────────────────────────────────────────────────

    /// A point destination. Snaps to the nearest walkable cell within two
    /// cells of the point. Non-finite coordinates are invalid input; a point
    /// with no walkable cell that close is not routable.
    pub fn add_point_destination(&mut self, region: usize, x: f64, y: f64) -> Result<usize> {
        check_finite(x, y)?;
        let source = self.point_source(region, x, y)?;
        Ok(self.push_destination(vec![source]))
    }

    /// One destination made of several polygons, each within one region: every
    /// walkable cell whose centre lies inside a piece is a source. Agents head
    /// for whichever part is nearest in travel time.
    pub fn add_area_destination(&mut self, pieces: &[AreaPiece]) -> Result<usize> {
        if pieces.is_empty() {
            return Err(MultiRegionError::InvalidInput(
                "an area destination needs at least one piece".into(),
            ));
        }
        let sources = pieces
            .iter()
            .map(|piece| self.piece_source(piece.region, &piece.outer))
            .collect::<Result<Vec<_>>>()?;
        Ok(self.push_destination(sources))
    }

    /// Number of registered destinations; ids run `0..destination_count()`.
    pub fn destination_count(&self) -> usize {
        self.destinations.len()
    }

    /// Whether `dest` is a registered destination id.
    pub fn has_destination(&self, dest: usize) -> bool {
        dest < self.destinations.len()
    }

    /// Travel time to the destination for every cell of `region`.
    /// Unreachable cells are infinite.
    pub fn travel_times(&mut self, region: usize, dest: usize) -> Result<Vec<f64>> {
        self.region(region)?;
        let sol = self.solution(dest)?;
        let halo = &sol.halo[region];
        Ok(match &sol.query[region] {
            Some(query) => (0u32..)
                .zip(query)
                .map(|(cell, t)| {
                    if halo.contains(&cell) {
                        f64::INFINITY // an obstacle here, see `Solution::query`
                    } else {
                        *t
                    }
                })
                .collect(),
            None => vec![f64::INFINITY; self.regions[region].grid.cell_count()],
        })
    }

    /// Which way to walk from (x, y) in `region` towards `dest`: a unit
    /// vector, or the zero vector once there.
    ///
    /// * Downhill along the travel-time gradient, taken at the point's cell or,
    ///   if that is an obstacle (next to a slanted wall), at the nearest
    ///   walkable cell within two cells.
    /// * Inside an area destination (a cell of one of its pieces): zero.
    /// * Inside the cell of a point destination: straight towards the point,
    ///   zero exactly on it.
    /// * Where the gradient vanishes outside the destination (a ridge between
    ///   two equally near exits): towards the neighbouring cell with the
    ///   steepest descent.
    ///
    /// Errors: non-finite coordinates (`InvalidInput`); unknown region or
    /// destination; a point outside the region's grid or without a walkable
    /// cell near it (`NotRoutable`); a point from which the destination cannot
    /// be reached (`Unreachable`). Solves the destination on first use, which
    /// can fail with `NotConverged`.
    pub fn direction(&mut self, region: usize, x: f64, y: f64, dest: usize) -> Result<(f64, f64)> {
        let g = self.covered(region, x, y)?;
        let cell = nearest_walkable(&self.spec, g, [x, y], None)
            .ok_or(MultiRegionError::NotRoutable { region, x, y })?;
        self.solution(dest)?;
        let d = &self.destinations[dest];
        let g = &self.regions[region];

        let source = d
            .sources
            .iter()
            .find(|s| s.region == region && s.cells.binary_search(&cell).is_ok());
        match source.map(|s| s.kind) {
            Some(SourceKind::Area) => return Ok((0.0, 0.0)),
            Some(SourceKind::Point { anchor }) => return Ok(unit(anchor.0 - x, anchor.1 - y)),
            None => {}
        }

        let unreachable = MultiRegionError::Unreachable { region, dest };
        let sol = d.solution.as_ref().expect("solved above");
        let query = sol.query[region].as_ref().ok_or(unreachable.clone())?;
        if !query[cell as usize].is_finite() {
            return Err(unreachable);
        }
        let (row, col) = g.row_col(cell);
        let (gx, gy) = geometry::sobel_gradient(&g.grid, row, col, query);
        if gx.hypot(gy) > 1e-12 {
            return Ok(unit(-gx, -gy));
        }
        steepest_descent(g, query, cell).ok_or(unreachable)
    }

    // ── Internals ───────────────────────────────────────────────────────────

    fn region(&self, region: usize) -> Result<&RegionGrid> {
        self.regions
            .get(region)
            .ok_or(MultiRegionError::UnknownRegion(region))
    }

    /// Check a query point coming from outside: finite (`InvalidInput`), in a
    /// known region (`UnknownRegion`), and covered by that region's grid
    /// (`NotRoutable`). A point outside the polygon but inside the grid passes:
    /// next to a slanted wall an agent can stand in an obstacle cell and still
    /// get a direction from the nearest walkable one.
    fn covered(&self, region: usize, x: f64, y: f64) -> Result<&RegionGrid> {
        check_finite(x, y)?;
        let g = self.region(region)?;
        g.local(self.spec.lattice_cell(x, y))
            .map(|_| g)
            .ok_or(MultiRegionError::NotRoutable { region, x, y })
    }

    /// The walkable cell for a point: see `add_point_destination`.
    fn point_source(&self, region: usize, x: f64, y: f64) -> Result<Source> {
        let g = self.region(region)?;
        let cell = nearest_walkable(&self.spec, g, [x, y], None)
            .ok_or(MultiRegionError::NotRoutable { region, x, y })?;
        // `nearest_walkable` returns the point's own cell whenever that is
        // walkable. Otherwise the point lies on an obstacle (a wall, a door
        // frame) and heading for it would walk agents into the wall: head for
        // the snapped cell instead.
        let own_cell = g.local(self.spec.lattice_cell(x, y)) == Some(cell);
        let anchor = if own_cell {
            (x, y)
        } else {
            g.grid.cell_center(cell)
        };
        Ok(Source {
            region,
            cells: vec![cell],
            kind: SourceKind::Point { anchor },
        })
    }

    /// The walkable cells inside a piece; one covering no cell centre falls
    /// back to the cell of a point inside the piece (not its bounding box
    /// centre, which can lie outside an L or a ring).
    fn piece_source(&self, region: usize, outer: &Ring) -> Result<Source> {
        region_graph::check_ring(outer).map_err(|msg| {
            MultiRegionError::InvalidInput(format!("area piece in region {region}: ring {msg}"))
        })?;
        let g = self.region(region)?;
        let shape = geometry::polygon(outer, &[]);
        let cells: Vec<u32> = geometry::cells_inside_polygon(&g.grid, &shape)
            .into_iter()
            .filter(|&c| g.walkable(c))
            .collect();
        let cells = if cells.is_empty() {
            let [x, y] = geometry::interior_point(&shape);
            self.point_source(region, x, y)?.cells
        } else {
            cells
        };
        Ok(Source {
            region,
            cells,
            kind: SourceKind::Area,
        })
    }

    fn push_destination(&mut self, sources: Vec<Source>) -> usize {
        self.destinations.push(Destination {
            sources,
            solution: None,
        });
        self.destinations.len() - 1
    }

    fn solution(&mut self, dest: usize) -> Result<&Solution> {
        let d = self
            .destinations
            .get(dest)
            .ok_or(MultiRegionError::UnknownDestination(dest))?;
        if d.solution.is_none() {
            let cells = d.cells_per_region(self.regions.len());
            let sol = self.solve(&cells)?;
            self.destinations[dest].solution = Some(sol);
        }
        Ok(self.destinations[dest].solution.as_ref().expect("just set"))
    }

    /// Solve `region` from its sources and seeds. The first solve starts
    /// cold, from infinity. A region solved again starts warm, from its
    /// `previous` field: seeds are only ever added or lowered, never removed or
    /// raised, so that field is an upper bound on the new one - what
    /// `fsm::solve_into_seeded` needs - and the sweeps only relax what the
    /// improved seeds reach. The result is the same as from a cold start.
    fn solve_region(
        &self,
        region: usize,
        previous: Option<Vec<f64>>,
        sources: &[u32],
        seeds: &[f64],
    ) -> Vec<f64> {
        let g = &self.regions[region];
        let mut out = previous.unwrap_or_else(|| vec![f64::INFINITY; g.grid.cell_count()]);
        fsm::solve_into_seeded(
            &mut out,
            &g.speed,
            sources,
            seeds,
            g.grid.width as usize,
            g.grid.height as usize,
            self.spec.cell_size,
        );
        out
    }

    /// Label-correcting propagation. The destination's cells (`dest_cells`,
    /// per region) are seeds with travel time 0, so the regions holding them
    /// are solved first. Every solve carries arrival times across the region's
    /// seams: a seam cell on the other side whose time improves becomes a seed
    /// there, and that region is queued (smallest seed first) to be solved,
    /// or solved again. Regions no seed ever reaches keep no field.
    fn solve(&self, dest_cells: &[Vec<u32>]) -> Result<Solution> {
        let n = self.regions.len();
        let mut fields: Vec<Option<Vec<f64>>> = vec![None; n];
        let mut seeds: Vec<HashMap<u32, f64>> = vec![HashMap::new(); n];
        let mut dirty = vec![false; n];
        let mut heap = BinaryHeap::new();

        for (r, cells) in dest_cells.iter().enumerate() {
            if cells.is_empty() {
                continue;
            }
            for &c in cells {
                seeds[r].insert(c, 0.0);
            }
            dirty[r] = true;
            heap.push(Entry::new(0.0, r));
        }

        // A region is queued again only for a seed that improves by more than
        // EPS, so propagation ends. The budget turns a pathological case -
        // seams trading ever smaller improvements - into `NotConverged`
        // instead of a long loop.
        let budget = 50 * (n + self.seams.len()) + 100;
        let mut pops = 0;
        while let Some(Entry { region: r, .. }) = heap.pop() {
            if !dirty[r] {
                continue;
            }
            dirty[r] = false;
            pops += 1;
            if pops > budget {
                return Err(MultiRegionError::NotConverged);
            }
            let mut sources: Vec<(u32, f64)> = seeds[r].iter().map(|(&c, &t)| (c, t)).collect();
            sources.sort_unstable_by_key(|&(c, _)| c); // deterministic
            let (cells, values): (Vec<u32>, Vec<f64>) = sources.into_iter().unzip();
            let field = self.solve_region(r, fields[r].take(), &cells, &values);

            for &s in &self.out_seams[r] {
                let seam = &self.seams[s];
                let y = seam.to;
                let mut improved = false;
                for &(cr, cy, cross) in &seam.pairs {
                    let t = field[cr as usize] + cross;
                    if !t.is_finite() {
                        continue;
                    }
                    let known = fields[y].as_ref().map_or(f64::INFINITY, |f| f[cy as usize]);
                    let current = seeds[y]
                        .get(&cy)
                        .copied()
                        .unwrap_or(f64::INFINITY)
                        .min(known);
                    if t < current - EPS {
                        seeds[y].insert(cy, t);
                        improved = true;
                    }
                }
                if improved {
                    dirty[y] = true;
                    let key = seeds[y].values().copied().fold(f64::INFINITY, f64::min);
                    heap.push(Entry::new(key, y));
                }
            }
            fields[r] = Some(field);
        }

        // Halo: across every seam, the neighbour's own travel times, written
        // in place. Reading a neighbour that already has its halo is safe: the
        // values come from seam cells, which are walkable, and the halo only
        // ever writes obstacle cells.
        let mut query = fields;
        let mut halo = vec![HashSet::new(); n];
        for r in 0..n {
            if query[r].is_none() {
                continue;
            }
            let mut writes = Vec::new();
            for &s in &self.out_seams[r] {
                let seam = &self.seams[s];
                let Some(other) = &query[seam.to] else {
                    continue;
                };
                for &(_, cy, _) in &seam.pairs {
                    let lattice = self.regions[seam.to].lattice(cy);
                    writes.push((lattice, other[cy as usize]));
                }
            }
            let q = query[r].as_mut().expect("checked above");
            for (lattice, value) in writes {
                write_halo(&self.regions[r], q, &mut halo[r], lattice, value);
            }
        }
        Ok(Solution { query, halo })
    }
}

/// Write a neighbour value into the halo cell at `lattice`, if the region's
/// grid covers it and it is an obstacle there (never overwrite walkable cells).
fn write_halo(
    g: &RegionGrid,
    query: &mut [f64],
    halo: &mut HashSet<u32>,
    lattice: [i64; 2],
    value: f64,
) {
    let Some(cell) = g.local(lattice) else { return };
    if g.walkable(cell) || !value.is_finite() {
        return;
    }
    if value < query[cell as usize] {
        query[cell as usize] = value;
        halo.insert(cell);
    }
}

/// Rasterise a seam into facing cell pairs. Samples run along the seam at most
/// half a cell apart; at each, the nearest walkable cell on the left (in
/// `from`) is paired with the nearest walkable cell on the right (in `to`).
fn seam_pairs(
    spec: &GridSpec,
    from: &RegionGrid,
    to: &RegionGrid,
    seam: &DirectedSeam,
) -> Vec<(u32, u32, f64)> {
    let d = [seam.b[0] - seam.a[0], seam.b[1] - seam.a[1]];
    let len = (d[0] * d[0] + d[1] * d[1]).sqrt();
    if len == 0.0 {
        return Vec::new();
    }
    let left = [-d[1] / len, d[0] / len];
    let right = [-left[0], -left[1]];
    let samples = (len / (spec.cell_size * 0.5)).ceil().max(1.0) as usize;

    let mut seen = HashSet::new();
    let mut pairs = Vec::new();
    for k in 0..samples {
        let t = (k as f64 + 0.5) / samples as f64;
        let p = [seam.a[0] + d[0] * t, seam.a[1] + d[1] * t];
        let side_a = Some((seam.a, left));
        let side_b = Some((seam.a, right));
        let (Some(ca), Some(cb)) = (
            nearest_walkable(spec, from, p, side_a),
            nearest_walkable(spec, to, p, side_b),
        ) else {
            continue;
        };
        if !seen.insert((ca, cb)) {
            continue;
        }
        let pa = from.grid.cell_center(ca);
        let pb = to.grid.cell_center(cb);
        let dist = ((pa.0 - pb.0).powi(2) + (pa.1 - pb.1).powi(2)).sqrt();
        let speed = 0.5 * (from.speed[ca as usize] + to.speed[cb as usize]);
        pairs.push((ca, cb, dist / speed));
    }
    pairs
}

/// Walkable cell for `p`: without `side`, the cell containing it if walkable,
/// else the nearest walkable cell within two cells. With `side = (a, n)`, the
/// nearest walkable cell whose centre lies strictly on the `n` side of the line
/// through `a`.
fn nearest_walkable(
    spec: &GridSpec,
    g: &RegionGrid,
    p: [f64; 2],
    side: Option<([f64; 2], [f64; 2])>,
) -> Option<u32> {
    let centre = spec.lattice_cell(p[0], p[1]);
    if side.is_none() {
        // The cell containing the point wins; distance only breaks the tie
        // when it is not walkable (a point on a cell corner is equally far
        // from four centres).
        if let Some(cell) = g.local(centre).filter(|&c| g.walkable(c)) {
            return Some(cell);
        }
    }
    let mut best: Option<(f64, u32)> = None;
    for dr in -2..=2 {
        for dc in -2..=2 {
            // Saturating: next to a saturated centre the neighbour stays out of
            // every grid.
            let neighbour = [centre[0].saturating_add(dc), centre[1].saturating_add(dr)];
            let Some(cell) = g.local(neighbour) else {
                continue;
            };
            if !g.walkable(cell) {
                continue;
            }
            let (cx, cy) = g.grid.cell_center(cell);
            if let Some((a, n)) = side {
                if (cx - a[0]) * n[0] + (cy - a[1]) * n[1] <= 0.0 {
                    continue;
                }
            }
            let dist = (cx - p[0]).powi(2) + (cy - p[1]).powi(2);
            if best.is_none_or(|(b, _)| dist < b) {
                best = Some((dist, cell));
            }
        }
    }
    best.map(|(_, cell)| cell)
}

/// (dx, dy) scaled to length 1; zero stays zero.
fn unit(dx: f64, dy: f64) -> (f64, f64) {
    let norm = dx.hypot(dy);
    if norm > 0.0 {
        (dx / norm, dy / norm)
    } else {
        (0.0, 0.0)
    }
}

/// Towards the neighbour of `cell` (eight-neighbourhood, halo included) whose
/// travel time drops fastest per distance; `None` if none is lower. A
/// reachable cell outside the destination always has a lower neighbour: its
/// time came from one.
fn steepest_descent(g: &RegionGrid, query: &[f64], cell: u32) -> Option<(f64, f64)> {
    let t = query[cell as usize];
    let (row, col) = g.row_col(cell);
    let mut best: Option<(f64, (f64, f64))> = None;
    for (dr, dc) in [
        (-1, -1),
        (-1, 0),
        (-1, 1),
        (0, -1),
        (0, 1),
        (1, -1),
        (1, 0),
        (1, 1),
    ] {
        let (r, c) = (row + dr, col + dc);
        if r < 0 || c < 0 || r >= g.grid.height as i32 || c >= g.grid.width as i32 {
            continue;
        }
        let drop = t - query[r as usize * g.grid.width as usize + c as usize];
        let rate = drop / f64::from(dr * dr + dc * dc).sqrt();
        if rate > 0.0 && best.is_none_or(|(b, _)| rate > b) {
            best = Some((rate, unit(f64::from(dc), f64::from(dr))));
        }
    }
    best.map(|(_, dir)| dir)
}

/// Coordinates from outside must be finite: NaN would land in lattice cell 0
/// and look like a real place.
fn check_finite(x: f64, y: f64) -> Result<()> {
    if x.is_finite() && y.is_finite() {
        Ok(())
    } else {
        Err(MultiRegionError::InvalidInput(format!(
            "coordinates must be finite, got ({x}, {y})"
        )))
    }
}

fn unique(cells: impl Iterator<Item = u32>) -> Vec<u32> {
    let mut v: Vec<u32> = cells.collect();
    v.sort_unstable();
    v.dedup();
    v
}

/// Both settings must be positive and finite. (The graph is checked by
/// `region_graph::validate_graph`.)
fn validate_settings(settings: &FieldSettings) -> Result<()> {
    let invalid = |msg: &str| Err(MultiRegionError::InvalidInput(msg.to_string()));
    // `!(x > 0.0)` also rejects NaN.
    if !(settings.cell_size > 0.0 && settings.cell_size.is_finite()) {
        return invalid("cell_size must be positive and finite");
    }
    if !(settings.wall_influence_radius > 0.0 && settings.wall_influence_radius.is_finite()) {
        return invalid("wall_influence_radius must be positive and finite");
    }
    Ok(())
}

/// Reject grids too large to allocate before allocating them. `boxes` are the
/// padded bounding boxes of the regions, in region order. Saturating
/// arithmetic: a tiny `cell_size` on a large region must not overflow here.
fn check_cell_counts(spec: &GridSpec, boxes: &[([f64; 2], [f64; 2])]) -> Result<()> {
    for (region, &(min, max)) in boxes.iter().enumerate() {
        let lo = spec.lattice_cell(min[0], min[1]);
        let hi = spec.lattice_cell(max[0], max[1]);
        let extent = |axis: usize| hi[axis].saturating_sub(lo[axis]).saturating_add(1) as u64;
        let cells = extent(0).saturating_mul(extent(1));
        if cells > MAX_CELLS_PER_REGION {
            return Err(MultiRegionError::InvalidInput(format!(
                "region {region} needs {cells} cells at cell_size {}, more than the \
                 limit of {MAX_CELLS_PER_REGION}; use a larger cell_size",
                spec.cell_size
            )));
        }
    }
    Ok(())
}

/// Min-heap entry: smallest key first, ties by region index.
#[derive(PartialEq)]
struct Entry {
    key: f64,
    region: usize,
}

impl Entry {
    fn new(key: f64, region: usize) -> Self {
        Self { key, region }
    }
}

impl Eq for Entry {}

impl Ord for Entry {
    fn cmp(&self, other: &Self) -> Ordering {
        other
            .key
            .total_cmp(&self.key)
            .then_with(|| other.region.cmp(&self.region))
    }
}

impl PartialOrd for Entry {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::test_support::*;
    use crate::{Region, Seam};

    const CS: f64 = 0.25;

    fn field(graph: &RegionGraph, cell_size: f64) -> Result<MultiRegionFloorfield> {
        let settings = FieldSettings {
            cell_size,
            wall_influence_radius: 0.5,
        };
        MultiRegionFloorfield::new(graph, &settings)
    }

    fn build(regions: Vec<Region>, seams: Vec<Seam>) -> Result<MultiRegionFloorfield> {
        field(&RegionGraph { regions, seams }, CS)
    }

    /// 20 x 4 corridor, split at x = 10 into two regions sharing the full edge.
    fn split_corridor() -> MultiRegionFloorfield {
        let regions = vec![
            region(rect(0., 0., 10., 4.)),
            region(rect(10., 0., 20., 4.)),
        ];
        // rect edges: 0 bottom, 1 right, 2 top, 3 left.
        build(regions, join(0, 1, 1, 3).to_vec()).unwrap()
    }

    fn whole_corridor() -> MultiRegionFloorfield {
        build(vec![region(rect(0., 0., 20., 4.))], vec![]).unwrap()
    }

    /// The region seam `s` leaves from.
    fn seam_from(ff: &MultiRegionFloorfield, s: usize) -> usize {
        ff.out_seams
            .iter()
            .position(|out| out.contains(&s))
            .unwrap()
    }

    /// Time at a world point.
    fn time_at(ff: &mut MultiRegionFloorfield, region: usize, x: f64, y: f64, dest: usize) -> f64 {
        let tt = ff.travel_times(region, dest).unwrap();
        let g = &ff.regions[region];
        tt[g.local(ff.spec.lattice_cell(x, y)).unwrap() as usize]
    }

    /// Follow `direction` from (x, y) the way an agent does, in steps of one
    /// cell length, as (region, x, y) points. Stands in for the geometry's
    /// region tracking: a point stays in its region while inside the polygon,
    /// else moves to whichever region's polygon holds it - fine here, these
    /// test layouts have no stacked floors. Stops on arrival (travel time 0,
    /// or a zero direction), off the surface, or after a step budget.
    fn walk(
        ff: &mut MultiRegionFloorfield,
        region: usize,
        x: f64,
        y: f64,
        dest: usize,
    ) -> Vec<(usize, f64, f64)> {
        use geo::Intersects;
        let (mut r, mut px, mut py) = (region, x, y);
        let mut points = vec![(r, px, py)];
        for _ in 0..2000 {
            if time_at(ff, r, px, py, dest) == 0.0 {
                break;
            }
            let Ok((dx, dy)) = ff.direction(r, px, py, dest) else {
                break;
            };
            if (dx, dy) == (0.0, 0.0) {
                break;
            }
            let (nx, ny) = (px + CS * dx, py + CS * dy);
            let p = Point::new(nx, ny);
            let Some(next) = std::iter::once(r)
                .chain(0..ff.regions.len())
                .find(|&c| ff.regions[c].shape.intersects(&p))
            else {
                break;
            };
            (r, px, py) = (next, nx, ny);
            points.push((r, px, py));
        }
        points
    }

    /// An agent inside the polygon but in a cell whose centre lies outside it
    /// (next to a slanted wall) stands on an obstacle cell, which has no
    /// gradient. It gets the direction at the nearest walkable cell, and its
    /// walk goes around the inner corner rather than through the wall.
    #[test]
    fn agent_in_an_obstacle_cell_follows_the_nearest_walkable_cell() {
        // L-shaped room: the destination sits in the upper arm, around the
        // inner corner at (5, 5). The right wall is slanted, (10, 0)-(10.1, 5),
        // so it cuts through cells off the lattice.
        let outer = ring(&[0., 0., 10., 0., 10.1, 5., 5., 5., 5., 10., 0., 10.]);
        let mut ff = build(vec![region(outer)], vec![]).unwrap();
        let dest = ff.add_point_destination(0, 2.5, 9.0).unwrap();

        let (x, y) = (10.02, 2.0);
        let g = &ff.regions[0];
        let cell = g.local(ff.spec.lattice_cell(x, y)).unwrap();
        assert!(
            !g.walkable(cell),
            "the agent must stand in an obstacle cell"
        );

        let (dx, dy) = ff.direction(0, x, y, dest).unwrap();
        assert!(
            (dx.hypot(dy) - 1.0).abs() < 1e-12,
            "expected a unit vector, got ({dx:.2}, {dy:.2})"
        );
        assert!(dx < 0.0, "the direction must lead away from the right wall");

        // The whole path goes around the inner corner, never across the
        // missing quarter x > 5, y > 5.
        let path = walk(&mut ff, 0, x, y, dest);
        for &(_, px, py) in &path {
            assert!(
                !(px > 5.0 + CS && py > 5.0 + CS),
                "path cuts through the wall at ({px:.2}, {py:.2})"
            );
        }
    }

    /// A point destination keeps its own coordinates as the anchor when they
    /// are walkable; one snapped off an obstacle anchors at the snapped cell,
    /// so agents arriving there are not sent into the wall.
    #[test]
    fn point_destination_anchor_is_walkable() {
        let anchor =
            |ff: &MultiRegionFloorfield, dest: usize| match ff.destinations[dest].sources[0].kind {
                SourceKind::Point { anchor } => anchor,
                SourceKind::Area => panic!("a point destination"),
            };
        let mut ff = whole_corridor();
        let inside = ff.add_point_destination(0, 10.05, 2.0).unwrap();
        assert_eq!(anchor(&ff, inside), (10.05, 2.0));

        // Just below the corridor's bottom wall (y = 0): an obstacle cell.
        let on_wall = ff.add_point_destination(0, 10.05, -0.1).unwrap();
        let (ax, ay) = anchor(&ff, on_wall);
        let cell = ff.destinations[on_wall].sources[0].cells[0];
        assert_eq!(
            (ax, ay),
            ff.regions[0].grid.cell_center(cell),
            "anchored at the snapped cell"
        );
        assert!(ay > 0.0 && ff.is_routable(0, ax, ay));
    }

    /// `is_routable` decides by the polygon, not by whole cells: a point inside
    /// the room next to a slanted wall is routable even though its cell centre
    /// lies outside; a point just beyond that wall is not, even though
    /// walkable cells are within reach.
    #[test]
    fn routability_follows_the_polygon_not_the_cells() {
        let outer = ring(&[0., 0., 10., 0., 10.1, 5., 5., 5., 5., 10., 0., 10.]);
        let ff = build(vec![region(outer)], vec![]).unwrap();
        let g = &ff.regions[0];
        let edge_cell = g.local(ff.spec.lattice_cell(10.02, 2.0)).unwrap();
        assert!(!g.walkable(edge_cell), "precondition: an obstacle cell");
        assert!(ff.is_routable(0, 10.02, 2.0), "inside the room");
        assert!(!ff.is_routable(0, 10.2, 2.0), "beyond the wall");
        assert!(
            !ff.is_routable(0, 7.5, 7.5),
            "in the missing quarter of the L"
        );
    }

    /// A piece covering no cell centre falls back to the cell of a point inside
    /// it - the bounding box centre of this thin L, (10.1, 1.1), lies outside
    /// it - and is still an area: zero direction there.
    #[test]
    fn tiny_l_shaped_piece_falls_back_to_a_point_inside_it() {
        let mut ff = whole_corridor();
        let l = [
            10.0, 1.0, 10.2, 1.0, 10.2, 1.05, 10.05, 1.05, 10.05, 1.2, 10.0, 1.2,
        ];
        let dest = ff.add_area_destination(&[piece(0, ring(&l))]).unwrap();
        let source = &ff.destinations[dest].sources[0];
        assert_eq!(source.kind, SourceKind::Area);
        let [x, y] = geometry::interior_point(&geometry::polygon(&ring(&l), &[]));
        let g = &ff.regions[0];
        assert_eq!(
            source.cells,
            vec![g.local(ff.spec.lattice_cell(x, y)).unwrap()]
        );
        assert_eq!(ff.direction(0, x, y, dest).unwrap(), (0.0, 0.0));
    }

    /// Coordinates from outside: non-finite ones are invalid input, finite
    /// ones beyond the region's grid are not routable. Nothing panics.
    #[test]
    fn bad_coordinates_are_reported_not_panicked() {
        let mut ff = whole_corridor();
        let dest = ff.add_point_destination(0, 10.0, 2.0).unwrap();
        fn invalid<T>(r: &Result<T>) -> bool {
            matches!(r, Err(MultiRegionError::InvalidInput(_)))
        }
        fn not_routable<T>(r: &Result<T>) -> bool {
            matches!(r, Err(MultiRegionError::NotRoutable { .. }))
        }

        let non_finite = [
            (f64::NAN, 2.0),
            (2.0, f64::NAN),
            (f64::INFINITY, 2.0),
            (2.0, f64::NEG_INFINITY),
        ];
        for (x, y) in non_finite {
            assert!(invalid(&ff.add_point_destination(0, x, y)), "({x}, {y})");
            assert!(invalid(&ff.direction(0, x, y, dest)), "({x}, {y})");
            assert!(!ff.is_routable(0, x, y), "({x}, {y})");
        }
        // Finite, but so far beyond the grid that the lattice cell saturates
        // at the i64 limits.
        for (x, y) in [(1e300, 2.0), (-1e300, 2.0), (2.0, 1e19), (100.0, 2.0)] {
            assert!(
                not_routable(&ff.add_point_destination(0, x, y)),
                "({x}, {y})"
            );
            assert!(not_routable(&ff.direction(0, x, y, dest)), "({x}, {y})");
            assert!(!ff.is_routable(0, x, y), "({x}, {y})");
        }
        // Just outside the polygon, still inside the grid: a direction back in.
        let (dx, _) = ff.direction(0, 20.1, 2.0, dest).unwrap();
        assert!(dx < 0.0);
    }

    /// A ring of four regions around a 4 x 4 hole in a 10 x 10 square: bottom 0,
    /// right 1, top 2, left 3, each joined to the next - a cycle in the region
    /// graph. Ring edges are split at the seams, so that only the seam part of
    /// an edge is a portal.
    fn ring_graph() -> RegionGraph {
        let bottom = ring(&[0., 0., 10., 0., 10., 3., 7., 3., 3., 3., 0., 3.]);
        let top = ring(&[0., 7., 3., 7., 7., 7., 10., 7., 10., 10., 0., 10.]);
        RegionGraph {
            regions: vec![
                region(bottom),
                region(rect(7., 3., 10., 7.)),
                region(top),
                region(rect(0., 3., 3., 7.)),
            ],
            // bottom edges 2 and 4 face right edge 0 and left edge 0; top
            // edges 2 and 0 face right edge 2 and left edge 2.
            seams: [
                join(0, 2, 1, 0),
                join(1, 2, 2, 2),
                join(3, 2, 2, 0),
                join(0, 4, 3, 0),
            ]
            .concat(),
        }
    }

    /// Propagation around a cycle of regions: it must settle (improving
    /// seeds die out, no NotConverged) and agree with the same shape as one
    /// region. Off-centre, one way around wins; centred, both tie at the top.
    /// This takes 5 and 4 region solves; the result is within 0.5% of the
    /// single grid.
    #[test]
    fn propagation_settles_around_a_cycle_of_regions() {
        let mut square = region(rect(0., 0., 10., 10.));
        square.holes.push(ring(&[3., 3., 3., 7., 7., 7., 7., 3.]));
        let whole = RegionGraph {
            regions: vec![square],
            seams: vec![],
        };
        for target_x in [8.0, 5.0] {
            let mut ring = field(&ring_graph(), CS).unwrap();
            let mut single = field(&whole, CS).unwrap();
            let dr = ring.add_point_destination(0, target_x, 1.5).unwrap();
            let ds = single.add_point_destination(0, target_x, 1.5).unwrap();
            ring.travel_times(0, dr).expect("propagation settles");

            // One point per region, and three along the top, the far side.
            for (r, x, y) in [
                (0, 2.0, 1.5),
                (1, 8.5, 5.0),
                (2, 8.5, 8.5),
                (2, 5.0, 8.5),
                (2, 1.5, 8.5),
                (3, 1.5, 5.0),
            ] {
                let t_ring = time_at(&mut ring, r, x, y, dr);
                let t_single = time_at(&mut single, 0, x, y, ds);
                assert!(
                    (t_ring - t_single).abs() <= 0.01 * t_single + 1e-9,
                    "target x {target_x}, ({x}, {y}): ring {t_ring}, single region {t_single}"
                );
            }
        }
    }

    #[test]
    fn seams_are_rasterised_on_both_sides() {
        let ff = split_corridor();
        for s in 0..2 {
            let seam = &ff.seams[s];
            let from = seam_from(&ff, s);
            assert!(seam.pairs.len() >= 14, "{} pairs", seam.pairs.len());
            for &(ca, cb, cross) in &seam.pairs {
                let (xa, ya) = ff.regions[from].grid.cell_center(ca);
                let (xb, yb) = ff.regions[seam.to].grid.cell_center(cb);
                let (left_x, right_x) = if from == 0 { (xa, xb) } else { (xb, xa) };
                assert!(left_x < 10.0 && right_x > 10.0, "pair straddles the seam");
                // Paired cells face each other: at most a diagonal step apart.
                let dist = ((xa - xb).powi(2) + (ya - yb).powi(2)).sqrt();
                assert!(
                    dist > 0.0 && dist <= 2f64.sqrt() * CS + 1e-12,
                    "dist {dist}"
                );
                // `cross` is a time: at least the distance (speed <= 1), more
                // where the walls slow agents down.
                assert!(cross >= dist - 1e-12, "cross {cross} < dist {dist}");
            }
        }
    }

    #[test]
    fn exact_matches_a_single_grid() {
        let mut split = split_corridor();
        let mut whole = whole_corridor();
        let ds = split.add_point_destination(1, 19.0, 2.0).unwrap();
        let dw = whole.add_point_destination(0, 19.0, 2.0).unwrap();

        for &(x, y) in &[(1.0, 2.0), (5.0, 0.5), (9.5, 3.5), (12.0, 2.0)] {
            let region = if x < 10.0 { 0 } else { 1 };
            let s = time_at(&mut split, region, x, y, ds);
            let w = time_at(&mut whole, 0, x, y, dw);
            // One seam crossing costs at most a fraction of a cell.
            assert!(
                (s - w).abs() < CS,
                "at ({x}, {y}): split {s:.3} vs whole {w:.3}"
            );
        }
    }

    #[test]
    fn walk_crosses_a_wide_doorway_along_the_straight_line() {
        // Room A (0..10 x 0..10) opens into room B (10..20 x 0..10) through a
        // wide doorway x = 10, y in [0, 10]. Exit near the top right of B.
        let regions = vec![
            region(rect(0., 0., 10., 10.)),
            region(rect(10., 0., 20., 10.)),
        ];
        let mut ff = build(regions, join(0, 1, 1, 3).to_vec()).unwrap();
        let dest = ff.add_point_destination(1, 19.0, 9.0).unwrap();
        let path = walk(&mut ff, 0, 5.0, 1.0, dest);

        let end = *path.last().unwrap();
        assert_eq!(end.0, 1);
        assert!(
            (end.1 - 19.0).hypot(end.2 - 9.0) <= 1.5 * CS,
            "ends at {end:?}"
        );
        // The straight line (5,1) -> (19,9) crosses x = 10 at y = 1 + 5 * 8/14
        // = 3.86: the walk crosses the doorway there, not at its nearest
        // point y = 1.
        let crossing = path.iter().find(|p| p.0 == 1).copied().unwrap();
        assert!(crossing.2 > 3.0, "crossed at y = {}", crossing.2);
        let length: f64 = path
            .windows(2)
            .map(|w| (w[1].1 - w[0].1).hypot(w[1].2 - w[0].2))
            .sum();
        let straight = (end.1 - 5.0).hypot(end.2 - 1.0);
        assert!(
            length >= straight - 1e-9,
            "a walk cannot beat the straight line"
        );
        assert!(
            length < straight * 1.05,
            "walk {length:.2} vs straight {straight:.2}"
        );
    }

    #[test]
    fn directions_lead_across_the_seam_instead_of_stalling() {
        let mut ff = split_corridor();
        let dest = ff.add_point_destination(1, 19.0, 2.0).unwrap();
        let path = walk(&mut ff, 0, 1.0, 2.0, dest);
        let &(r, x, y) = path.last().unwrap();
        assert!(
            r == 1 && (x - 19.0).hypot(y - 2.0) <= 1.5 * CS,
            "ends in {r} at ({x}, {y})"
        );
        // Regions only ever advance 0 -> 1, never back.
        assert!(path.windows(2).all(|w| w[0].0 <= w[1].0), "ping-pong");
    }

    #[test]
    fn directions_lead_across_the_seam_against_the_lattice_too() {
        // Destination in the LEFT region: the walk leaves region 1 through the
        // low-x side of its grid.
        let mut ff = split_corridor();
        let dest = ff.add_point_destination(0, 1.0, 2.0).unwrap();
        let path = walk(&mut ff, 1, 19.0, 2.0, dest);
        let &(r, x, y) = path.last().unwrap();
        assert!(
            r == 0 && (x - 1.0).hypot(y - 2.0) <= 1.5 * CS,
            "ends in {r} at ({x}, {y})"
        );
    }

    /// A square split along the slanted seam (10, 0)-(4, 10): region 0 west,
    /// region 1 east. Along a slanted seam the cells paired across it leave
    /// gaps, and region 1 ends in an acute tip at (4, 10). Directions must
    /// still lead across and to the target.
    #[test]
    fn directions_lead_across_a_diagonal_seam() {
        let west = ring(&[0., 0., 10., 0., 4., 10., 0., 10.]);
        let east = ring(&[10., 0., 10., 10., 4., 10.]);
        let regions = vec![region(west), region(east)];
        let mut ff = build(regions, join(0, 1, 1, 2).to_vec()).unwrap();
        let dest = ff.add_point_destination(1, 9.0, 5.0).unwrap();

        // (1, 9) heads for the acute tip, (6.5, 2) starts right at the seam.
        for (x, y) in [(1.0, 9.0), (1.0, 1.0), (3.0, 9.8), (6.5, 2.0)] {
            let path = walk(&mut ff, 0, x, y, dest);
            let &(r, ex, ey) = path.last().unwrap();
            assert_eq!(r, 1, "from ({x}, {y}): stopped at ({ex}, {ey})");
            assert!(
                (ex - 9.0).hypot(ey - 5.0) <= 1.5 * CS,
                "from ({x}, {y}): ends at ({ex}, {ey})"
            );
        }
    }

    #[test]
    fn every_seam_pair_has_a_halo_cell_on_both_sides() {
        let ff = split_corridor();
        for (s, seam) in ff.seams.iter().enumerate() {
            let (from, to) = (&ff.regions[seam_from(&ff, s)], &ff.regions[seam.to]);
            for &(ca, cb, _) in &seam.pairs {
                // The neighbour's cell exists in this region's grid as an obstacle,
                // and vice versa: that is where the halo goes.
                let there = from
                    .local(to.lattice(cb))
                    .expect("halo cell inside the grid");
                let back = to
                    .local(from.lattice(ca))
                    .expect("halo cell inside the grid");
                assert!(!from.walkable(there) && !to.walkable(back));
            }
        }
    }

    #[test]
    fn an_unreachable_region_has_no_field_and_no_direction() {
        let regions = vec![region(rect(0., 0., 5., 5.)), region(rect(10., 0., 15., 5.))];
        let mut ff = build(regions, vec![]).unwrap();
        let dest = ff.add_point_destination(0, 2.0, 2.0).unwrap();
        assert!(ff
            .travel_times(1, dest)
            .unwrap()
            .iter()
            .all(|t| t.is_infinite()));
        assert_eq!(
            ff.direction(1, 12.0, 2.0, dest),
            Err(MultiRegionError::Unreachable { region: 1, dest })
        );
    }

    /// Two rooms in one region, joined by a passage narrower than a cell: no
    /// cell centre lies in it, so the far room is cut off although its region
    /// has a field.
    #[test]
    fn a_pocket_cut_off_within_a_region_has_no_direction() {
        let outer = ring(&[
            0., 0., 5., 0., 5., 1.95, 6., 1.95, 6., 0., 11., 0., 11., 4., 6., 4., 6., 2.05, 5.,
            2.05, 5., 4., 0., 4.,
        ]);
        let mut ff = build(vec![region(outer)], vec![]).unwrap();
        let dest = ff.add_point_destination(0, 1.0, 2.0).unwrap();
        assert!(time_at(&mut ff, 0, 3.0, 2.0, dest).is_finite());
        assert!(ff.direction(0, 3.0, 2.0, dest).is_ok());
        assert_eq!(
            ff.direction(0, 9.0, 2.0, dest),
            Err(MultiRegionError::Unreachable { region: 0, dest })
        );
    }

    #[test]
    fn inside_a_point_destination_cell_head_for_the_point() {
        let mut ff = split_corridor();
        let dest = ff.add_point_destination(1, 19.0, 2.0).unwrap();
        // The point's cell spans [19, 19.25) x [2, 2.25).
        let (dx, dy) = ff.direction(1, 19.1, 2.1, dest).unwrap();
        let expected = unit(-0.1, -0.1);
        assert!((dx - expected.0).abs() < 1e-12 && (dy - expected.1).abs() < 1e-12);
        assert_eq!(ff.direction(1, 19.0, 2.0, dest).unwrap(), (0.0, 0.0));
    }

    /// Where the gradient vanishes outside the destination, the steepest
    /// descent to a neighbour decides.
    #[test]
    fn steepest_descent_breaks_a_flat_gradient() {
        let ff = whole_corridor();
        let g = &ff.regions[0];
        let (w, cell) = (g.grid.width, 10 * g.grid.width + 40);
        // The four edge neighbours drop by 1 per cell length; the upper-right
        // diagonal by 1.5 over sqrt(2) cell lengths, about 1.06: it wins.
        let mut query = vec![5.0; g.grid.cell_count()];
        for (c, t) in [
            (cell - 1, 4.0),
            (cell + 1, 4.0),
            (cell - w, 4.0),
            (cell + w, 4.0),
            (cell + w + 1, 3.5),
        ] {
            query[c as usize] = t;
        }
        let (dx, dy) = steepest_descent(g, &query, cell).unwrap();
        let expected = unit(1.0, 1.0);
        assert!((dx - expected.0).abs() < 1e-12 && (dy - expected.1).abs() < 1e-12);

        // No lower neighbour at all: nothing to follow.
        let flat = vec![5.0; g.grid.cell_count()];
        assert_eq!(steepest_descent(g, &flat, cell), None);
    }

    /// Exactly between two equally near exits: still a unit vector, towards
    /// one of them.
    #[test]
    fn between_two_equal_exits_there_is_still_a_direction() {
        // 20.25 x 4.25 m: a cell is centred on the middle, (10.125, 2.125).
        let mut ff = build(vec![region(rect(0., 0., 20.25, 4.25))], vec![]).unwrap();
        let dest = ff
            .add_area_destination(&[
                piece(0, rect(0., 0., 1., 4.25)),
                piece(0, rect(19.25, 0., 20.25, 4.25)),
            ])
            .unwrap();
        // The layout is symmetric down to the last bit: the gradient vanishes,
        // and the steepest descent decides.
        let tt = ff.travel_times(0, dest).unwrap();
        let g = &ff.regions[0];
        let cell = g.local(ff.spec.lattice_cell(10.125, 2.125)).unwrap();
        let (row, col) = g.row_col(cell);
        assert_eq!(geometry::sobel_gradient(&g.grid, row, col, &tt), (0.0, 0.0));
        let (dx, dy) = ff.direction(0, 10.125, 2.125, dest).unwrap();
        assert!((dx.hypot(dy) - 1.0).abs() < 1e-12, "({dx}, {dy})");
    }

    #[test]
    fn directions_follow_the_destination_they_are_asked_for() {
        let mut ff = split_corridor();
        let left = ff.add_point_destination(0, 1.0, 2.0).unwrap();
        let right = ff.add_point_destination(1, 19.0, 2.0).unwrap();
        assert_ne!(left, right);
        // Ask in either order, repeatedly: each id keeps its own field.
        for _ in 0..2 {
            let (rx, _) = ff.direction(0, 5.0, 2.0, right).unwrap();
            let (lx, _) = ff.direction(0, 5.0, 2.0, left).unwrap();
            assert!(rx > 0.0, "towards the right end, got x = {rx}");
            assert!(lx < 0.0, "towards the left end, got x = {lx}");
        }
    }

    #[test]
    fn registered_destinations_can_be_checked() {
        let mut ff = split_corridor();
        assert_eq!(ff.destination_count(), 0);
        assert!(!ff.has_destination(0));
        let a = ff.add_point_destination(0, 1.0, 2.0).unwrap();
        let b = ff
            .add_area_destination(&[piece(1, rect(18., 1., 19., 3.))])
            .unwrap();
        assert_eq!((a, b), (0, 1));
        assert!(ff.has_destination(a) && ff.has_destination(b));
        assert!(!ff.has_destination(2));
        assert_eq!(ff.destination_count(), 2);
        // A failed registration adds nothing.
        assert!(ff.add_point_destination(0, 50., 50.).is_err());
        assert!(ff.add_area_destination(&[]).is_err());
        let invalid = |r| matches!(r, Err(MultiRegionError::InvalidInput(_)));
        assert!(invalid(
            ff.add_area_destination(&[piece(0, ring(&[1., 1., 2., 2.]))])
        ));
        let nan = ring(&[1., 1., 2., 1., f64::NAN, 2.]);
        assert!(invalid(ff.add_area_destination(&[piece(0, nan)])));
        assert_eq!(
            ff.add_area_destination(&[piece(5, rect(1., 1., 2., 2.))]),
            Err(MultiRegionError::UnknownRegion(5))
        );
        assert!(!ff.has_destination(2));
        assert_eq!(
            ff.direction(0, 5.0, 2.0, 2),
            Err(MultiRegionError::UnknownDestination(2))
        );
    }

    #[test]
    fn an_area_across_a_seam_is_zero_on_both_sides() {
        // Exit strip x in [8, 12] straddling the seam at x = 10.
        let mut ff = split_corridor();
        let dest = ff
            .add_area_destination(&[
                piece(0, rect(8., 0., 10., 4.)),
                piece(1, rect(10., 0., 12., 4.)),
            ])
            .unwrap();
        assert_eq!(time_at(&mut ff, 0, 9.0, 2.0, dest), 0.0);
        assert_eq!(time_at(&mut ff, 1, 11.0, 2.0, dest), 0.0);
        // Both ends walk to their own side of the strip, about 7 m.
        let from_left = time_at(&mut ff, 0, 1.0, 2.0, dest);
        let from_right = time_at(&mut ff, 1, 19.0, 2.0, dest);
        for t in [from_left, from_right] {
            assert!((t - 7.0).abs() < 1.0, "t = {t:.2}");
        }
        // The walk from the right end stops at the strip without crossing it.
        let path = walk(&mut ff, 1, 19.0, 2.0, dest);
        let &(r, x, _) = path.last().unwrap();
        assert!(
            r == 1 && (11.5..=12.5).contains(&x),
            "ended in {r} at x = {x}"
        );
    }

    #[test]
    fn agents_head_for_the_nearer_of_two_areas() {
        // Two separate exits at both ends of the corridor, one destination.
        let mut ff = split_corridor();
        let dest = ff
            .add_area_destination(&[
                piece(0, rect(0., 0., 1., 4.)),
                piece(1, rect(19., 0., 20., 4.)),
            ])
            .unwrap();
        let (x_west, _) = ff.direction(0, 4.0, 2.0, dest).unwrap();
        let (x_east, _) = ff.direction(1, 16.0, 2.0, dest).unwrap();
        assert!(x_west < 0.0, "x = {x_west}");
        assert!(x_east > 0.0, "x = {x_east}");
        // Inside either exit: the zero vector.
        assert_eq!(ff.direction(1, 19.5, 2.0, dest).unwrap(), (0.0, 0.0));
        assert_eq!(ff.direction(0, 0.5, 3.9, dest).unwrap(), (0.0, 0.0));
    }

    /// Upper floor F (0) with stairs S1 (1, left) and S2 (2, right) down to
    /// ground G (3). The destination in G is a bit closer to S2; an agent next
    /// to S1 still takes S1, one next to S2 takes S2.
    #[test]
    fn agent_takes_the_nearer_of_two_stairs() {
        let floor = ring(&[0., 0., 20., 0., 20., 4., 18., 4., 2., 4., 0., 4.]);
        let ground = ring(&[0., 8., 2., 8., 18., 8., 20., 8., 20., 12., 0., 12.]);
        let regions = vec![
            region(floor),
            region(rect(0., 4., 2., 8.)),   // stair 1
            region(rect(18., 4., 20., 8.)), // stair 2
            region(ground),
        ];
        // F edges 4 and 2 meet the stairs' bottom edges; the stairs' top edges
        // meet G edges 0 and 2.
        let seams = [
            join(0, 4, 1, 0),
            join(0, 2, 2, 0),
            join(1, 2, 3, 0),
            join(2, 2, 3, 2),
        ]
        .concat();
        let mut ff = build(regions, seams).unwrap();
        let dest = ff.add_point_destination(3, 12.0, 11.0).unwrap();

        for (x, stairs) in [(1.0, 1), (19.0, 2)] {
            let path = walk(&mut ff, 0, x, 2.0, dest);
            let taken = path.iter().map(|p| p.0).find(|&r| r == 1 || r == 2);
            assert_eq!(taken, Some(stairs), "from x = {x}");
            let &(r, ex, ey) = path.last().unwrap();
            assert!(
                r == 3 && (ex - 12.0).hypot(ey - 11.0) <= 1.5 * CS,
                "from x = {x}: ends in {r} at ({ex:.2}, {ey:.2})"
            );
        }
    }

    #[test]
    fn three_regions_in_a_row_propagate_through() {
        let regions = vec![
            region(rect(0., 0., 5., 4.)),
            region(rect(5., 0., 10., 4.)),
            region(rect(10., 0., 15., 4.)),
        ];
        let seams = [join(0, 1, 1, 3), join(1, 1, 2, 3)].concat();
        let mut ff = build(regions, seams).unwrap();
        let dest = ff.add_point_destination(2, 14.0, 2.0).unwrap();
        let t = time_at(&mut ff, 0, 1.0, 2.0, dest);
        assert!((t - 13.0).abs() < 1.0, "t = {t:.2}");
    }

    #[test]
    fn invalid_settings_and_queries_are_reported() {
        let bad =
            |r: Result<MultiRegionFloorfield>| matches!(r, Err(MultiRegionError::InvalidInput(_)));
        let settings = |cell_size, wall_influence_radius| FieldSettings {
            cell_size,
            wall_influence_radius,
        };
        let mut graph = RegionGraph {
            regions: vec![],
            seams: vec![],
        };
        assert!(
            bad(MultiRegionFloorfield::new(&graph, &settings(CS, 0.5))),
            "no regions"
        );
        graph.regions.push(region(rect(0., 0., 5., 5.)));
        let mut broken = graph.clone();
        broken.regions.push(region(rect(5., 0., 10., 5.)));
        broken.seams = join(0, 9, 1, 3).to_vec();
        assert!(
            bad(MultiRegionFloorfield::new(&broken, &settings(CS, 0.5))),
            "the graph is checked before anything is built"
        );
        assert!(bad(MultiRegionFloorfield::new(&graph, &settings(0.0, 0.5))));
        assert!(bad(MultiRegionFloorfield::new(
            &graph,
            &settings(CS, f64::NAN)
        )));
        for (cell_size, radius) in [
            (f64::NAN, 0.5),
            (f64::INFINITY, 0.5),
            (-CS, 0.5),
            (CS, f64::INFINITY),
            (CS, 0.0),
        ] {
            assert!(
                bad(MultiRegionFloorfield::new(
                    &graph,
                    &settings(cell_size, radius)
                )),
                "cell_size {cell_size}, radius {radius}"
            );
        }
        // A 5 x 5 m region at 0.1 mm cells needs ~2.5e9 cells: refused before
        // anything is allocated.
        assert!(bad(MultiRegionFloorfield::new(
            &graph,
            &settings(1e-4, 0.5)
        )));

        let mut ff = MultiRegionFloorfield::new(&graph, &settings(CS, 0.5)).unwrap();
        assert_eq!(
            ff.add_point_destination(3, 1., 1.),
            Err(MultiRegionError::UnknownRegion(3))
        );
        assert!(matches!(
            ff.add_point_destination(0, 50., 50.),
            Err(MultiRegionError::NotRoutable { .. })
        ));
        assert_eq!(
            ff.travel_times(0, 7),
            Err(MultiRegionError::UnknownDestination(7))
        );
    }

    #[test]
    fn one_graph_backs_several_fields() {
        let graph = RegionGraph {
            regions: vec![
                region(rect(0., 0., 10., 10.)),
                region(rect(10., 0., 20., 10.)),
            ],
            seams: join(0, 1, 1, 3).to_vec(),
        };

        let time = |cell_size| {
            let mut ff = field(&graph, cell_size).unwrap();
            let dest = ff.add_point_destination(1, 19.0, 9.0).unwrap();
            time_at(&mut ff, 0, 5.0, 1.0, dest)
        };
        // Same graph, two resolutions: both reach across the seam, and agree
        // to within the coarser grid's error.
        let (fine, coarse) = (time(CS), time(2.0 * CS));
        assert!(fine.is_finite() && coarse.is_finite());
        assert!(
            (fine - coarse).abs() < 0.05 * fine,
            "fine {fine}, coarse {coarse}"
        );
    }
}
