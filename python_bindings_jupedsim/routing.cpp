// SPDX-License-Identifier: LGPL-3.0-or-later
#include "conversion.hpp"
#include "floorfield_routing_engine.hpp"
#include "geometry/geometry.hpp"
#include "geometry/location.hpp"
#include "geometry/validation.hpp"
#include "simulation_error.hpp"
#include "surface_mesh_shortest_path_routing_engine.hpp"
#include "type_casters.hpp"

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <fmt/format.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace py = pybind11;

namespace
{
/// Checks that the arrays of @p grid hold one entry per cell: the NumPy copies of its
/// values and directions rely on it.
void check_cell_count(const TravelTimeGrid& grid)
{
    const auto cells = std::size_t{grid.width} * grid.height;
    if(grid.values.size() != cells || grid.directions.size() != cells) {
        throw std::logic_error(
            fmt::format(
                "TravelTimeGrid of {}x{} cells holds {} values and {} directions",
                grid.width,
                grid.height,
                grid.values.size(),
                grid.directions.size()));
    }
}

/// The placement properties every floor field grid has: origin, size and extent.
template <typename Grid>
void def_placement(py::class_<Grid>& grid)
{
    grid.def_property_readonly(
            "origin",
            [](const Grid& g) { return std::tuple{g.origin.x, g.origin.y}; },
            "Lower-left corner of cell (0, 0) in metres.")
        .def_readonly("width", &Grid::width, "Number of cells along x.")
        .def_readonly("height", &Grid::height, "Number of cells along y.")
        .def_readonly("cell_size", &Grid::cell_size, "Edge length of a cell in metres.")
        .def_property_readonly(
            "extent",
            [](const Grid& g) {
                return std::tuple{
                    g.origin.x,
                    g.origin.x + g.width * g.cell_size,
                    g.origin.y,
                    g.origin.y + g.height * g.cell_size};
            },
            "``(x_min, x_max, y_min, y_max)`` of the grid in metres, as matplotlib's "
            "``extent`` expects it.")
        .def("__repr__", [](const Grid& g) {
            return py::str("{}(origin=({}, {}), width={}, height={}, cell_size={})")
                .format(
                    py::type::of<Grid>().attr("__name__"),
                    g.origin.x,
                    g.origin.y,
                    g.width,
                    g.height,
                    g.cell_size);
        });
}
} // namespace

