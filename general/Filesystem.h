#pragma once

#ifdef __APPLE__
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

/**
 * Add a prefix to a filename.
 *
 * @param prefix to add
 * @param file to prefix.
 * @return path wih new prefix for the last element
 */
fs::path add_prefix_to_filename(
    const std::string& prefix, const fs::path& file);
