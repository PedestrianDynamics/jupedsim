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
using std::string;
using std::vector;
using std::pair;

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
using namespace boost::geometry;
typedef model::d2::point_xy<double,  cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;


class OutputHandler;
extern OutputHandler* Log;



class ArgumentParser {
private:

	FileFormat pFormat;

	string pGeometryFilename;
	string pErrorLogFile;
	string pTrajectoriesFile;
	string pTrajectoryName;

	string pMeasureAreaId;
	double pLengthMeasurementArea;
	polygon_2d pMeasureArea;
	double pLine_startx;
	double pLine_starty;
	double pLine_endx;
	double pLine_endy;
	char	pVComponent;
	bool pIsMethodA;
	int pTimeInterval_A;
	bool pIsMethodB;
	bool pIsMethodC;
	bool pIsMethodD;
	bool pIsCutByCircle;
	bool pIsOutputGraph;
	bool pIsIndividualFD;
	double pSteady_start;
	double pSteady_end;
	int pNrow;
	int pNcolumn;
	int pScale_x;
	int pScale_y;
	double pLow_ed_x;
	double pLow_ed_y;


	// private Funktionen
	void Usage();

public:
	// Konstruktor
	ArgumentParser(); // gibt die Programmoptionen aus

	// Getter-Funktionen
	const string& GetTrajectoriesFile() const;
	const string& GetMeasureAreaId() const;


	double GetLengthMeasurementArea() const;
	polygon_2d GetMeasureArea() const;
	double GetLine_startx() const;
	double GetLine_starty() const;
	double GetLine_endx() const;
	double GetLine_endy() const;
	char	GetVComponent() const;
	bool GetIsMethodA() const;
	int GetTimeInterval_A() const;
	bool GetIsMethodB() const;
	bool GetIsMethodC() const;
	bool GetIsMethodD() const;
	bool GetIsCutByCircle() const;
	bool GetIsOutputGraph() const;
	bool GetIsIndividualFD() const;
	double GetSteady_start() const;
	double GetSteady_end() const;
	int GetNrow() const;
	int GetNcolumn() const;
	int GetScale_x() const;
	int GetScale_y() const;



	void SetHostname(const string& hostname);
	void SetPort(int port);
	void SetTrajectoriesFile(const string& trajectoriesFile);
	void SetTrajectoriesPath(const string& trajectoriesPath);
	const string& GetGeometryFilename() const;

	//vector< pair<int, RoutingStrategy> > GetRoutingStrategy() const;
	const FileFormat& GetFileFormat() const;


	void ParseArgs(int argc, char **argv);

	/**
	 * parse the initialization file
	 * @param inifile
	 */
	void ParseIniFile(string inifile);
};

#endif /*ARGPARSER_H_*/
