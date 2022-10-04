/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <filesystem>
#include <optional>
#include <tinyxml.h>
#include <vector>

/// Copies required files (inifile / geometry) and optional files to 'outPath'.
/// Additionally all references to the files get patched so that / the copied
/// inifile points to the copied files in 'outPath'.
/// E.g. if inifile pointed to 'foo/bar/geometry.xml' the copied ini file will
/// just point to 'geometry.xml'
void collectInputFilesIn(
    const std::filesystem::path& iniFile,
    const std::filesystem::path& outPath);

// Do not use any function from this namespace directly,
// this is only exposed to allow unit tests to be written.
namespace detail
{
/// Only public for testing.
/// Expects a filepath to be present in the xml document at the provided
/// path and will only leave the filename in place.
/// E.g.:
/// call `patchPath(doc, {"a", "b", "c", "file"}) to transform
///
/// ```
/// <a>
///   <b>
///     <c>
///       <file>foo/bar.txt</file>
///     </c>
///   </b>
/// </a>
/// ```
/// to this
///
/// ```
/// <a>
///   <b>
///     <c>
///       <file>bar.txt</file>
///     </c>
///   </b>
/// </a>
/// ```
/// @param doc XmlDocument to work on.
/// @param xmlPathToPatch vector of strings representing the paath to the node to modify
void patchPath(TiXmlDocument& doc, const std::vector<std::string>& xmlPathToPatch);

/// Only public for testing.
/// Creates a 'Path' object from the test found at the specified xml path. It
/// the path cannot be found or the text node is null a 'nullopt' will be
/// returned.
/// E.g.: <a><b>file.txt</a></b> can be querried with extractPath(doc, {"a", "b"})
/// @param doc TiXmlDocument to search in
/// @param xmlPath to search
std::optional<std::filesystem::path>
extractPath(const TiXmlDocument& doc, const std::vector<std::string>& xmlPath);
} // namespace detail
