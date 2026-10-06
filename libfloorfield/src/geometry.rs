// SPDX-License-Identifier: LGPL-3.0-or-later
//! Grid geometry for the floor fields: everything between a walkable polygon
//! and the cell arrays the eikonal solver works on.
//!
//! * `GridParams` places one grid in the world; `GridSpec` is a lattice shared
//!   by several grids, so cells of neighbouring regions line up exactly.
//! * `speed_field_on_grid` rasterises a region into a speed field: 0 outside
//!   (obstacle), up to 1 inside, slowed within `wall_influence_radius` of a
//!   wall. Portal edges bound the polygon but do not slow anyone down.
//! * `cells_inside_polygon` finds the source cells of a destination polygon;
//!   `interior_point` a point inside it.
//! * `sobel_gradient` gives the gradient of a travel-time field; agents walk
//!   against it.
//!
//! Conventions shared by all functions:
//! * Cells are row-major (`row * width + col`), row 0 at the lowest y.
//! * A cell belongs to a polygon iff its centre lies strictly inside it.
//! * Rings come checked (`region_graph`): at least 3 finite points each.

use crate::{Region, Ring};
use geo::{BoundingRect, Contains, Coord, InteriorPoint, LineString, Point, Polygon};
use rstar::{PointDistance, RTreeObject, AABB};
use std::collections::HashSet;

/// Placement of a floor-field grid in world coordinates.
#[derive(Clone, Copy, Debug)]
pub struct GridParams {
    /// World position of the lower-left corner of cell (0, 0).
    pub origin: [f64; 2],
    pub width: u32,
    pub height: u32,
    pub cell_size: f64,
}

impl GridParams {
    pub fn cell_count(&self) -> usize {
        self.width as usize * self.height as usize
    }

    /// (column, row) of the cell containing a world point. Not clamped:
    /// points outside the grid give indices outside `0..width` / `0..height`.
    pub fn cell_of(&self, x: f64, y: f64) -> [i64; 2] {
        floor_cell(self.origin, self.cell_size, x, y)
    }

    /// World centre of cell `idx` (row-major).
    pub fn cell_center(&self, idx: u32) -> (f64, f64) {
        (
            self.origin[0] + (idx % self.width) as f64 * self.cell_size + self.cell_size * 0.5,
            self.origin[1] + (idx / self.width) as f64 * self.cell_size + self.cell_size * 0.5,
        )
    }
}

/// (column, row) of the cell containing `(x, y)` on a lattice anchored at
/// `origin`.
fn floor_cell(origin: [f64; 2], cell_size: f64, x: f64, y: f64) -> [i64; 2] {
    [
        ((x - origin[0]) / cell_size).floor() as i64,
        ((y - origin[1]) / cell_size).floor() as i64,
    ]
}

/// A lattice shared by several grids: one origin, one cell size.
///
/// Grids placed with `cover` start on a lattice line, so the same world point
/// falls into the same lattice cell in every grid and converting a cell index
/// between two grids is a constant integer offset - no interpolation.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct GridSpec {
    pub origin: [f64; 2],
    pub cell_size: f64,
}

impl GridSpec {
    /// Lattice cell (column, row) containing a world point.
    pub fn lattice_cell(&self, x: f64, y: f64) -> [i64; 2] {
        floor_cell(self.origin, self.cell_size, x, y)
    }

    /// The smallest lattice-aligned grid covering the box `[min, max]`, and the
    /// lattice cell of its cell (0, 0). The box is snapped outward, so the grid
    /// overhangs it by less than one cell per side.
    pub fn cover(&self, min: [f64; 2], max: [f64; 2]) -> (GridParams, [i64; 2]) {
        let lo = self.lattice_cell(min[0], min[1]);
        let hi = self.lattice_cell(max[0], max[1]);
        let grid = GridParams {
            origin: [
                self.origin[0] + lo[0] as f64 * self.cell_size,
                self.origin[1] + lo[1] as f64 * self.cell_size,
            ],
            width: u32::try_from(hi[0] - lo[0] + 1).expect("grid width exceeds u32"),
            height: u32::try_from(hi[1] - lo[1] + 1).expect("grid height exceeds u32"),
            cell_size: self.cell_size,
        };
        (grid, lo)
    }
}

