/**
 * @headerfile SystemSettings.h
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


#ifndef SYSTEMSETTINGS_H_
#define SYSTEMSETTINGS_H_

class QString;
class QColor;


class SystemSettings
{
public:
    SystemSettings();
    virtual ~SystemSettings();

    // set/get enable the legend
    void static setShowLegend(bool legend);
    bool static getShowLegend();

    // set/get the caption status
    void static setShowAgentsCaptions(bool caption);
    bool static getShowAgentsCaptions();

    // set/get the velocity status
    void static setShowDirections(bool direction);
    bool static getShowDirections();

    // set/get the listening port
    void static setListeningPort(unsigned short port);
    unsigned short static getListeningPort();

    // set/get the  2D visio
    void static set2D(bool is2D);
    bool static get2D();

    // set/get the  agents visio
    void static setShowAgents(bool status);
    bool static getShowAgents();

    // set/get the  geometry visibility
    void static setShowGeometry(bool status);
    bool static getShowGeometry();

    // set/get the  geometry captions visibility
    void static setShowGeometryCaptions(bool status);
    bool static getShowGeometryCaptions();

    // set/get the  floor visibility
    void static setShowFloor(bool status);
    bool static getShowFloor();

    // set/get the  doors visibility
    void static setShowExits(bool status);
    bool static getShowExits();

    // set/get the  walls visibility
    void static setShowWalls(bool status);
    bool static getShowWalls();

    // set/get the obstacles visibility
    void static setShowObstacles(bool status);
    bool static getShowObstacles();

    // set/get the gradient field visibility
    void static setShowGradientField(bool status);
    bool static getShowGradientField();


    // set/get the  navigation lines visibility
    void static setShowNavLines(bool status);
    bool static getShowNavLines();

    // set/get the  trajectories visibility
    void static setShowTrajectories(bool status);
    bool static getShowTrajectories();

    void static setWorkingDirectory(const QString & dir);
    void static getWorkingDirectory(QString & dir);

    // set/get the gk color
    // todo done directly in the travisto.cpp
    static const QColor & getBackgroundColor();
    void static setBackgroundColor(const QColor & col);

    static const QColor & getFloorColor();
    void static setFloorColor(const QColor & col);

    static const QColor & getWallsColor();
    void static setWallsColor(const QColor & col);

    static const QColor & getExitsColor();
    void static setExitsColor(const QColor & col);

    static const QColor & getNavLinesColor();
    void static setNavLinesColor(const QColor & col);

    static const QColor & getObstacleColor();
    void static setObstacleColor(const QColor & col);

    /// set/get pedestrian private sphere ellipse resolution
    int static getEllipseResolution();
    void static setEllipseResolution(int resolution);

    /// set/get the pedestrian shape
    /// 0 for default, 1 for Ellipse, 2 for pinguins
    void static setPedestrianShape(int shape);
    int static getPedestrianShape();

    /// define the pedestrian trail count and type
    /// type =0, backwards, 1= symetric, 3=forward
    // geometry= 0 point, polygones=1;
    void static setTrailsInfo(int count, int type, int geometry);
    void static getTrailsInfo(int * count, int * type, int * geometry);

    /// set the pedestrian colors
    void static setPedestrianColor(int groupID, int color[3]);
    void static getPedestrianColor(int groupID, int color[3]);

    /// set the pedestrian color profile
    /// either read from file, or set manually
    void static setPedestrianColorProfileFromFile(bool readFromFile);
    bool static getPedestrianColorProfileFromFile();

    /// set/get the file directory. where travisto will output video/screenshots etc..
    void static setOutputDirectory(QString dir);
    void static getOutputDirectory(QString & dir);

    /// set/get the caption size
    // void static setPedestrianCaptionSize( int size);
    int static getPedestrianCaptionSize();

    /**
     * set the caption parameters
     *
     * @param size the caption size in point, default is 30
     * @param Col the color in rgb format
     * @param orientation 0 is default, 1 is billboard
     * @param automaticRotation true if the captions should keep their original orientation
     */
    void static setCaptionsParameters(
        int size,
        const QColor & Col,
        int orientation        = 0,
        bool automaticRotation = true);

    /**
     *
     * retrieve the caption parameters
     *
     * @param size the caption size in point, default ist 30
     * @param Col the color in rgb format
     * @param orientation 0 is default, 1 is billboard
     * @param automaticRotation true if the captions should keep their original orientation
     */
    void static getCaptionsParameters(
        int & size,
        QColor & Col,
        int & orientation,
        bool & automaticRotation);

    /// set/get the virtual reality agent id
    void static setVirtualAgent(int agent);
    int static getVirtualAgent();

    /// set/get the png recording flag
    void static setRecordPNGsequence(bool status);
    bool static getRecordPNGsequence();

    /// set/get the onscreen visibility
    void static setOnScreenInfos(bool status);
    bool static getOnScreenInfos();

    ///\brief set the video/picture prefix name.
    /// This is just the name of the input file.
    void static setFilenamePrefix(QString prefix);

    ///\brief get the video/picture prefix name.
    /// This is just the name of the input file.
    static QString getFilenamePrefix();

    static void CreateLogfile();
    static QString & getLogfile();
    static void DeleteLogfile();

private:
    static bool recordPNGsequence;
    static bool pedColorProfileReadFromFile;
    static bool showLegend;
    static bool showAgentsCaptions;
    static bool showDirections;
    static bool is2D; //   Toggle 2D/3D mode
    static bool showAgents;
    static bool showGeometry;
    static bool showGeometryCaptions;
    static bool showFloor;
    static bool showObstacle;
    static bool showGradienField;
    static bool showWalls;
    static bool showExits;
    static bool showNavLines;
    static bool showTrajectories;
    static unsigned short port;
    static QColor bgColor;
    static QColor floorColor;
    static QColor wallsColor;
    static QColor exitsColor;
    static QColor obstacleColor;
    static QColor navLinesColor;
    static int ellipseResolution;
    static int pedestriansColor[3][3]; // 3 groups, and 3 color per groups
    static int pedesShape;
    static int virtualAgentID;
    static int trailCount;
    static int trailingType;
    static int trailingGeometry;
    static QString outputDir;
    static QString workingDir;
    static QString logfile;
    static int captionSize;
    static QColor captionColor;
    static int captionOrientaton;
    static bool captionAutomaticRotation;
    static QString filesPrefix;
    static bool onScreenInfos;
};

#endif /* SYSTEMSETTINGS_H_ */
