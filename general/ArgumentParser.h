/**
 * @file    main.cpp
 * @author  U.Kemloh, A.Portz
 * @version 0.4
 * Created on: Apr 20, 2019
 * Copyright (C) <2009-2012>
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
#ifndef ARGUMENTPARSER_H_
#define ARGUMENTPARSER_H_

#include <string>
#include <vector>
#include "Macros.h"
#include "MeasurementArea.h"

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;

//using std::string;
//using std::vector;
//using std::map;

class OutputHandler;
extern OutputHandler* Log;

class ArgumentParser {
private:

	std::string _geometryFileName;
	std::string _errorLogFile;
	std::string _trajectoriesLocation;
	std::string _trajectoriesFilename;
	std::vector<std::string> _trajectoriesFiles;

	char _vComponent;
	bool _isMethodA;
	bool _isMethodB;
	bool _isMethodC;
	bool _isMethodD;
	bool _isCutByCircle;
	bool _isOutputGraph;
	bool _isIndividualFD;
	bool _isGetProfile;
	double _steadyStart;
	double _steadyEnd;
	int _delatTVInst;
	int _timeIntervalA;
	int _areaIDforMethodA;
	int _areaIDforMethodB;
	int _areaIDforMethodC;
	int _areaIDforMethodD;
	int _scaleX;
	int _scaleY;
	int _log;

	std::map <int, MeasurementArea*> _measurementAreas;
	void Usage();

public:
	// Konstruktor
	ArgumentParser();

	const std::string& GetTrajectoriesFilename() const;
	const std::vector<std::string>& GetTrajectoriesFiles() const;
	const std::string& GetTrajectoriesLocation() const;
	const FileFormat& GetFileFormat() const;
	const std::string& GetGeometryFilename() const;
	const std::string& GetErrorLogFile() const;

	double GetLengthMeasurementArea() const;
	polygon_2d GetMeasureArea() const;
	double GetLineStartX() const;
	double GetLineStartY() const;
	double GetLineEndX() const;
	double GetLineEndY() const;

	char GetVComponent() const;
	int GetDelatT_Vins() const;
	int GetTimeIntervalA() const;
	bool GetIsMethodA() const;
	bool GetIsMethodB() const;
	bool GetIsMethodC() const;
	bool GetIsMethodD() const;
	int GetAreaIDforMethodA() const;
	int GetAreaIDforMethodB() const;
	int GetAreaIDforMethodC() const;
	int GetAreaIDforMethodD() const;
	bool GetIsCutByCircle() const;
	bool GetIsOutputGraph() const;
	bool GetIsIndividualFD() const;
	double GetSteadyStart() const;
	double GetSteadyEnd() const;
	bool GetIsGetProfile() const;
	int GetScaleX() const;
	int GetScaleY() const;
	int GetLog() const;
	void ParseArgs(int argc, char **argv);

	MeasurementArea* GetMeasurementArea(int id);

	/**
	 * parse the initialization file
	 * @param inifile
	 */
	void ParseIniFile(std::string inifile);
};

#endif /*ARGPARSER_H_*/
