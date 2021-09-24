/**
 * @file SystemSettings.cpp
 * @author   Ulrich Kemloh <kemlohulrich@gmail.com>
 * @version 0.1
 * Copyright (C) <2009-2010>
 *
 * @section LICENSE
 * This file is part of OpenPedSim.
 *
 * OpenPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 * \brief contains travisto global settings
 *
 *
 *
 *  Created on: 14.09.2009
 *
 */


#include "SystemSettings.h"

#include "../forms/Settings.h"
#include "IO/OutputHandler.h"
#include "Log.h"
#include "QDateTime"

#include <QColor>
#include <QDir>

bool SystemSettings::showLegend                  = false;
bool SystemSettings::pedColorProfileReadFromFile = true;
bool SystemSettings::showAgentsCaptions          = false;
bool SystemSettings::showDirections              = false;
bool SystemSettings::is2D                        = false;
bool SystemSettings::showAgents                  = true;
bool SystemSettings::showGeometry                = true;
bool SystemSettings::showGeometryCaptions        = true;
bool SystemSettings::showFloor                   = true;
bool SystemSettings::showWalls                   = true;
bool SystemSettings::showExits                   = true;
bool SystemSettings::showNavLines                = true;
bool SystemSettings::showTrajectories            = false;
bool SystemSettings::showObstacle                = true;
bool SystemSettings::showGradienField            = true;


unsigned short SystemSettings::port = 8989;
// double SystemSettings::bgColor[]= {1.0,1.0,1.0};
QColor SystemSettings::bgColor       = QColor(Qt::white);
QColor SystemSettings::floorColor    = QColor(0, 0, 255);
QColor SystemSettings::wallsColor    = QColor(180, 180, 180); // 180.0/255,180.0/255.0,180.0/255.0
QColor SystemSettings::obstacleColor = QColor(180, 180, 180); // 180.0/255,180.0/255.0,180.0/255.0
QColor SystemSettings::exitsColor    = QColor(175, 175, 255); // 175.0/255,175.0/255.0,255.0/255.0
QColor SystemSettings::navLinesColor = QColor(Qt::white);

int SystemSettings::pedestriansColor[3][3] = {{255, 17, 224}, {122, 255, 122}, {130, 130, 130}};
int SystemSettings::pedesShape             = Settings::PINGUINS;
int SystemSettings::ellipseResolution      = 10;
int SystemSettings::trailCount             = 50;
int SystemSettings::trailingType           = 1;
int SystemSettings::trailingGeometry       = 1;
int SystemSettings::virtualAgentID         = -1;

int SystemSettings::captionSize               = 30;
QColor SystemSettings::captionColor           = QColor();
int SystemSettings::captionOrientaton         = 1;
bool SystemSettings::captionAutomaticRotation = 1;
bool SystemSettings::onScreenInfos            = true;

bool SystemSettings::recordPNGsequence = false;
// QString SystemSettings::outputDir =QDir::currentPath()+"/";
QString SystemSettings::outputDir = QDir::homePath() + QDir::separator() + "Desktop" +
                                    QDir::separator() + "JPSvis_Files" + QDir::separator();
QString SystemSettings::workingDir  = QDir::currentPath();
QString SystemSettings::filesPrefix = "";
QString SystemSettings::logfile     = "log.txt";
// the log file is writting by a different module
OutputHandler * Log = NULL;


SystemSettings::SystemSettings() {}

SystemSettings::~SystemSettings() {}

void SystemSettings::setShowLegend(bool legend)
{
    showLegend = legend;
}

bool SystemSettings::getShowLegend()
{
    return showLegend;
}


void SystemSettings::setListeningPort(unsigned short porta)
{
    port = porta;
}

unsigned short SystemSettings::getListeningPort()
{
    return port;
}


void SystemSettings::setShowAgentsCaptions(bool caption)
{
    showAgentsCaptions = caption;
}