/// Rasterise a region onto an existing grid: 0.0 outside, (0, 1] inside,
/// slowed within `wall_influence_radius` of a wall.
///
/// `portals` lists edges, as (ring, edge index), that are passages rather than
/// walls: they still bound the polygon - cells beyond them stay 0 - but do not
/// slow anyone down. Ring 0 is the outer boundary, ring k is hole k - 1.
pub fn speed_field_on_grid(
    region: &Region,
    portals: &HashSet<(u32, u32)>,
    grid: &GridParams,
    wall_influence_radius: f64,
) -> Vec<f64> {
    let polygon = polygon(&region.outer, &region.holes);
    let tree = rstar::RTree::bulk_load(wall_segments(region, portals));

    let mut speed_field = vec![0.0f64; grid.cell_count()];
    for (idx, speed) in speed_field.iter_mut().enumerate() {
        let (px, py) = grid.cell_center(idx as u32);
        if !polygon.contains(&Point::new(px, py)) {
            continue; // outside: stays an obstacle
        }
        let dist2 = tree
            .nearest_neighbor_iter_with_distance_2(&[px, py])
            .next()
            .map(|(_, d2)| d2)
            .unwrap_or(f64::INFINITY); // no walls at all
        *speed = (dist2.sqrt() / wall_influence_radius).clamp(0.0, 1.0);
    }
    speed_field
}

/// Sobel gradient of a travel-time field at a cell. Out-of-grid and
/// unreachable neighbours fall back to the centre value, so the gradient stays
/// finite next to walls.
pub fn sobel_gradient(grid: &GridParams, row: i32, col: i32, tt: &[f64]) -> (f64, f64) {
    let w = grid.width as i32;
    let h = grid.height as i32;
    let tc = tt[row as usize * grid.width as usize + col as usize];
    if !tc.is_finite() {
        return (0.0, 0.0);
    }
    let get = |r: i32, c: i32| -> f64 {
        if r < 0 || r >= h || c < 0 || c >= w {
            return tc;
        }
        let v = tt[r as usize * grid.width as usize + c as usize];
        if v.is_infinite() {
            tc
        } else {
            v
        }
    };
    let gx = (get(row - 1, col + 1) + 2. * get(row, col + 1) + get(row + 1, col + 1))
        - (get(row - 1, col - 1) + 2. * get(row, col - 1) + get(row + 1, col - 1));
    let gy = (get(row + 1, col - 1) + 2. * get(row + 1, col) + get(row + 1, col + 1))
        - (get(row - 1, col - 1) + 2. * get(row - 1, col) + get(row - 1, col + 1));
    (gx / (8.0 * grid.cell_size), gy / (8.0 * grid.cell_size))
}

/// Cells whose centre lies inside the given polygon, as row-major indices.
/// Empty when the polygon covers no cell centre; callers pick a fallback.
pub fn cells_inside_polygon(grid: &GridParams, polygon: &Polygon<f64>) -> Vec<u32> {
    // Only cells under the polygon's bounding box can qualify. Clamp in i64:
    // a polygon beside the grid gives an empty range, not a wrapped one.
    let Some(bbox) = polygon.bounding_rect() else {
        return Vec::new();
    };
    let (min, max) = (bbox.min(), bbox.max());
    let (lo, hi) = (grid.cell_of(min.x, min.y), grid.cell_of(max.x, max.y));
    let (c0, c1) = (lo[0].max(0), hi[0].min(grid.width as i64 - 1));
    let (r0, r1) = (lo[1].max(0), hi[1].min(grid.height as i64 - 1));

    // Row-major order, so the result is sorted and unique.
    let mut cells: Vec<u32> = Vec::new();
    for row in r0..=r1 {
        for col in c0..=c1 {
            let idx = row as u32 * grid.width + col as u32;
            let (px, py) = grid.cell_center(idx);
            if polygon.contains(&Point::new(px, py)) {
                cells.push(idx);
            }
        }
    }
    cells
}

/// The polygon bounded by `outer`, with `holes` cut out.
pub fn polygon(outer: &Ring, holes: &[Ring]) -> Polygon<f64> {
    Polygon::new(line_string(outer), holes.iter().map(line_string).collect())
}

fn line_string(ring: &Ring) -> LineString<f64> {
    ring.points
        .iter()
        .map(|p| Coord { x: p.x, y: p.y })
        .collect()
}

/// A point guaranteed to lie inside the polygon - unlike its centroid or its
/// bounding box centre, which can fall outside an L or a ring.
pub fn interior_point(polygon: &Polygon<f64>) -> [f64; 2] {
    let p = polygon
        .interior_point()
        .expect("a checked ring has at least 3 points");
    [p.x(), p.y()]
}

