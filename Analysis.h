/**
 * \file        Analysis.h
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J�lich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * The Analysis class represents a process of analyzing groups of pedestrian
 * trajectories from experiment or simulation. Different measurement methods
 * can be used and are defined by various parameters and functions.
 *
 *
 **/


#ifndef ANALYSIS_H_
#define ANALYSIS_H_

#include "general/ArgumentParser.h"
#include "tinyxml/tinyxml.h"
#include "geometry/Building.h"
#include "IO/OutputHandler.h"
#include "general/Macros.h"
#include "methods/PedData.h"

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>


using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
typedef model::ring<point_2d> ring;
typedef std::vector<polygon_2d > polygon_list;
typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double> > segment;

#include <map>
#include <vector>
extern OutputHandler* Log;

class Analysis
{

public:
     Analysis();
     virtual ~Analysis();

     void InitArgs(ArgumentParser *args);
     void InitializeFiles(const std::string& file);

     std::map<int, polygon_2d> ReadGeometry(const std::string& geometryFile, const std::vector<MeasurementArea_B*>& areas);

     /**
      * Run the analysis for different files.
      * @param file
      * @param path
      * @return
      */
     int RunAnalysis(const std::string& file, const std::string& path);

     /**
      * return the base name from the string.
      * // file.txt ---> file
      * @param str
      */
     std::string GetBasename(const std::string& str);

     /**
      * extract the filename from the path
      * // c:\\windows\\winhelp.exe ---> winhelp.exe
      * @param str
      */
     std::string GetFilename(const std::string& str);

     /**
      * create a file and the directory structure if needed.
      * @param filename
      * @return
      */
     static FILE* CreateFile(const std::string& filename);

     //TODO: merge apple and linux
#ifdef __linux__
     static int mkpath(char* file_path, mode_t mode=0755);
#elif __APPLE__
     static int mkpath(char* file_path, mode_t mode=0755);
#else //windows
     static int mkpath(char* file_path);
#endif

private:

     Building* _building;
     //polygon_2d _geoPoly;
     std::map<int, polygon_2d> _geoPoly;

     double _scaleX;      // the size of the grid
     double _scaleY;
     double _lowVertexX;  // LOWest vertex of the geometry (x coordinate)
     double _lowVertexY;  //  LOWest vertex of the geometry (y coordinate)
     double _highVertexX; // Highest vertex of the geometry
     double _highVertexY;
     int _deltaF;         // half of the time interval that used to calculate instantaneous velocity of ped i.
     int _deltaT;        // the time interval to calculate the classic flow
     bool _DoesUseMethodA;        // Method A (Zhang2011a)
     bool _DoesUseMethodB;  // Method B (Zhang2011a)
     bool _DoesUseMethodC;       // Method C //calculate and save results of classic in separate file
     bool _DoesUseMethodD;       // Method D--Voronoi method
     bool _cutByCircle;       //Adjust whether cut each original voronoi cell by a circle
     double _cutRadius;
     int _circleEdges;
     bool _getProfile;        // Whether make field analysis or not
     bool _outputGraph;       // Whether output the data for plot the fundamental diagram each frame
     bool _calcIndividualFD;  //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
     char _vComponent;        // to mark whether x, y or x and y coordinate are used when calculating the velocity
     std::string _projectRootDir;
     FileFormat _trajFormat;  // format of the trajectory file

     std::vector<MeasurementArea_L*> _areaForMethod_A;
     std::vector<MeasurementArea_B*> _areaForMethod_B;
     std::vector<MeasurementArea_B*> _areaForMethod_C;
     std::vector<MeasurementArea_B*> _areaForMethod_D;
};

#endif /*ANALYSIS_H_*/
