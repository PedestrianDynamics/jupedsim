// SPDX-License-Identifier: LGPL-3.0-or-later
#include "Geometry/Validation.hpp"

#include <CGAL/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/detect_features.h>
#include <CGAL/Polygon_mesh_processing/measure.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

namespace PMP = CGAL::Polygon_mesh_processing;

namespace
{
// Faces are validated planar, so a real crease stands out well above numerical noise.
constexpr double sharp_edge_angle_deg = 5.0;

struct Options {
    std::filesystem::path input{};
    double max_edge_length = 0.3;
    unsigned int iterations = 3;
};

void PrintUsage()
{
    std::cerr << "Usage: remesh [--side-length=<max>] [--iterations=<n>] <input.obj>\n"
              << "  --side-length=<max>  maximum triangle side length, > 0 (default: 0.3)\n"
              << "  --iterations=<n>     isotropic remeshing iterations, >= 1 (default: 3)\n";
}

template <typename T>
std::optional<T> ParseValue(std::string_view text)
{
    T value{};
    const auto [ptr, ec] = std::from_chars(text.begin(), text.end(), value);
    if(ec != std::errc{} || ptr != text.end()) {
        return std::nullopt;
    }
    return value;
}

std::optional<Options> ParseArgs(int argc, char** argv)
{
    Options opts{};
    bool have_input = false;
    for(int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        if(constexpr std::string_view prefix = "--side-length="; arg.starts_with(prefix)) {
            const auto value = ParseValue<double>(arg.substr(prefix.size()));
            if(!value || *value <= 0.0) {
                std::cerr << "Invalid --side-length value: '" << arg << "'\n";
                return std::nullopt;
            }
            opts.max_edge_length = *value;
        } else if(constexpr std::string_view prefix = "--iterations="; arg.starts_with(prefix)) {
            const auto value = ParseValue<unsigned int>(arg.substr(prefix.size()));
            if(!value || *value < 1) {
                std::cerr << "Invalid --iterations value: '" << arg << "'\n";
                return std::nullopt;
            }
            opts.iterations = *value;
        } else if(arg.starts_with("--")) {
            std::cerr << "Unknown option: '" << arg << "'\n";
            return std::nullopt;
        } else if(!have_input) {
            opts.input = std::filesystem::path{arg};
            have_input = true;
        } else {
            std::cerr << "Unexpected extra argument: '" << arg << "'\n";
            return std::nullopt;
        }
    }
    if(!have_input) {
        return std::nullopt;
    }
    return opts;
}

/// Refine #mesh so that no edge is longer than #max_edge_length, keeping borders and
/// creases (stair noses, floor/ramp transitions) in place.
void RemeshWithMaxEdgeLength(SurfaceMesh& mesh, double max_edge_length, unsigned int iterations)
{
    // isotropic_remeshing only splits edges longer than 4/3 x target, so aim below the cap.
    const double target_edge_length = max_edge_length * 3. / 4.;

    auto ecm =
        mesh.add_property_map<EdgeDescriptor<SurfaceMesh>, bool>("e:constrained", false).first;
    PMP::detect_sharp_edges(mesh, sharp_edge_angle_deg, ecm);

    std::vector<EdgeDescriptor<SurfaceMesh>> constrained{};
    for(auto e : CGAL::edges(mesh)) {
        if(get(ecm, e)) {
            constrained.emplace_back(e);
        }
    }
    // protect_constraints keeps these edges out of the remesher's reach, so cap their length
    // here; this also satisfies the remesher's < 4/3 x target precondition on them.
    PMP::split_long_edges(
        constrained, target_edge_length, mesh, CGAL::parameters::edge_is_constrained_map(ecm));

    PMP::isotropic_remeshing(
        CGAL::faces(mesh),
        target_edge_length,
        mesh,
        CGAL::parameters::edge_is_constrained_map(ecm)
            .protect_constraints(true)
            .number_of_iterations(iterations));

    // The remesher's smoothing step can stretch edges slightly past 4/3 x target, so the
    // target alone does not bound edge lengths; a final split pass makes the cap strict.
    std::vector<EdgeDescriptor<SurfaceMesh>> all_edges{};
    all_edges.reserve(mesh.number_of_edges());
    for(auto e : CGAL::edges(mesh)) {
        all_edges.emplace_back(e);
    }
    PMP::split_long_edges(
        all_edges, max_edge_length, mesh, CGAL::parameters::edge_is_constrained_map(ecm));

    mesh.remove_property_map(ecm);
}

double MaxEdgeLength(const SurfaceMesh& mesh)
{
    double max_length = 0.0;
    for(auto e : CGAL::edges(mesh)) {
        max_length = std::max(max_length, CGAL::to_double(PMP::edge_length(e, mesh)));
    }
    return max_length;
}
} // namespace

int main(int argc, char** argv)
{
    const auto opts = ParseArgs(argc, argv);
    if(!opts) {
        PrintUsage();
        return EXIT_FAILURE;
    }

    SurfaceMesh mesh{};
    if(!PMP::IO::read_polygon_mesh(opts->input.string(), mesh) || mesh.is_empty()) {
        std::cerr << "Could not read a mesh from OBJ file '" << opts->input.string() << "'\n";
        return EXIT_FAILURE;
    }

    try {
        NormaliseAndValidateMesh(mesh);
    } catch(const std::exception& e) {
        std::cerr << "Mesh validation failed: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    const auto faces_before = mesh.number_of_faces();
    RemeshWithMaxEdgeLength(mesh, opts->max_edge_length, opts->iterations);
    mesh.collect_garbage();

    const auto output =
        std::filesystem::current_path() / (opts->input.stem().string() + "_remeshed.obj");
    if(!CGAL::IO::write_polygon_mesh(
           output.string(), mesh, CGAL::parameters::stream_precision(17))) {
        std::cerr << "Could not write mesh to '" << output.string() << "'\n";
        return EXIT_FAILURE;
    }

    std::cout << "Wrote " << output.string() << ": " << faces_before << " -> "
              << mesh.number_of_faces() << " faces, max edge length " << MaxEdgeLength(mesh)
              << '\n';
    return EXIT_SUCCESS;
}
