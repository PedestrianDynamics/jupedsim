// SPDX-License-Identifier: LGPL-3.0-or-later
//! Static floor fields.
//!
//! Static means: the geometry and the speed field never change. Agent density
//! plays no part, and a solved field is never recomputed.
//!
//! A `MultiRegionFloorfield` owns one grid and one speed field per region of
//! a `RegionGraph`, joined along seams. A single walkable polygon is the
//! special case of a graph with one region and no seams. The input types are
//! plain structs shared with C++; they are checked once, when a field is built
//! from them. Destinations are registered up front; each is solved on first
//! use with the Fast Sweeping Method and then kept.

pub mod fsm;
pub mod geometry;
pub mod multi_region;
pub mod region_graph;

pub use ffi::{AreaPiece, GridInfo, Point2d, Region, RegionGraph, Ring, Seam};
use multi_region::{FieldSettings, MultiRegionError, MultiRegionFloorfield};

// ── cxx bridge ──────────────────────────────────────────────────────────────

#[cxx::bridge(namespace = "jupedsim::floorfield")]
mod ffi {
    /// 2-D point passed between Rust and C++.
    #[derive(Clone, Copy, Debug, PartialEq)]
    struct Point2d {
        x: f64,
        y: f64,
    }

    /// A closed polygon ring, its vertices in order; the closing vertex is not
    /// repeated.
    #[derive(Clone, Debug, PartialEq)]
    struct Ring {
        points: Vec<Point2d>,
    }

    /// One walkable region: a polygon with holes. Boundaries run
    /// counterclockwise, holes clockwise, so the region lies left of every
    /// edge.
    #[derive(Clone, Debug, PartialEq)]
    struct Region {
        outer: Ring,
        holes: Vec<Ring>,
    }

    /// Region `from` opens into region `to` through edge `index` of its ring
    /// `ring` (ring 0 is the outer boundary, ring k is hole k - 1; edge i runs
    /// from point i to point i + 1, the last one closing the ring). Every seam
    /// is given in both directions, each through the edge of its own `from`
    /// region. Seam edges are portals; all other edges are walls.
    #[derive(Clone, Copy, Debug, PartialEq)]
    struct Seam {
        from: usize,
        to: usize,
        ring: u32,
        index: u32,
    }

    /// Regions and the seams joining them: the input of a multi-region floor
    /// field, laid out like the C++ region graph (`Geometry::RegionGraph2D`).
    /// One graph can back several fields.
    #[derive(Clone, Debug, PartialEq)]
    struct RegionGraph {
        regions: Vec<Region>,
        seams: Vec<Seam>,
    }

    /// The part of an area destination lying in one region: a polygon
    /// without holes.
    #[derive(Clone, Debug, PartialEq)]
    struct AreaPiece {
        region: usize,
        outer: Ring,
    }

    /// Placement of one region's grid.
    struct GridInfo {
        origin_x: f64,
        origin_y: f64,
        width: u32,
        height: u32,
        cell_size: f64,
    }

    extern "Rust" {
        type MultiRegionFloorfield;

        /// Floor fields over the regions of `graph`, which is checked first.
        /// The field keeps no reference to the graph.
        fn new_multi_region_floorfield(
            graph: &RegionGraph,
            cell_size: f64,
            wall_influence_radius: f64,
        ) -> Result<Box<MultiRegionFloorfield>>;

        /// Where `region`'s grid lies; `region_travel_times` is row-major on it.
        fn region_grid(self: &MultiRegionFloorfield, region: usize) -> Result<GridInfo>;
        /// Whether `p` lies in `region`'s polygon with a walkable cell near
        /// enough to take a direction from.
        fn region_is_routable(self: &MultiRegionFloorfield, region: usize, p: Point2d) -> bool;

        /// A destination at `p` in `region`, snapped to the nearest walkable
        /// cell within two cells. Throws if there is none. Returns its id.
        #[cxx_name = "add_point_destination"]
        fn add_point_destination_ffi(
            self: &mut MultiRegionFloorfield,
            region: usize,
            p: Point2d,
        ) -> Result<usize>;
        /// One destination made of several polygons, each within one region:
        /// every walkable cell whose centre lies in a piece is part of it.
        /// Agents head for whichever piece is nearest in travel time.
        fn add_area_destination(
            self: &mut MultiRegionFloorfield,
            pieces: &[AreaPiece],
        ) -> Result<usize>;
        /// Whether `dest` was registered with this field.
        fn has_destination(self: &MultiRegionFloorfield, dest: usize) -> bool;

        /// Which way to walk from `p` in `region` towards `dest`: a unit
        /// vector downhill, or the zero vector inside an area destination
        /// (inside a point destination's cell: towards the point, zero exactly
        /// on it). Throws where the destination cannot be reached from `p`.
        /// Solves the destination on first use. The caller tracks which region
        /// a moving agent is in, also across seams (the geometry does).
        fn region_direction(
            self: &mut MultiRegionFloorfield,
            region: usize,
            p: Point2d,
            dest: usize,
        ) -> Result<Point2d>;
        /// Travel time to `dest` for every cell of `region`'s grid, infinite
        /// where it cannot be reached. Solves the destination on first use.
        fn region_travel_times(
            self: &mut MultiRegionFloorfield,
            region: usize,
            dest: usize,
        ) -> Result<Vec<f64>>;
    }
}