bool SystemSettings::getShowAgentsCaptions()
{
    return showAgentsCaptions;
}

void SystemSettings::setShowDirections(bool direction)
{
    showDirections = direction;
}

bool SystemSettings::getShowDirections()
{
    return showDirections;
}

void SystemSettings::set2D(bool lis2D)
{
    is2D = lis2D;
}
bool SystemSettings::get2D()
{
    return is2D;
}

// set/get the  2D visio
void SystemSettings::setShowAgents(bool status)
{
    showAgents = status;
}

bool SystemSettings::getShowAgents()
{
    return showAgents;
}

void SystemSettings::setShowGeometry(bool status)
{
    showGeometry = status;
}

bool SystemSettings::getShowGeometry()
{
    return showGeometry;
}

void SystemSettings::setShowGeometryCaptions(bool status)
{
    showGeometryCaptions = status;
}

bool SystemSettings::getShowGeometryCaptions()
{
    return showGeometryCaptions;
}

void SystemSettings::setShowFloor(bool status)
{
    showFloor = status;
}

bool SystemSettings::getShowFloor()
{
    return showFloor;
}

void SystemSettings::setShowExits(bool status)
{
    showExits = status;
}

bool SystemSettings::getShowExits()
{
    return showExits;
}

void SystemSettings::setShowWalls(bool status)
{
    showWalls = status;
}

bool SystemSettings::getShowWalls()
{
    return showWalls;
}

void SystemSettings::setShowObstacles(bool status)
{
    showObstacle = status;
}
void SystemSettings::setShowGradientField(bool status)
{
    showGradienField = status;
}
bool SystemSettings::getShowGradientField()
{
    return showGradienField;
}
bool SystemSettings::getShowObstacles()
{
    return showObstacle;
}

void SystemSettings::setShowNavLines(bool status)
{
    showNavLines = status;
}

bool SystemSettings::getShowNavLines()
{
    return showNavLines;
}

void SystemSettings::setShowTrajectories(bool status)
{
    showTrajectories = status;
}

bool SystemSettings::getShowTrajectories()
{
    return showTrajectories;
}

void SystemSettings::setWorkingDirectory(const QString & dir)
{
    workingDir = dir;
}

void SystemSettings::getWorkingDirectory(QString & dir)
{
    dir = workingDir;
}

const QColor & SystemSettings::getBackgroundColor()
{
    return bgColor;
}

void SystemSettings::setBackgroundColor(const QColor & col)
{
    bgColor = col;
}

const QColor & SystemSettings::getFloorColor()
{
    return floorColor;
}

void SystemSettings::setFloorColor(const QColor & col)
{
    floorColor = col;
}

const QColor & SystemSettings::getWallsColor()
{
    return wallsColor;
}

void SystemSettings::setWallsColor(const QColor & col)
{
    wallsColor = col;
}

const QColor & SystemSettings::getExitsColor()
{
    return exitsColor;
}

void SystemSettings::setExitsColor(const QColor & col)
{
    exitsColor = col;
}

const QColor & SystemSettings::getNavLinesColor()
{
    return navLinesColor;
}

const QColor & SystemSettings::getObstacleColor()
{
    return obstacleColor;
}

void SystemSettings::setObstacleColor(const QColor & col)
{
    obstacleColor = col;
}

void SystemSettings::setNavLinesColor(const QColor & col)
{
    navLinesColor = col;
}

/// set/get pedestrian private sphere ellipse resolution
int SystemSettings::getEllipseResolution()
{
    return ellipseResolution;
}

void SystemSettings::setEllipseResolution(int resolution)
{
    ellipseResolution = resolution;
}

/// set/get the pedestrian shape
/// 0 for default, 1 for Ellipse, 2 for pinguins
void SystemSettings::setPedestrianShape(int shape)
{
    pedesShape = shape;
}

int SystemSettings::getPedestrianShape()
{
    return pedesShape;
}

