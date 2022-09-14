/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ResultHandling.hpp"

#include <fmt/format.h>
#include <stdexcept>

namespace fs = std::filesystem;
using detail::extractPath;
using detail::patchPath;

static void patchIniFileAfterCopy(TiXmlDocument& doc)
{
    patchPath(doc, {"JuPedSim", "geometry"});
    patchPath(doc, {"JuPedSim", "header", "geometry"});
    patchPath(doc, {"JuPedSim", "routing", "goals", "file"});
    patchPath(doc, {"JuPedSim", "agents", "agents_sources", "file"});
    patchPath(doc, {"JuPedSim", "events_file"});
    patchPath(doc, {"JuPedSim", "header", "events_file"});
    patchPath(doc, {"JuPedSim", "schedule_file"});
    patchPath(doc, {"JuPedSim", "header", "schedule_file"});
    patchPath(doc, {"JuPedSim", "train_constraints", "train_time_table"});
    patchPath(doc, {"JuPedSim", "train_constraints", "train_types"});
}

static void patchGeometryFileAfterCopy(TiXmlDocument& doc)
{
    patchPath(doc, {"geometry", "transitions", "file"});
}

void collectInputFilesIn(const std::filesystem::path& iniFile, const std::filesystem::path& outPath)
{
    fs::create_directories(outPath);
    fs::copy(iniFile, outPath, fs::copy_options::overwrite_existing);

    const auto copiedIniFile = outPath / iniFile.filename();
    TiXmlDocument iniFileXmlDoc(copiedIniFile.string());
    if(!iniFileXmlDoc.LoadFile()) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("XML parser, cannot open {:s}, error: {:s}"),
            copiedIniFile.string(),
            iniFileXmlDoc.ErrorDesc()));
    }

    auto geometryFile = extractPath(iniFileXmlDoc, {"JuPedSim", "geometry"});
    if(!geometryFile) {
        geometryFile = extractPath(iniFileXmlDoc, {"JuPedSim", "header", "geometry"});
        if(!geometryFile) {
            throw std::runtime_error("XML parser, cannot find geometry file path in iniFile");
        }
    }
    fs::copy(*geometryFile, outPath, fs::copy_options::overwrite_existing);
    const auto copiedGeometryFile = outPath / geometryFile->filename();
    TiXmlDocument geometryFileXmlDoc(copiedGeometryFile.string());
    if(!geometryFileXmlDoc.LoadFile()) {
        throw std::runtime_error(fmt::format(
            FMT_STRING("XML parser, cannot open {:s}, error: {:s}"),
            copiedGeometryFile.string(),
            geometryFileXmlDoc.ErrorDesc()));
    }

    if(const auto file = extractPath(iniFileXmlDoc, {"JuPedSim", "routing", "goals", "file"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    }

    if(const auto file =
           extractPath(iniFileXmlDoc, {"JuPedSim", "agents", "agents_sources", "file"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    }

    if(const auto file = extractPath(iniFileXmlDoc, {"JuPedSim", "events_file"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    } else if(const auto file = extractPath(iniFileXmlDoc, {"JuPedSim", "header", "events_file"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    }

    if(const auto file = extractPath(iniFileXmlDoc, {"JuPedSim", "schedule_file"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    } else if(
        const auto file = extractPath(iniFileXmlDoc, {"JuPedSim", "header", "schedule_file"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    }

    if(const auto file =
           extractPath(iniFileXmlDoc, {"JuPedSim", "train_constraints", "train_time_table"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    }

    if(const auto file =
           extractPath(iniFileXmlDoc, {"JuPedSim", "train_constraints", "train_types"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    }
    patchIniFileAfterCopy(iniFileXmlDoc);

    if(const auto file = extractPath(geometryFileXmlDoc, {"geometry", "transitions", "file"})) {
        fs::copy(*file, outPath, fs::copy_options::overwrite_existing);
    }
    patchGeometryFileAfterCopy(geometryFileXmlDoc);
}

namespace detail
{
void patchPath(TiXmlDocument& doc, const std::vector<std::string>& xmlPathToPatch)
{
    TiXmlNode* node = &doc;
    for(const auto& p : xmlPathToPatch) {
        node = node->FirstChild(p);
        if(node == nullptr) {
            return;
        }
    }
    auto* text = node->FirstChild();
    if(text == nullptr) {
        return;
    }
    const fs::path p{text->Value()};
    text->SetValue(p.filename().string().c_str());
}

std::optional<fs::path>
extractPath(const TiXmlDocument& doc, const std::vector<std::string>& xmlPath)
{
    const TiXmlNode* node = &doc;
    for(const auto& p : xmlPath) {
        node = node->FirstChild(p);
        if(node == nullptr) {
            return std::nullopt;
        }
    }
    auto* text = node->FirstChild();
    if(text == nullptr) {
        return std::nullopt;
    }
    return {text->Value()};
}
} // namespace detail
