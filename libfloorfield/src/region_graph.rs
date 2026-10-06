// SPDX-License-Identifier: LGPL-3.0-or-later
//! The input of a multi-region floor field: checking a `RegionGraph` and its
//! area pieces, and reading the seams' segments and portals off the rings.
//!
//! The input types are shared with C++ and arrive unchecked. Everything is
//! checked here, once, before a field is built: a `MultiRegionFloorfield` only
//! exists for a graph that passed.

use crate::multi_region::MultiRegionError;
use crate::{Point2d, Region, RegionGraph, Ring, Seam};
use std::collections::HashSet;

type Result<T> = std::result::Result<T, MultiRegionError>;

/// A seam with its segment: from region `from` to region `to`, running from
/// `a` to `b` with `from` on its left.
#[derive(Clone, Copy, Debug, PartialEq)]
pub(crate) struct DirectedSeam {
    pub from: usize,
    pub to: usize,
    pub a: [f64; 2],
    pub b: [f64; 2],
}

/// Check a graph: at least one region; every ring at least 3 finite points;
/// every seam between two existing, distinct regions, through an existing
/// edge of non-zero length; every seam given in both directions, and only
/// once.
pub(crate) fn validate_graph(graph: &RegionGraph) -> Result<()> {
    let invalid = |msg: String| Err(MultiRegionError::InvalidInput(msg));
    if graph.regions.is_empty() {
        return invalid("the region graph has no regions".into());
    }
    for (id, region) in graph.regions.iter().enumerate() {
        validate_region(region)
            .map_err(|msg| MultiRegionError::InvalidInput(format!("region {id}: {msg}")))?;
    }

    let n = graph.regions.len();
    let mut directed = HashSet::new();
    for (k, seam) in graph.seams.iter().enumerate() {
        let (from, to) = (seam.from, seam.to);
        if from >= n || to >= n {
            return invalid(format!("seam {k} ({from} -> {to}): unknown region"));
        }
        if from == to {
            return invalid(format!("seam {k} joins region {from} to itself"));
        }
        let Some((a, b)) = edge(&graph.regions[from], seam) else {
            return invalid(format!(
                "seam {k} ({from} -> {to}): region {from} has no edge {} on ring {}",
                seam.index, seam.ring
            ));
        };
        if a == b {
            return invalid(format!(
                "seam {k} ({from} -> {to}): edge {} of ring {} has zero length",
                seam.index, seam.ring
            ));
        }
        if !directed.insert((from, to)) {
            return invalid(format!("seam {k}: {from} -> {to} is given twice"));
        }
    }
    if let Some(&(from, to)) = directed
        .iter()
        .find(|&&(from, to)| !directed.contains(&(to, from)))
    {
        return invalid(format!(
            "seam {from} -> {to} is missing its reverse direction"
        ));
    }
    Ok(())
}

fn validate_region(region: &Region) -> std::result::Result<(), String> {
    check_ring(&region.outer).map_err(|msg| format!("outer ring {msg}"))?;
    for (k, hole) in region.holes.iter().enumerate() {
        check_ring(hole).map_err(|msg| format!("hole {k} {msg}"))?;
    }
    Ok(())
}

/// A ring needs at least 3 points, all finite.
pub(crate) fn check_ring(ring: &Ring) -> std::result::Result<(), String> {
    if ring.points.len() < 3 {
        return Err("needs at least 3 points".into());
    }
    if !ring
        .points
        .iter()
        .all(|p| p.x.is_finite() && p.y.is_finite())
    {
        return Err("has non-finite coordinates".into());
    }
    Ok(())
}

/// End points of the edge a seam runs through, if `from` has that edge.
fn edge(from: &Region, seam: &Seam) -> Option<(Point2d, Point2d)> {
    let ring = match seam.ring as usize {
        0 => &from.outer,
        k => from.holes.get(k - 1)?,
    };
    let (i, n) = (seam.index as usize, ring.points.len());
    (i < n).then(|| (ring.points[i], ring.points[(i + 1) % n]))
}

/// The seams of a validated graph with their segments, in input order.
pub(crate) fn directed_seams(graph: &RegionGraph) -> Vec<DirectedSeam> {
    graph
        .seams
        .iter()
        .map(|seam| {
            let (a, b) = edge(&graph.regions[seam.from], seam).expect("a validated seam");
            DirectedSeam {
                from: seam.from,
                to: seam.to,
                a: [a.x, a.y],
                b: [b.x, b.y],
            }
        })
        .collect()
}

