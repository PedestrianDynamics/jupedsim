/**
 * \file        Analysis.h
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
#include "general/Macros.h"
#include "methods/ConfigData_D.h"
#include "methods/PedData.h"

#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometry.hpp>
#include <tinyxml.h>

using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
typedef model::ring<point_2d> ring;
typedef std::vector<polygon_2d> polygon_list;
typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

#include <map>
#include <vector>

class Analysis
{
public:
    Analysis();
    ~Analysis() = default;

    void InitArgs(ArgumentParser * args);

    /**
      * Run the analysis for different files.
      * @param file
      * @param path
      * @return
      */
    int RunAnalysis(const fs::path & file, const fs::path & path);

    /**
      * return the base name from the string.
      * // file.txt ---> file
      * @param str
      */
    std::string GetBasename(const std::string & str);

    /**
      * extract the filename from the path
      * // c:\\windows\\winhelp.exe ---> winhelp.exe
      * @param str
      */
    std::string GetFilename(const std::string & str);

    /**
      * create a file and the directory structure if needed.
      * @param filename
      * @return
      */
    static FILE * CreateFile(const std::string & filename);

private:
    /**
     * Checks which rooms contain measurement areas \p areas.
     * @param areas Measurement areas of interest
     * @return Mapping from ID of measurement area to polygon.
     */
    std::map<int, polygon_2d>
    GetRoomForMeasurementArea(const std::vector<MeasurementArea_B *> & areas);

    /**
     * Compute the bounding box around all polygons contained in \p polygons. Bounding box can be
     * extended by \p extension.
     * @param polygons polygons of interest
     * @param extension extension of the bounding box [in cm]
     * @return bounding box of \p polygons extended \p extension
     */
    boost::geometry::model::box<point_2d>
    GetBoundingBox(const std::vector<polygon_2d> & polygons, double extension = 0.);

private:
    std::map<int, polygon_2d> _geoPolyMethodD;
    std::vector<polygon_2d> _geometry;
    polygon_2d _boundingBox;

    double _lowVertexX;  // LOWest vertex of the geometry (x coordinate)
    double _lowVertexY;  //  LOWest vertex of the geometry (y coordinate)
    double _highVertexX; // Highest vertex of the geometry
    double _highVertexY;
    int _deltaF; // half of the time interval that used to calculate instantaneous velocity of ped i.
    std::vector<int> _deltaT; // the time interval to calculate the classic flow
    bool _DoesUseMethodA;     // Method A (Zhang2011a)
    bool _DoesUseMethodB;     // Method B (Zhang2011a)
    bool _DoesUseMethodC;     // Method C //calculate and save results of classic in separate file
    bool _DoesUseMethodD;     // Method D--Voronoi method

    std::string
        _vComponent; // to mark whether x, y or x and y coordinate are used when calculating the velocity
    bool _IgnoreBackwardMovement;

    fs::path _projectRootDir;
    fs::path _scriptsLocation;
    fs::path _outputLocation;
    fs::path _geometryFileName;
    FileFormat _trajFormat; // format of the trajectory file

    std::vector<MeasurementArea_L *> _areasForMethodA;
    std::vector<MeasurementArea_B *> _areasForMethodB;
    std::vector<MeasurementArea_B *> _areasForMethodC;
    std::vector<MeasurementArea_B *> _areasForMethodD;

    ConfigData_D configData_D;
};

#endif /*ANALYSIS_H_*/