/// Axis-aligned bounds of a ring.
pub fn bounding_box(ring: &Ring) -> ([f64; 2], [f64; 2]) {
    let mut min = [f64::INFINITY; 2];
    let mut max = [f64::NEG_INFINITY; 2];
    for p in &ring.points {
        min = [min[0].min(p.x), min[1].min(p.y)];
        max = [max[0].max(p.x), max[1].max(p.y)];
    }
    (min, max)
}

/// The edges of all rings of `region` that are walls, i.e. not portals. Edge
/// i of a ring runs from point i to point i + 1; the last one closes the ring.
fn wall_segments(region: &Region, portals: &HashSet<(u32, u32)>) -> Vec<BoundarySeg> {
    let mut segs = Vec::new();
    for (k, ring) in std::iter::once(&region.outer)
        .chain(&region.holes)
        .enumerate()
    {
        let pts = &ring.points;
        for i in 0..pts.len() {
            if portals.contains(&(k as u32, i as u32)) {
                continue;
            }
            let (p0, p1) = (pts[i], pts[(i + 1) % pts.len()]);
            segs.push(BoundarySeg {
                p0: [p0.x, p0.y],
                p1: [p1.x, p1.y],
            });
        }
    }
    segs
}

#[derive(Clone)]
struct BoundarySeg {
    p0: [f64; 2],
    p1: [f64; 2],
}

impl RTreeObject for BoundarySeg {
    type Envelope = AABB<[f64; 2]>;

    fn envelope(&self) -> Self::Envelope {
        AABB::from_corners(
            [self.p0[0].min(self.p1[0]), self.p0[1].min(self.p1[1])],
            [self.p0[0].max(self.p1[0]), self.p0[1].max(self.p1[1])],
        )
    }
}

impl PointDistance for BoundarySeg {
    fn distance_2(&self, point: &[f64; 2]) -> f64 {
        seg_point_dist2(self.p0, self.p1, *point)
    }

    fn contains_point(&self, point: &<Self::Envelope as rstar::Envelope>::Point) -> bool {
        self.distance_2(point) == 0.0
    }
}

