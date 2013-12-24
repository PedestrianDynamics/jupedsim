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


class SystemSettings {
public:
	SystemSettings();
	virtual ~SystemSettings();

	// set/get enable the legend
	void static setShowLegend(bool legend);
	bool static getShowLegend();

	//set/get the caption status
	void static setShowCaptions(bool caption);
	bool static getShowCaption();

	// set/get the listening port
	void static setListningPort(unsigned short port);
	unsigned short static getListeningPort();

	// set/get the  visio
	void static set2D(bool is2D);
	bool static get2D();

	// set/get the bk color
	// todo done directly in the travisto.cpp
	void static getBackgroundColor(double* col);
	void static setBackgroundColor(double* col);

	/// set/get pedestrian private sphere ellipse resolution
	int static getEllipseResolution();
	void static setEllipseResolution(int resolution);


	/// set/get the pedestrian shape
	/// 0 for default, 1 for Ellipse, 2 for pinguins
	void static setPedestrianShape(int shape);
	int static getPedestrianShape();

	/// define the pedestrian trail count and type
	/// type =0, backwards, 1= symetric, 3=forward
	//geometry= 0 point, polygones=1;
	void static setTrailsInfo(int count, int type, int geometry);
	void static getTrailsInfo(int* count, int *type, int *geometry);

	/// set the pedestrian colors
	void static setPedestrianColor(int groupID, int color[3]);
	void static getPedestrianColor(int groupID, int color[3]);

	/// set the pedestrian color profile
	/// either read from file, or set manually
	void static setPedestrianColorProfileFromFile(bool readFromFile);
	bool static getPedestrianColorProfileFromFile();

	///set/get the file directory. where travisto will output video/screenshots etc..
	void static setOutputDirectory(QString dir);
	void static getOutputDirectory(QString& dir);

	/// set/get the caption size
	//void static setPedestrianCaptionSize( int size);
	int static getPedestrianCaptionSize();

	/**
	 * set the caption parameters
	 *
	 * @param size the caption size in point, default is 30
	 * @param Col the color in rgb format
	 * @param orientation 0 is default, 1 is billboard
	 * @param automaticRotation true if the captions should keep their original orientation
	 */
	void static setCaptionsParameters(int size, const QColor& Col, int orientation=0, bool automaticRotation=true);

	/**
	 *
	 * retrieve the caption parameters
	 *
	 * @param size the caption size in point, default ist 30
	 * @param Col the color in rgb format
	 * @param orientation 0 is default, 1 is billboard
	 * @param automaticRotation true if the captions should keep their original orientation
	 */
	void static getCaptionsParameters(int &size, QColor& Col, int &orientation, bool &automaticRotation);

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
	static QString  getFilenamePrefix();

private:

	static bool recordPNGsequence;
	static bool pedColorProfileReadFromFile;
	static bool showLegend;
	static bool showCaption;
	static bool is2D; //   Toggle 2D/3D mode
	static unsigned short port;
	static double bgColor[3];
	//static double ellipseShape[2];
	static int ellipseResolution;
	static int pedestriansColor[3][3]; // 3 groups, and 3 color per groups
	static int pedesShape;
	static int virtualAgentID;
	static int trailCount;
	static int trailingType;
	static int trailingGeometry;
	static QString outputDir;
	static int captionSize;
	static QColor captionColor;
	static int captionOrientaton;
	static bool captionAutomaticRotation;
	static QString filesPrefix;
	static bool onScreenInfos;
};

#endif /* SYSTEMSETTINGS_H_ */
