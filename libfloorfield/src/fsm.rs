// SPDX-License-Identifier: LGPL-3.0-or-later
//! Fast Sweeping Method (Zhao 2005) for the eikonal equation |grad u| = 1/f
//! on a 2D regular grid. O(k * N), where N is the cell count and k the number
//! of sweeping rounds. k grows with the number of times the shortest paths
//! turn: open space settles in a round or two, a maze takes many.

/// Initialises `out` to INFINITY, pins every source cell to zero, and sweeps
/// to convergence. `sources` are row-major cell indices (`row * width + col`).
pub fn solve_into(
    out: &mut [f64],
    speed_field: &[f64],
    sources: &[u32],
    width: usize,
    height: usize,
    cell_size: f64,
) {
    out.fill(f64::INFINITY);
    solve_into_seeded(out, speed_field, sources, &[], width, height, cell_size);
}

/// Like `solve_into`, but source `i` starts at `seed_values[i]` instead of 0:
/// the arrival time of a wave that entered from elsewhere. Empty
/// `seed_values` means all zero. A seed is an upper bound, not a pin - a
/// cheaper route through the grid still lowers it.
///
/// `out` is not reset: it holds the starting values, and sweeps only ever
/// lower them. It must be an upper bound on the result:
/// * INFINITY everywhere: a cold start.
/// * A previous solution on the same grid and speed field whose sources and
///   seeds have since only been added to or lowered: a warm start. The result
///   is the same as from a cold start, but only the cells the change reaches
///   are relaxed again.
///
/// Anything lower than the result (e.g. a zeroed buffer) is kept as it is:
/// the sweeps cannot raise it.
pub fn solve_into_seeded(
    out: &mut [f64],
    speed_field: &[f64],
    sources: &[u32],
    seed_values: &[f64],
    width: usize,
    height: usize,
    cell_size: f64,
) {
    assert!(
        seed_values.is_empty() || seed_values.len() == sources.len(),
        "seed_values must be empty or hold one value per source"
    );
    assert!(
        out.len() == width * height && speed_field.len() == width * height,
        "The output and speed field array must have the dimension of width * height."
    );

    for (i, &s) in sources.iter().enumerate() {
        if !(is_walkable(speed_field[s as usize])) {
            continue;
        }
        let t = seed_values.get(i).copied().unwrap_or(0.0);
        out[s as usize] = out[s as usize].min(t);
    }
    // One round sweeps in all four orderings of rows and columns, so a wave
    // travelling in any direction is carried across the grid in one pass.
    // Rounds repeat until one changes nothing.
    loop {
        let c0 = sweep(out, speed_field, width, height, cell_size, false, false);
        let c1 = sweep(out, speed_field, width, height, cell_size, false, true);
        let c2 = sweep(out, speed_field, width, height, cell_size, true, false);
        let c3 = sweep(out, speed_field, width, height, cell_size, true, true);
        if !(c0 | c1 | c2 | c3) {
            break;
        }
    }
}

/// One Gauss-Seidel pass over the grid, rows and columns in the given order.
/// Returns whether any cell was lowered.
fn sweep(
    u: &mut [f64],
    speed: &[f64],
    width: usize,
    height: usize,
    cell_size: f64,
    i_rev: bool,
    j_rev: bool,
) -> bool {
    let mut changed = false;
    for i in iter_range(height, i_rev) {
        for j in iter_range(width, j_rev) {
            let idx = i * width + j;
            let f = speed[idx];
            if !is_walkable(f) {
                continue; // obstacle or invalid (NaN) speed: never relaxed
            }
            let cost = cell_size / f;
            let a = min_neighbor_x(u, i, j, width);
            let b = min_neighbor_y(u, i, j, width, height);
            let candidate = godunov_update(a, b, cost);
            // Values only ever go down: that makes the sweeps converge, and it
            // is what lets a warm start keep a valid upper bound.
            if candidate < u[idx] {
                u[idx] = candidate;
                changed = true;
            }
        }
    }
    changed
}