fn seg_point_dist2(p0: [f64; 2], p1: [f64; 2], q: [f64; 2]) -> f64 {
    let dx = p1[0] - p0[0];
    let dy = p1[1] - p0[1];
    let len2 = dx * dx + dy * dy;
    if len2 == 0.0 {
        let ex = q[0] - p0[0];
        let ey = q[1] - p0[1];
        return ex * ex + ey * ey;
    }
    let t = (((q[0] - p0[0]) * dx + (q[1] - p0[1]) * dy) / len2).clamp(0.0, 1.0);
    let cx = p0[0] + t * dx - q[0];
    let cy = p0[1] + t * dy - q[1];
    cx * cx + cy * cy
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::test_support::*;

    fn square(size: f64) -> Region {
        region(rect(0.0, 0.0, size, size))
    }

    fn no_portals() -> HashSet<(u32, u32)> {
        HashSet::new()
    }

    /// A 10 x 10 grid of 1 m cells over `square(10.0)`.
    fn unit_grid() -> GridParams {
        GridParams {
            origin: [0.0, 0.0],
            width: 10,
            height: 10,
            cell_size: 1.0,
        }
    }

    #[test]
    fn cell_of_floors_and_does_not_clamp() {
        let grid = unit_grid();
        assert_eq!(grid.cell_of(3.7, 0.2), [3, 0]);
        assert_eq!(grid.cell_of(-0.5, 10.5), [-1, 10]);
    }

    #[test]
    fn cells_inside_polygon_scans_only_the_bounding_box() {
        let grid = unit_grid();
        // Cell centres (2.5, 3.5) and (3.5, 3.5) lie inside.
        let cells = |r: Ring| cells_inside_polygon(&grid, &polygon(&r, &[]));
        assert_eq!(cells(rect(2.0, 3.0, 4.0, 4.0)), vec![32, 33]);

        // Sticking out past the grid: only the cells on the grid count.
        assert_eq!(cells(rect(8.0, 8.0, 12.0, 12.0)), vec![88, 89, 98, 99]);

        // Entirely beside the grid, on either side: nothing, and no wrap-around.
        assert!(cells(rect(-5.0, 2.0, -3.0, 4.0)).is_empty());
        assert!(cells(rect(12.0, 2.0, 14.0, 4.0)).is_empty());
    }

    #[test]
    fn interior_is_walkable_and_slows_near_walls() {
        let grid = unit_grid();
        let speed = speed_field_on_grid(&square(10.0), &no_portals(), &grid, 2.0);
        assert_eq!(speed.len(), 100);
        let at = |r: usize, c: usize| speed[r * grid.width as usize + c];
        assert!(at(5, 5) > 0.99, "centre should be at full speed");
        assert!(
            at(0, 0) < at(5, 5),
            "corner cell must be slowed by the wall"
        );
        assert!(at(0, 0) > 0.0, "an interior cell is never an obstacle");
    }

    #[test]
    fn hole_cells_are_obstacles() {
        let grid = unit_grid();
        let mut room = square(10.0);
        room.holes
            .push(ring(&[4.0, 4.0, 4.0, 6.0, 6.0, 6.0, 6.0, 4.0]));
        let speed = speed_field_on_grid(&room, &no_portals(), &grid, 0.5);
        // Cell centre (5.5, 5.5) sits inside the hole.
        assert_eq!(speed[5 * grid.width as usize + 5], 0.0);
    }

    #[test]
    fn lattice_grids_align_across_regions() {
        let spec = GridSpec {
            origin: [0.0, 0.0],
            cell_size: 0.25,
        };
        // Upper floor starts off-lattice at x = 12.3.
        let (ground, ground_lo) = spec.cover([0.0, 0.0], [20.0, 10.0]);
        let (upper, upper_lo) = spec.cover([12.3, 0.0], [17.3, 10.0]);

        assert_eq!(ground.origin, [0.0, 0.0]);
        assert_eq!(upper.origin[0], 12.25, "snapped down onto the lattice");
        assert!(
            upper.origin[0] + upper.width as f64 * 0.25 >= 17.3,
            "covers the box"
        );

        // The same world point lands in cells exactly `lo` apart.
        let cell = spec.lattice_cell(14.1, 3.3);
        let in_ground = [cell[0] - ground_lo[0], cell[1] - ground_lo[1]];
        let in_upper = [cell[0] - upper_lo[0], cell[1] - upper_lo[1]];
        assert_eq!(in_ground[0] - in_upper[0], upper_lo[0] - ground_lo[0]);
        let centre =
            |g: &GridParams, c: [i64; 2]| g.cell_center((c[1] as u32) * g.width + c[0] as u32);
        let (a, b) = (centre(&ground, in_ground), centre(&upper, in_upper));
        assert!((a.0 - b.0).abs() < 1e-12 && (a.1 - b.1).abs() < 1e-12);
    }

    #[test]
    fn portal_edges_do_not_slow_agents() {
        // 4 x 4 room; edge 1 (x = 4, from (4,0) to (4,4)) is a portal.
        let room = square(4.0);
        // One extra column (x in [4, 5]) lies beyond the portal.
        let grid = GridParams {
            origin: [0.0, 0.0],
            width: 5,
            height: 4,
            cell_size: 1.0,
        };
        let walls = speed_field_on_grid(&room, &no_portals(), &grid, 2.0);
        let portal = speed_field_on_grid(&room, &HashSet::from([(0, 1)]), &grid, 2.0);

        let at = |f: &[f64], r: usize, c: usize| f[r * 5 + c];
        // Next to that edge (column 3, middle rows) agents are faster when it is a portal...
        assert!(at(&portal, 2, 3) > at(&walls, 2, 3));
        // ...next to the opposite wall (column 0) nothing changes.
        assert_eq!(at(&portal, 2, 0), at(&walls, 2, 0));
        // The portal still bounds the polygon: the column beyond it is not walkable.
        assert!((0..4).all(|r| at(&portal, r, 4) == 0.0));
        assert!((0..4).all(|r| at(&portal, r, 3) > 0.0));
    }

    #[test]
    fn portals_on_hole_edges_do_not_slow_agents() {
        // A hole at x in [4, 6], y in [4, 6]; its edge 0 runs along x = 4.
        let mut room = square(10.0);
        room.holes
            .push(ring(&[4.0, 4.0, 4.0, 6.0, 6.0, 6.0, 6.0, 4.0]));
        let walls = speed_field_on_grid(&room, &no_portals(), &unit_grid(), 1.5);
        let portal = speed_field_on_grid(&room, &HashSet::from([(1, 0)]), &unit_grid(), 1.5);

        // Cell (row 4, col 3), centre (3.5, 4.5), lies 0.5 from that edge.
        let at = |f: &[f64]| f[4 * 10 + 3];
        assert!(at(&portal) > at(&walls));
        // The hole stays an obstacle either way.
        assert_eq!(portal[5 * 10 + 5], 0.0);
    }
}