/// Portal edges of `region` in a validated graph, as (ring, edge index): the
/// edges its seams run through.
pub(crate) fn portals(graph: &RegionGraph, region: usize) -> HashSet<(u32, u32)> {
    graph
        .seams
        .iter()
        .filter(|seam| seam.from == region)
        .map(|seam| (seam.ring, seam.index))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::test_support::*;

    /// Two rooms joined along x = 5; the second has a hole.
    fn two_rooms() -> RegionGraph {
        let mut east = region(rect(5., 0., 10., 4.));
        east.holes.push(ring(&[7., 1., 7., 2., 8., 2., 8., 1.]));
        RegionGraph {
            regions: vec![region(rect(0., 0., 5., 4.)), east],
            seams: join(0, 1, 1, 3).to_vec(),
        }
    }

    /// Whether `graph` is rejected, with `what` in the message.
    fn rejected(graph: &RegionGraph, what: &str) -> bool {
        match validate_graph(graph) {
            Err(MultiRegionError::InvalidInput(msg)) => msg.contains(what),
            _ => false,
        }
    }

    #[test]
    fn seams_are_read_off_the_rings() {
        let mut graph = two_rooms();
        validate_graph(&graph).unwrap();
        assert_eq!(
            directed_seams(&graph),
            vec![
                DirectedSeam {
                    from: 0,
                    to: 1,
                    a: [5., 0.],
                    b: [5., 4.]
                },
                DirectedSeam {
                    from: 1,
                    to: 0,
                    a: [5., 4.],
                    b: [5., 0.]
                },
            ]
        );
        assert_eq!(portals(&graph, 0), HashSet::from([(0, 1)]));
        assert_eq!(portals(&graph, 1), HashSet::from([(0, 3)]));

        // A seam through a hole edge: (7, 2) -> (8, 2), the hole's edge 1.
        graph.regions.push(region(rect(7., 1., 8., 2.)));
        graph.seams.push(Seam {
            from: 1,
            to: 2,
            ring: 1,
            index: 1,
        });
        graph.seams.push(Seam {
            from: 2,
            to: 1,
            ring: 0,
            index: 2,
        });
        validate_graph(&graph).unwrap();
        assert_eq!(directed_seams(&graph)[2].a, [7., 2.]);
        assert_eq!(directed_seams(&graph)[2].b, [8., 2.]);
        assert_eq!(portals(&graph, 1), HashSet::from([(0, 3), (1, 1)]));
    }

    #[test]
    fn bad_regions_are_rejected() {
        let empty = RegionGraph {
            regions: vec![],
            seams: vec![],
        };
        assert!(rejected(&empty, "no regions"));

        let mut g = two_rooms();
        g.regions[0].outer.points.truncate(2);
        assert!(rejected(&g, "region 0: outer ring needs at least 3 points"));

        let mut g = two_rooms();
        g.regions[1].holes[0].points[2].y = f64::NAN;
        assert!(rejected(&g, "region 1: hole 0 has non-finite"));

        let mut g = two_rooms();
        g.regions[0].outer.points[0].x = f64::INFINITY;
        assert!(rejected(&g, "non-finite"));
    }

    #[test]
    fn bad_seams_are_rejected() {
        let with = |change: fn(&mut RegionGraph)| {
            let mut g = two_rooms();
            change(&mut g);
            g
        };
        assert!(rejected(&with(|g| g.seams[0].to = 2), "unknown region"));
        assert!(rejected(&with(|g| g.seams[0].to = 0), "to itself"));
        assert!(rejected(
            &with(|g| g.seams[0].index = 4),
            "has no edge 4 on ring 0"
        ));
        assert!(rejected(
            &with(|g| g.seams[0].ring = 1),
            "has no edge 1 on ring 1"
        ));
        assert!(rejected(
            &with(|g| g.seams[1].ring = 2),
            "has no edge 3 on ring 2"
        ));
        assert!(rejected(
            &with(|g| g.regions[0].outer.points[2] = g.regions[0].outer.points[1]),
            "zero length"
        ));
        assert!(rejected(
            &with(|g| {
                g.seams.pop();
            }),
            "missing its reverse"
        ));
        assert!(rejected(
            &with(|g| g.seams.push(g.seams[0])),
            "0 -> 1 is given twice"
        ));
    }
}