/// Upwind Godunov update: solves (u-a)+^2 + (u-b)+^2 = cost^2
fn godunov_update(a: f64, b: f64, cost: f64) -> f64 {
    let lo = a.min(b);
    let hi = a.max(b);

    let u1 = lo + cost;
    if u1 <= hi {
        return u1;
    }

    let disc = 2.0 * cost * cost - (a - b) * (a - b);
    // Reaching this point means |a - b| < cost, so disc > cost^2 > 0.
    debug_assert!(disc >= 0.0);
    (a + b + disc.sqrt()) / 2.0
}

/// The lower of the left and right neighbours; infinite beyond the grid.
fn min_neighbor_x(u: &[f64], i: usize, j: usize, width: usize) -> f64 {
    let left = if j > 0 {
        u[i * width + j - 1]
    } else {
        f64::INFINITY
    };
    let right = if j + 1 < width {
        u[i * width + j + 1]
    } else {
        f64::INFINITY
    };
    left.min(right)
}

/// The lower of the neighbours above and below; infinite beyond the grid.
fn min_neighbor_y(u: &[f64], i: usize, j: usize, width: usize, height: usize) -> f64 {
    let up = if i > 0 {
        u[(i - 1) * width + j]
    } else {
        f64::INFINITY
    };
    let down = if i + 1 < height {
        u[(i + 1) * width + j]
    } else {
        f64::INFINITY
    };
    up.min(down)
}

/// Speed 0, negative or NaN marks a cell as not walkable.
fn is_walkable(f: f64) -> bool {
    f > 0.0
}

enum Indices {
    Fwd(std::ops::Range<usize>),
    Rev(std::iter::Rev<std::ops::Range<usize>>),
}

impl Iterator for Indices {
    type Item = usize;
    fn next(&mut self) -> Option<usize> {
        match self {
            Indices::Fwd(r) => r.next(),
            Indices::Rev(r) => r.next(),
        }
    }
}