void init_routing(py::module_& m)
{
    const FloorfieldOptions floorfield_defaults{};
    py::class_<FloorfieldOptions> floorfield_options(m, "FloorfieldOptions");
    floorfield_options.doc() = clean_doc(R"(
        Parameters of the static floor fields used to route agents to their stage areas.

        Args:
            cell_size: Edge length of a floor field grid cell [m]. Smaller cells give
                smoother routes at the cost of memory and setup time.
            wall_influence_radius: Distance from walls at which the speed field is reduced to avoid 
            routing agents to close to walls. The reduction scales linearly from the influence radius to the wall.
    )");
    floorfield_options
        .def(
            py::init([](double cell_size, double wall_influence_radius) {
                return FloorfieldOptions{cell_size, wall_influence_radius};
            }),
            py::kw_only(),
            py::arg("cell_size") = floorfield_defaults.cell_size,
            py::arg("wall_influence_radius") = floorfield_defaults.wall_influence_radius)
        .def_readwrite("cell_size", &FloorfieldOptions::cell_size)
        .def_readwrite("wall_influence_radius", &FloorfieldOptions::wall_influence_radius)
        .def("__repr__", [](const FloorfieldOptions& self) {
            return py::str("FloorfieldOptions(cell_size={!r}, wall_influence_radius={!r})")
                .format(self.cell_size, self.wall_influence_radius);
        });

    py::class_<TravelTimeGrid> travel_time_grid(m, "TravelTimeGrid");
    travel_time_grid.doc() = clean_doc(R"(
        Travel times and walking directions to one stage over the floor field grid of one
        region.

        Get one from :func:`~jupedsim.Simulation.travel_times`. All regions share one
        lattice, so the grids of neighbouring regions line up cell for cell. Plot one
        with matplotlib:

        .. code:: python

            grid = sim.travel_times(exit_id)[room]
            plt.imshow(
                numpy.ma.masked_invalid(grid.values),
                origin="lower",
                extent=grid.extent,
            )
    )");
    def_placement(travel_time_grid);
    travel_time_grid
        .def_property_readonly(
            "values",
            [](const TravelTimeGrid& g) {
                check_cell_count(g);
                py::array_t<double> values(
                    {static_cast<py::ssize_t>(g.height), static_cast<py::ssize_t>(g.width)});
                std::copy(g.values.begin(), g.values.end(), values.mutable_data());
                return values;
            },
            "Travel times in seconds as an array of shape ``(height, width)``; row 0 is the "
            "lowest. Zero inside the stage's area, ``inf`` where a cell is not walkable. "
            "Each access returns a new copy.")
        .def_property_readonly(
            "directions",
            [](const TravelTimeGrid& g) {
                check_cell_count(g);
                py::array_t<double> directions(
                    {static_cast<py::ssize_t>(g.height),
                     static_cast<py::ssize_t>(g.width),
                     py::ssize_t{2}});
                auto* out = directions.mutable_data();
                for(const auto& d : g.directions) {
                    *out++ = d.x;
                    *out++ = d.y;
                }
                return directions;
            },
            "Walking directions as unit vectors ``(x, y)`` in an array of shape "
            "``(height, width, 2)``, laid out like :attr:`values`: the direction an agent at "
            "a cell's centre is sent along. Zero inside the stage's area, ``nan`` where a "
            "cell is not walkable. Each access returns a new copy.");

    py::class_<SpeedGrid> speed_grid(m, "SpeedGrid");
    speed_grid.doc() = clean_doc(R"(
        Walking speeds over the floor field grid of one region, the same grid as
        :class:`~jupedsim.TravelTimeGrid`'s.

        Get one from :func:`~jupedsim.Simulation.speed_fields`, change its values and
        hand them back with :func:`~jupedsim.Simulation.set_speed_field`, for example to
        slow agents down in smoke:

        .. code:: python

            grid = sim.speed_fields()[room]
            x, y = grid.cell_centres
            sim.set_speed_field(room, grid.values * visibility(x, y))
    )");
    def_placement(speed_grid);
    speed_grid
        .def_property_readonly(
            "values",
            [](const SpeedGrid& g) {
                const auto cells = std::size_t{g.width} * g.height;
                if(g.values.size() != cells) {
                    throw std::logic_error(
                        fmt::format(
                            "SpeedGrid of {}x{} cells holds {} values",
                            g.width,
                            g.height,
                            g.values.size()));
                }
                py::array_t<double> values(
                    {static_cast<py::ssize_t>(g.height), static_cast<py::ssize_t>(g.width)});
                std::copy(g.values.begin(), g.values.end(), values.mutable_data());
                return values;
            },
            "Speeds in m/s as an array of shape ``(height, width)``; row 0 is the lowest. "
            "Positive where a cell is walkable, 0 elsewhere. Each access returns a new copy.")
        .def_property_readonly(
            "cell_centres",
            [](const SpeedGrid& g) {
                const std::vector<py::ssize_t> shape{
                    static_cast<py::ssize_t>(g.height), static_cast<py::ssize_t>(g.width)};
                py::array_t<double> x(shape);
                py::array_t<double> y(shape);
                auto xs = x.mutable_unchecked<2>();
                auto ys = y.mutable_unchecked<2>();
                for(py::ssize_t row = 0; row < shape[0]; ++row) {
                    for(py::ssize_t col = 0; col < shape[1]; ++col) {
                        xs(row, col) = g.origin.x + (static_cast<double>(col) + 0.5) * g.cell_size;
                        ys(row, col) = g.origin.y + (static_cast<double>(row) + 0.5) * g.cell_size;
                    }
                }
                return std::tuple{x, y};
            },
            "``(x, y)``: the coordinates of every cell's centre in metres, two arrays laid "
            "out like :attr:`values`.");

    py::class_<Location> location(m, "Location");
    location.doc() = clean_doc(R"(
        A point on the walkable surface, together with the region it lies in.

        Locations cannot be created directly, get them from the simulation:

        .. code:: python

            sim.get_location(x, y, region_id=upper_floor)
            sim.agent(agent_id).location

        A location is read-only and stays valid as long as its simulation exists.
        It does not move with an agent: read
        :attr:`~jupedsim.Agent.location` again to get the agent's current
        location.
    )");
    location
        .def_property_readonly(
            "x",
            [](const Location& l) { return l.xy().x; },
            clean_doc("x coordinate in metres.").c_str())
        .def_property_readonly(
            "y",
            [](const Location& l) { return l.xy().y; },
            clean_doc("y coordinate in metres.").c_str())
        .def_property_readonly(
            "z", &Location::z, clean_doc("Height of the surface here, in metres.").c_str())
        .def_property_readonly(
            "region_id", &Location::region, clean_doc("Region this location lies in.").c_str())
        .def("__repr__", [](const Location& l) {
            // Python float formatting (3.0, not fmt's 3), as the former Python wrapper printed.
            return py::str("Location({!r}, {!r}, {!r})").format(l.xy().x, l.xy().y, l.z());
        });

    py::class_<SurfaceMeshShortestPathRoutingEngine>(m, "SurfaceMeshShortestPathRoutingEngine")
        // The engine borrows the geometry; keep_alive ties the Python-side
        // Geometry's lifetime to the engine so the borrow can't dangle.
        .def(
            py::init([](const Geometry& geometry) {
                return std::make_unique<SurfaceMeshShortestPathRoutingEngine>(geometry);
            }),
            py::arg("geometry"),
            py::keep_alive<1, 2>())
        .def("is_valid_location", &SurfaceMeshShortestPathRoutingEngine::is_valid_location)
        .def("get_shortest_path", &SurfaceMeshShortestPathRoutingEngine::get_shortest_path)
        .def("get_orientation", &SurfaceMeshShortestPathRoutingEngine::get_orientation)
        .def("wall_clearance", &SurfaceMeshShortestPathRoutingEngine::wall_clearance);
}