// ── Multi-region bridge ─────────────────────────────────────────────────────

fn new_multi_region_floorfield(
    graph: &RegionGraph,
    cell_size: f64,
    wall_influence_radius: f64,
) -> Result<Box<MultiRegionFloorfield>, MultiRegionError> {
    let settings = FieldSettings {
        cell_size,
        wall_influence_radius,
    };
    MultiRegionFloorfield::new(graph, &settings).map(Box::new)
}

impl MultiRegionFloorfield {
    fn region_grid(&self, region: usize) -> Result<GridInfo, MultiRegionError> {
        let g = self.grid(region)?;
        Ok(GridInfo {
            origin_x: g.origin[0],
            origin_y: g.origin[1],
            width: g.width,
            height: g.height,
            cell_size: g.cell_size,
        })
    }

    fn region_is_routable(&self, region: usize, p: Point2d) -> bool {
        self.is_routable(region, p.x, p.y)
    }

    fn add_point_destination_ffi(
        &mut self,
        region: usize,
        p: Point2d,
    ) -> Result<usize, MultiRegionError> {
        self.add_point_destination(region, p.x, p.y)
    }

    fn region_direction(
        &mut self,
        region: usize,
        p: Point2d,
        dest: usize,
    ) -> Result<Point2d, MultiRegionError> {
        let (x, y) = self.direction(region, p.x, p.y, dest)?;
        Ok(Point2d { x, y })
    }

    fn region_travel_times(
        &mut self,
        region: usize,
        dest: usize,
    ) -> Result<Vec<f64>, MultiRegionError> {
        self.travel_times(region, dest)
    }
}

#[cfg(test)]
pub(crate) mod test_support {
    //! Building input structs in tests.
    use super::*;

    pub fn ring(xy: &[f64]) -> Ring {
        Ring {
            points: xy
                .chunks_exact(2)
                .map(|c| Point2d { x: c[0], y: c[1] })
                .collect(),
        }
    }

    /// Axis-aligned rectangle, counterclockwise from (x0, y0). Edges: 0
    /// bottom, 1 right, 2 top, 3 left.
    pub fn rect(x0: f64, y0: f64, x1: f64, y1: f64) -> Ring {
        ring(&[x0, y0, x1, y0, x1, y1, x0, y1])
    }

    /// A region without holes.
    pub fn region(outer: Ring) -> Region {
        Region {
            outer,
            holes: Vec::new(),
        }
    }

    /// Regions `a` and `b` joined through outer edges `edge_a` and `edge_b`:
    /// the seam in both directions.
    pub fn join(a: usize, edge_a: u32, b: usize, edge_b: u32) -> [Seam; 2] {
        [
            Seam {
                from: a,
                to: b,
                ring: 0,
                index: edge_a,
            },
            Seam {
                from: b,
                to: a,
                ring: 0,
                index: edge_b,
            },
        ]
    }

    pub fn piece(region: usize, outer: Ring) -> AreaPiece {
        AreaPiece { region, outer }
    }
}

#[cfg(test)]
mod tests {
    use super::test_support::*;
    use super::*;

    fn pt(x: f64, y: f64) -> Point2d {
        Point2d { x, y }
    }