void SystemSettings::setTrailsInfo(int count, int type, int geo)
{
    trailCount       = count;
    trailingType     = type;
    trailingGeometry = geo;
}

void SystemSettings::getTrailsInfo(int * count, int * type, int * geo)
{
    *count = trailCount;
    *type  = trailingType;
    *geo   = trailingGeometry;
}

void SystemSettings::setPedestrianColor(int groupID, int color[3])
{
    if((groupID < 0) || (groupID > 3)) {
        Log::Error(" wrong pedestrian group ID %d", groupID);
        return;
    }
    pedestriansColor[groupID][0] = color[0];
    pedestriansColor[groupID][1] = color[1];
    pedestriansColor[groupID][2] = color[2];
}

void SystemSettings::getPedestrianColor(int groupID, int color[3])
{
    if((groupID < 0) || (groupID > 3)) {
        Log::Error(" wrong pedestrian group ID %d", groupID);
        return;
    }

    color[0] = pedestriansColor[groupID][0];
    color[1] = pedestriansColor[groupID][1];
    color[2] = pedestriansColor[groupID][2];
}

void SystemSettings::setOutputDirectory(QString dir)
{
    outputDir = dir;
}

void SystemSettings::getOutputDirectory(QString & dir)
{
    dir = QString(outputDir);
}

int SystemSettings::getPedestrianCaptionSize()
{
    return captionSize;
}

void SystemSettings::setRecordPNGsequence(bool status)
{
    recordPNGsequence = status;
}

bool SystemSettings::getRecordPNGsequence()
{
    return recordPNGsequence;
}

void SystemSettings::setFilenamePrefix(QString prefix)
{
    filesPrefix = prefix;
}

QString SystemSettings::getFilenamePrefix()
{
    return filesPrefix;
}

void SystemSettings::CreateLogfile()
{
    // create directory if not exits
    if(!QDir(outputDir).exists()) {
        QDir dir;
        if(!dir.mkpath(outputDir)) {
            Log::Error("could not create directory: %s", outputDir.toStdString().c_str());
            outputDir = ""; // current
        }
    }

    logfile = outputDir + "log_" +
              QDateTime::currentDateTime()
                  .toString("yyMMdd_hh_mm_")
                  .append(SystemSettings::getFilenamePrefix())
                  .append(".txt");
    Log = new FileHandler(logfile.toStdString().c_str());
}

QString & SystemSettings::getLogfile()
{
    return logfile;
}

void SystemSettings::DeleteLogfile()
{
    // first close the opened file
    delete Log;
    if(!QFile::remove(logfile)) {
        Log::Info("Could not delete logfile %s", logfile.toStdString().c_str());
    }
}

void SystemSettings::setPedestrianColorProfileFromFile(bool readFromFile)
{
    pedColorProfileReadFromFile = readFromFile;
}

bool SystemSettings::getPedestrianColorProfileFromFile()
{
    return pedColorProfileReadFromFile;
}
void SystemSettings::setVirtualAgent(int agent)
{
    virtualAgentID = agent;
}

int SystemSettings::getVirtualAgent()
{
    return virtualAgentID;
}

void SystemSettings::setCaptionsParameters(
    int size,
    const QColor & col,
    int orientation,
    bool automaticRotation)
{
    captionSize              = size;
    captionColor             = col;
    captionOrientaton        = orientation;
    captionAutomaticRotation = automaticRotation;
}

void SystemSettings::getCaptionsParameters(
    int & size,
    QColor & col,
    int & orientation,
    bool & automaticRotation)
{
    size              = captionSize;
    col               = captionColor;
    orientation       = captionOrientaton;
    automaticRotation = captionAutomaticRotation;
}

void SystemSettings::setOnScreenInfos(bool status)
{
    onScreenInfos = status;
}

bool SystemSettings::getOnScreenInfos()
{
    return onScreenInfos;
}