fn iter_range(len: usize, rev: bool) -> Indices {
    if rev {
        Indices::Rev((0..len).rev())
    } else {
        Indices::Fwd(0..len)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::geometry::{sobel_gradient, GridParams};

    /// On a uniform speed field the travel time must equal euclidean distance.
    #[test]
    fn uniform_point_source_matches_euclidean() {
        let (w, h) = (101usize, 101usize);
        let (cx, cy) = (w / 2, h / 2);
        let speed = vec![1.0f64; w * h];
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[(cy * w + cx) as u32], w, h, 1.0);

        // The 4-neighbour Godunov stencil is first order: axis-aligned
        // distances are nearly exact, diagonals are overestimated by a few
        // percent. The looser diagonal tolerance reflects that, not a bug.

        for ((i, j), tol) in [
            ((cy, cx + 10), 0.02),
            ((cy + 10, cx), 0.02),
            ((cy + 10, cx + 10), 0.08),
        ] {
            let expected =
                (((i as f64 - cy as f64).powi(2) + (j as f64 - cx as f64).powi(2)).sqrt()).abs();
            let got = u[i * w + j];
            assert!(
                (got - expected).abs() / expected < tol,
                "({i},{j}): expected {expected:.3}, got {got:.3}"
            );
        }
    }

    /// A uniform seed shifts the whole field by exactly that amount.
    #[test]
    fn seeding_shifts_the_field() {
        let (w, h) = (30usize, 20usize);
        let speed = vec![1.0f64; w * h];
        let sources: Vec<u32> = (0..h as u32).map(|r| r * w as u32).collect(); // left column
        let mut zero = vec![0.0; w * h];
        let mut shifted = vec![f64::INFINITY; w * h];
        solve_into(&mut zero, &speed, &sources, w, h, 0.5);
        solve_into_seeded(&mut shifted, &speed, &sources, &vec![3.0; h], w, h, 0.5);
        for (a, b) in zero.iter().zip(&shifted) {
            assert!((b - a - 3.0).abs() < 1e-12);
        }
    }

    /// A warm start from a previous solution whose seeds have since dropped
    /// ends where a cold start does.
    #[test]
    fn warm_start_from_a_previous_solution_matches_a_cold_start() {
        let (w, h) = (30usize, 20usize);
        let mut speed = vec![1.0f64; w * h];
        for r in 0..15 {
            speed[r * w + 15] = 0.0; // a wall with a gap at the top
        }
        let sources: Vec<u32> = (0..h as u32).map(|r| r * w as u32).collect(); // left column
        let mut seeds = vec![5.0; h];

        let mut warm = vec![f64::INFINITY; w * h];
        solve_into_seeded(&mut warm, &speed, &sources, &seeds, w, h, 0.5);
        seeds[3] = 1.0;
        seeds[17] = 2.0;
        solve_into_seeded(&mut warm, &speed, &sources, &seeds, w, h, 0.5);

        let mut cold = vec![f64::INFINITY; w * h];
        solve_into_seeded(&mut cold, &speed, &sources, &seeds, w, h, 0.5);
        for (i, (a, b)) in warm.iter().zip(&cold).enumerate() {
            assert!(
                a == b || (a - b).abs() < 1e-12,
                "cell {i}: warm {a}, cold {b}"
            );
        }
    }

    /// Unequal seeds: a later seed is corrected if a cheaper route reaches it.
    #[test]
    fn inconsistent_seed_is_lowered() {
        let (w, h) = (10usize, 1usize);
        let speed = vec![1.0f64; w * h];
        let mut u = vec![f64::INFINITY; w * h];
        solve_into_seeded(&mut u, &speed, &[0, 5], &[0.0, 100.0], w, h, 1.0);
        assert!(
            (u[5] - 5.0).abs() < 1e-12,
            "seed 100 must drop to 5, got {}",
            u[5]
        );
    }

    /// The wave has to bend around a wall through a gap: the travel time is
    /// the detour via the gap, not the straight distance through the wall.
    #[test]
    fn wave_bends_around_wall_through_gap() {
        let (w, h) = (21usize, 21usize);
        let mut speed = vec![1.0f64; w * h];
        for row in 0..18 {
            speed[row * w + 10] = 0.0; // wall in column 10, gap in rows 18..21
        }
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[(10 * w + 2) as u32], w, h, 1.0);

        let got = u[10 * w + 18];
        let straight = 16.0;
        // Shortest path passes the gap at (18, 10): two legs of length sqrt(8^2 + 8^2).
        let detour = 2.0 * (8.0f64 * 8.0 + 8.0 * 8.0).sqrt();
        assert!(
            got > straight * 1.2,
            "wave leaked through the wall: {got:.3}"
        );
        // Both legs run diagonally, where the first-order stencil
        // overestimates the most (~7% per leg on this grid, see
        // `uniform_point_source_matches_euclidean`), and the corner adds a
        // little more: the solver gives ~24.7 against an exact 22.6.
        assert!(
            (got - detour).abs() / detour < 0.12,
            "expected about {detour:.3}, got {got:.3}"
        );
    }

    /// A source that lies on an obstacle cell must not emit a wave.
    #[test]
    fn source_on_obstacle_is_ignored() {
        let (w, h) = (10usize, 1usize);
        let mut speed = vec![1.0f64; w * h];
        speed[5] = 0.0;
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[5], w, h, 1.0);
        assert!(
            u.iter().all(|t| t.is_infinite()),
            "obstacle source emitted: {u:?}"
        );
    }

    /// Without sources nothing is reachable, and the solver still terminates.
    #[test]
    fn no_sources_leaves_everything_unreachable() {
        let (w, h) = (8usize, 6usize);
        let speed = vec![1.0f64; w * h];
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[], w, h, 1.0);
        assert!(u.iter().all(|t| t.is_infinite()));
    }

    /// In a 1D corridor each cell adds exactly h / f, so a faster half of the
    /// corridor is crossed in half the time.
    #[test]
    fn varying_speed_accumulates_cell_costs() {
        let (w, h) = (10usize, 1usize);
        let speed: Vec<f64> = (0..w).map(|j| if j < 5 { 1.0 } else { 2.0 }).collect();
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[0], w, h, 1.0);
        // 4 cells at cost 1 to reach cell 4, then 5 cells at cost 0.5.
        assert!((u[4] - 4.0).abs() < 1e-12, "got {}", u[4]);
        assert!((u[9] - 6.5).abs() < 1e-12, "got {}", u[9]);
    }

    /// Angle in degrees between the floor-field gradient at `(row, col)` and
    /// the exact direction `(dx, dy)` away from where the wave came from.
    fn gradient_angle_error(
        grid: &GridParams,
        u: &[f64],
        row: usize,
        col: usize,
        dx: f64,
        dy: f64,
    ) -> f64 {
        let (gx, gy) = sobel_gradient(grid, row as i32, col as i32, u);
        let cos = (gx * dx + gy * dy) / ((gx * gx + gy * gy).sqrt() * (dx * dx + dy * dy).sqrt());
        cos.clamp(-1.0, 1.0).acos().to_degrees()
    }

    fn unit_grid(w: usize, h: usize) -> GridParams {
        GridParams {
            origin: [0.0, 0.0],
            width: w as u32,
            height: h as u32,
            cell_size: 1.0,
        }
    }

    /// Agents walk along the gradient, so its direction is what the
    /// first-order error costs them. Around a point source in open space the
    /// gradient must point radially away from the source.
    #[test]
    fn gradient_direction_open_space() {
        let (w, h) = (101usize, 101usize);
        let (cr, cc) = (h / 2, w / 2);
        let speed = vec![1.0f64; w * h];
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[(cr * w + cc) as u32], w, h, 1.0);
        let grid = unit_grid(w, h);

        let (mut max, mut sum, mut n) = (0.0f64, 0.0, 0);
        for row in 1..h - 1 {
            for col in 1..w - 1 {
                let (dx, dy) = (col as f64 - cc as f64, row as f64 - cr as f64);
                let r = (dx * dx + dy * dy).sqrt();
                if !(5.0..=45.0).contains(&r) {
                    continue; // skip the source's start-up zone and the border
                }
                let e = gradient_angle_error(&grid, &u, row, col, dx, dy);
                max = max.max(e);
                sum += e;
                n += 1;
            }
        }
        let mean = sum / n as f64;
        // Measured with the 4-neighbour stencil: max 11.2 deg, mean 2.6 deg.
        // The error decays slowly with distance: max ~11 deg within 10 cells,
        // ~8 deg at 10-20, ~3 deg beyond 40. The bounds sit just above.
        assert!(max < 12.0, "max gradient angle error {max:.2} deg");
        assert!(mean < 3.0, "mean gradient angle error {mean:.2} deg");
    }

    /// Behind a wall corner the wave restarts at the corner, so the gradient
    /// must point away from the corner, not from the original source.
    #[test]
    fn gradient_direction_behind_wall_corner() {
        let (w, h) = (61usize, 61usize);
        let mut speed = vec![1.0f64; w * h];
        for row in 0..40 {
            speed[row * w + 30] = 0.0; // wall cells x in [30, 31], y in [0, 40]
        }
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[(20 * w + 10) as u32], w, h, 1.0);
        let grid = unit_grid(w, h);

        // The shortest path to a shadow cell bends at the wall's top-right
        // corner, so the exact direction points away from it. Cell centres
        // sit at +0.5.
        let corner = (31.0, 40.0);
        let (mut max, mut sum, mut n) = (0.0f64, 0.0, 0);
        for row in 2..=35 {
            for col in 35..w - 2 {
                let (dx, dy) = (col as f64 + 0.5 - corner.0, row as f64 + 0.5 - corner.1);
                if (dx * dx + dy * dy).sqrt() < 10.0 {
                    continue; // the solver's corner is only known to half a cell
                }
                let e = gradient_angle_error(&grid, &u, row, col, dx, dy);
                max = max.max(e);
                sum += e;
                n += 1;
            }
        }
        let mean = sum / n as f64;
        // Measured with the 4-neighbour stencil: max 8.0 deg, mean 2.2 deg.
        assert!(max < 9.0, "max gradient angle error {max:.2} deg");
        assert!(mean < 2.5, "mean gradient angle error {mean:.2} deg");
    }

    /// Cells with speed 0 are obstacles and stay unreachable.
    #[test]
    fn obstacle_cells_stay_infinite() {
        let (w, h) = (10usize, 1usize);
        let mut speed = vec![1.0f64; w * h];
        speed[5] = 0.0;
        let mut u = vec![0.0; w * h];
        solve_into(&mut u, &speed, &[0], w, h, 1.0);
        assert!(u[5].is_infinite());
        assert!(u[9].is_infinite(), "wall must block the 1D corridor");
    }
}