    /// Travel time at a world point in `region`.
    fn tt_at(ff: &MultiRegionFloorfield, tt: &[f64], region: usize, x: f64, y: f64) -> f64 {
        let g = ff.grid(region).unwrap();
        let [col, row] = g.cell_of(x, y);
        tt[row as usize * g.width as usize + col as usize]
    }

    /// A single walkable polygon is a graph with one region and no seams.
    fn room() -> Box<MultiRegionFloorfield> {
        // 20 x 10 room, 0.25 m cells.
        let graph = RegionGraph {
            regions: vec![region(rect(0.0, 0.0, 20.0, 10.0))],
            seams: vec![],
        };
        new_multi_region_floorfield(&graph, 0.25, 0.5).unwrap()
    }

    #[test]
    fn single_region_routable_inside_not_outside() {
        let ff = room();
        assert!(ff.region_is_routable(0, pt(10.0, 5.0)));
        assert!(!ff.region_is_routable(0, pt(-1.0, 5.0)));
        assert!(!ff.region_is_routable(0, pt(25.0, 5.0)));
    }

    #[test]
    fn single_region_travel_time_grows_with_distance_from_the_exit() {
        let mut ff = room();
        let exit = ff.add_point_destination(0, 0.5, 5.0).unwrap();
        let tt = ff.region_travel_times(0, exit).unwrap();
        let at = |x: f64, y: f64| tt_at(&ff, &tt, 0, x, y);
        assert!(at(5.0, 5.0) < at(10.0, 5.0));
        assert!(at(10.0, 5.0) < at(19.0, 5.0));
    }

    #[test]
    fn single_region_direction_is_a_unit_vector_toward_the_exit() {
        let mut ff = room();
        let exit = ff.add_point_destination(0, 0.5, 5.0).unwrap();
        let d = ff.region_direction(0, pt(15.0, 5.0), exit).unwrap();
        assert!((d.x.hypot(d.y) - 1.0).abs() < 1e-12, "unit length: {d:?}");
        assert!(
            d.x < -0.99,
            "should point toward the exit at x=0.5, got {d:?}"
        );
    }

    #[test]
    fn single_region_directions_lead_to_the_exit() {
        let mut ff = room();
        let exit = ff.add_point_destination(0, 0.5, 5.0).unwrap();
        let tt = ff.region_travel_times(0, exit).unwrap();
        // One region: following the directions needs no region tracking.
        // Steps of one cell length, until the exit's cell (travel time 0).
        let (mut x, mut y) = (18.0, 5.0);
        let mut steps = 0;
        while tt_at(&ff, &tt, 0, x, y) > 0.0 && steps < 1000 {
            let d = ff.region_direction(0, pt(x, y), exit).unwrap();
            (x, y) = (x + 0.25 * d.x, y + 0.25 * d.y);
            steps += 1;
        }
        assert!(steps > 2, "expected several steps");
        assert!(
            (x - 0.5).hypot(y - 5.0) <= 0.25,
            "should end at the exit, got ({x}, {y})"
        );
    }

    /// Two rooms side by side, joined along x = 5; the second room has a
    /// 1 x 1 hole.
    #[test]
    fn graph_with_a_seam_and_a_hole_backs_a_field() {
        let mut east = region(rect(5., 0., 10., 4.));
        east.holes.push(ring(&[7., 1., 7., 2., 8., 2., 8., 1.]));
        let graph = RegionGraph {
            regions: vec![region(rect(0., 0., 5., 4.)), east],
            seams: join(0, 1, 1, 3).to_vec(),
        };

        let mut ff = new_multi_region_floorfield(&graph, 0.25, 0.5).unwrap();
        assert!(ff.seam_pair_count(0) > 0 && ff.seam_pair_count(1) > 0);
        assert!(ff.region_is_routable(1, pt(9.0, 3.0)));
        assert!(!ff.region_is_routable(1, pt(7.5, 1.5)), "inside the hole");

        let dest = ff.add_point_destination(1, 9.5, 3.5).unwrap();
        let tt = ff.region_travel_times(0, dest).unwrap();
        assert!(
            tt_at(&ff, &tt, 0, 1.0, 1.0).is_finite(),
            "the field reaches across the seam"
        );
        let d = ff.region_direction(0, pt(1.0, 1.0), dest).unwrap();
        assert!(d.x > 0.0, "heads towards the seam");
    }
}
