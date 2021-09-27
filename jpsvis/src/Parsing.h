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

#include "SyncData.h"
#include "geometry/GeometryFactory.h"
#include "geometry/Point.h"
#include "tinyxml.h"
#include "trains/train.h"

#include <QDomNode>
#include <QString>
#include <map>
#include <memory>

namespace Parsing
{
/// provided for convenience and will be removed in the next version
bool parseGeometryJPS(QString content, GeometryFactory & geo);

/// provided for convenience and will be removed in the next version
void parseGeometryXMLV04(QString content, GeometryFactory & geo);

/// provided for convenience and will be removed in the next version
void parseGeometryTRAV(QString fileName, GeometryFactory & geoFac, QDomNode geoNode = QDomNode());

/// take a large file and find the geometry file location.
QString extractGeometryFilename(QString & filename);

QString extractGeometryFilenameTXT(QString & filename);

QString extractSourceFileTXT(QString & filename);

QString extractGoalFileTXT(QString & filename);

/// parse the txt file format
bool ParseTxtFormat(const QString & fileName, SyncData * dataset, double * fps);

/// Trains
bool LoadTrainTimetable(
    std::string filename,
    std::map<int, std::shared_ptr<TrainTimeTable>> & trainTimeTables);

std::shared_ptr<TrainTimeTable> parseTrainTimeTableNode(TiXmlElement * e);

std::shared_ptr<TrainType> parseTrainTypeNode(TiXmlElement * e);

QString extractTrainTypeFileTXT(QString & filename);

QString extractTrainTimeTableFileTXT(QString & filename);

bool LoadTrainType(
    std::string Filename,
    std::map<std::string, std::shared_ptr<TrainType>> & trainTypes);

double GetElevation(QString geometryFile, int roomId, int subroomId);

std::tuple<Point, Point> GetTrackStartEnd(QString geometryFile, int trackId);
}; // namespace Parsing
