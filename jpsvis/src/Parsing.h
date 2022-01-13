/**
 * @file    SaxParser.h
 * @author  Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Created on: 6 Sep 2010
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */
#pragma once

#include "TrajectoryData.h"
#include "geometry/GeometryFactory.h"
#include "geometry/Point.h"
#include "tinyxml.h"
#include "trains/train.h"

#include <QString>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <tuple>

namespace Parsing
{
/// This enum describes all file types that can be selected via the OpenFile dialog.
enum class InputFileType {
    /// This is geometry data in XML format, this is the same format jpscore consumes
    GEOMETRY_XML,
    /// This is trajectory data in TXT format created by jpscore
    TRAJECTORIES_TXT,
    /// This is dummy type indicating that the file format is not recognised.
    UNRECOGNIZED
};

/// Encapsulates paths to all additional inputs defined in a trajectory txt file.
struct AdditionalInputs {
    /// Path to the geometry file
    std::optional<std::filesystem::path> geometry_path;
    /// Path to the time table file
    std::optional<std::filesystem::path> train_time_table_path;
    /// Path to the train type file
    std::optional<std::filesystem::path> train_type_path;
};

/// This function will return the detected file type, this may be 'UNRECOGNISED' if detection fails.
/// @param path to the file
/// @return InputFileType that was detected
InputFileType detectFileType(const std::filesystem::path & path);

/// provided for convenience and will be removed in the next version
bool readJpsGeometryXml(const std::filesystem::path & path, GeometryFactory & geo);

/// parse the txt file format
bool ParseTxtFormat(const QString & fileName, TrajectoryData * trajectories);

/// Trains
bool LoadTrainTimetable(
    std::string filename,
    std::map<int, std::shared_ptr<TrainTimeTable>> & trainTimeTables);

std::shared_ptr<TrainTimeTable> parseTrainTimeTableNode(TiXmlElement * e);

std::shared_ptr<TrainType> parseTrainTypeNode(TiXmlElement * e);

bool LoadTrainType(
    std::string Filename,
    std::map<std::string, std::shared_ptr<TrainType>> & trainTypes);

std::tuple<Point, Point> GetTrackStartEnd(QString geometryFile, int trackId);

/// Extract additional input files from the trajectory txt file.
/// All paths returned are absolute paths. If no entry has been found for a particular additional
/// input the optional will be set to NONE.
/// This functions does not do any constriant checking, i.e. all found paths are returned as is.
/// @param path to the trajectory txt file, this file is expected to exist.
/// @return AdditionalInputs that have been found.
AdditionalInputs extractAdditionalInputFilePaths(const std::filesystem::path & path);
}; // namespace Parsing
