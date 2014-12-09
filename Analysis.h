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

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/ring.hpp>
//#include <boost/filesystem.hpp>

using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;
typedef model::ring<point_2d> ring;
typedef std::vector<polygon_2d > polygon_list;
typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double> > segment;

#define CMtoM 0.01
#define M2CM 100
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

     polygon_2d ReadGeometry(const std::string& geometryFile);
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


private:

     /**
      * calculate the instantaneous velocity of ped ID in Frame
      * Tnow based on his coordinates and his state.
      *
      * @param Tnow
      * @param Tpast
      * @param Tfuture
      * @param ID
      * @param Tfirst
      * @param Tlast
      * @param Xcor
      * @param Ycor
      * @param VComponent
      * @return
      */
     void CreateGlobalVariables(int numPeds, int numFrames);
     double GetVinFrame(int Tnow, int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast,
               double **Xcor, double **Ycor, char VComponent);

     /**
      *  according to the location of a pedestrian in adjacent frame (pt1_X,pt1_Y) and (pr2_X,pt2_Y), we
      *  adjust whether he pass the line from Line_start to Line_end
      * @param Line_startX
      * @param Line_startY
      * @param Line_endX
      * @param Line_endY
      * @param pt1_X
      * @param pt1_Y
      * @param pt2_X
      * @param pt2_Y
      * @return
      */
     bool IsPassLine(double Line_startX, double Line_startY, double Line_endX, double Line_endY,
               double pt1_X, double pt1_Y, double pt2_X, double pt2_Y);

     /**
      * output: the fundamental diagram based on the time a pedestrian enter and exit the measurement area
      *
      * @param Tin
      * @param Tout
      * @param DensityPerFrame
      * @param fps
      * @param LengthMeasurementarea
      * @param Nped
      * @param ofile
      */
     void GetFundamentalTinTout(int *Tin, int *Tout, double *DensityPerFrame, int fps,
               double LengthMeasurementarea, int Nped, const std::string & ofile);

     /**
      * Calculate the Flow rate during a certain time interval DeltaT and the mean velocity passing a line.
      * Note: here the time interval in calculating the flow rate is modified.
      * it is the actual time between the first person and last person
      * passing the line in DeltaT.
      *
      * @param DeltaT
      * @param fps
      * @param AccumPeds
      * @param AccumVelocity
      * @param ofile
      */
     void FlowRate_Velocity(int DeltaT, int fps, const std::vector<int>& AccumPeds,
               const std::vector<double>& AccumVelocity, const std::string& ofile);

     /**
      * calculate individual density and velocity using voronoi method. the individual density is defined as the inverse
      * of the area of the pedestrian's voronoi cell. The individual velocity is his instantaneous velocity at this time.
      * note that, Only the pedestrians in the measurement area are considered.
      * @param polygon
      * @param Velocity
      * @param Id
      * @param measureArea
      * @param frid
      */
     void GetIndividualFD(const std::vector<polygon_2d>& polygon, double* Velocity, int* Id,
               const polygon_2d& measureArea, int frid);

     /**
      *
      * @return the euclidean distance between the two points
      */
     double Distance(double x1, double y1, double x2, double y2);

     /**
      * calculate the voronoi density according to the area of the voronoi cell and the measurement area.
      * input: the list of the voronoi polygons and the measurement area
      * note the unit of the polygons
      * @param polygon
      * @param measureArea
      * @return the voronoi density in the measurement area
      */
     double GetVoronoiDensity(const std::vector<polygon_2d>& polygon,
               const polygon_2d& measureArea);

     /**
      * calculate the voronoi velocity according to voronoi cell of each pedestrian and their instantaneous velocity "Velocity".
      * input: voronoi cell and velocity of each pedestrian and the measurement area
      * @param polygon
      * @param Velocity
      * @param measureArea
      * @return the voronoi velocity in the measurement area
      */
     double GetVoronoiVelocity(const std::vector<polygon_2d>& polygon, double* Velocity,
               const polygon_2d& measureArea);

     /**
      * calculate the classical density according to the coordinate of pedestrians (xs,ys) and the measurement area.
      * input: xs,ys, the number of pedestrians in the geometry and the measurement area
      * note that the number of pedestrians should be the pedestrians in the the geometry used to cut the voronoi diagram.this
      * is very important. because sometimes the selected
      * geometry is smaller than the actual one. in this case, some pedestrian can not included in the geometry.
      * @param xs
      * @param ys
      * @param pednum
      * @param measureArea
      * @return the classical density in the measurement area
      */
     double GetClassicalDensity(double *xs, double *ys, int pednum, const polygon_2d& measureArea);

     /**
      *
      * @param xs
      * @param ys
      * @param VInFrame
      * @param pednum
      * @param measureArea
      * @return
      */
     double GetClassicalVelocity(double *xs, double *ys, double *VInFrame, int pednum,
               const polygon_2d& measureArea);

     /**
      *
      * @param frameId
      * @param polygons
      * @param velocity
      * @param filename
      */
     void GetProfiles(const std::string& frameId, const std::vector<polygon_2d>& polygons,
               double * velocity, const std::string& filename);

     /**
      *
      * @param frameId
      * @param polygons
      * @param numPedsInFrame
      * @param XInFrame
      * @param YInFrame
      * @param VInFrame
      * @param filename
      */
     void OutputVoroGraph(const std::string & frameId, const std::vector<polygon_2d>& polygons,
               int numPedsInFrame, double* XInFrame, double* YInFrame, double* VInFrame,
               const std::string& filename);

     /**
      *
      * @param frequency
      * @param fraction
      * @param Line_startX
      * @param Line_startY
      * @param Line_endX
      * @param Line_endY
      * @param pt1_X
      * @param pt1_Y
      * @param pt2_X
      * @param pt2_Y
      */
     void DistributionOnLine(int *frequency, int fraction, double Line_startX, double Line_startY,
               double Line_endX, double Line_endY, double pt1_X, double pt1_Y, double pt2_X,
               double pt2_Y);

     /**
      *
      * @param xRootNode
      */
     void InitializeVariables(TiXmlElement* xRootNode);

     /**
      * @return the total number of pedestrians in that frame
      */

     void InitializeVariables(const std::string& filename);
     /**
      * read globe variables from .txt format trajectory files
      */

     int getPedsNumInFrame(TiXmlElement* xFrame);


     /**
      * create a file and the directory structure if needed.
      * @param filename
      * @return
      */
     FILE* CreateFile(const std::string& filename);

     /**
      * Output the classical density and velocity in the corresponding file
      * @param frmNr
      * @param frmId
      * @param numPedsInFrame
      */
     void OutputClassicalResults(int frmNr, int frmId, int numPedsInFrame);

     /**
      * Output the Voronoi density and velocity in the corresponding file
      * @param polygons
      * @param frid
      */
     void OutputVoronoiResults(const std::vector<polygon_2d>& polygons, int frid);

     /**
      * Output the time series of pedestrian number N passing the reference line.
      * @param frmId
      */
     void OutputFlow_NT(int frmId);

     /**
      *
      * @param polygon
      * @param XInFrame
      * @param YInFrame
      * @param measureArea
      * @return
      */
     double GetVoronoiDensity2(const std::vector<polygon_2d>& polygon, double* XInFrame,
               double* YInFrame, const polygon_2d& measureArea);

     /**
      *  i) get Voronoi polygons without considering the geometry
      * ii) cut these polygons by the geometry.
      * iii) if necessary, the polygons can be cut by a circle or polygon with certain raduis.
      * @param NrInFrm
      * @return
      */
     std::vector<polygon_2d> GetPolygons(int NrInFrm);

     /**
      * create a directory depending on the platform
      * @param file_path
      * @return
      */
     void getIDinFrame(int Frame);

     /**
      * get the ids in a certain Frame
      */
     void getPedsParametersInFrame(int frm, std::map< int, std::vector<int> > &pdt);
     /**
      * In this function, pedestrian parameters in this frame including the instantaneous velocity, x and y coordinates,
      * as well as the corresponding PedID will be determined.
      * @param PedNum
      * @param xFrame
      * @param frameNr
      */
#ifdef __linux__
     int mkpath(char* file_path, mode_t mode=0755);
#elif __APPLE__
     int mkpath(char* file_path, mode_t mode=0755);
#else //windows
     int mkpath(char* file_path);
#endif

private:
     FILE *_fClassicRhoV;
     FILE *_fVoronoiRhoV;
     FILE *_individualFD;
     FILE *_fN_t;
     Building* _building;
     polygon_2d _geoPoly;

	 std::vector<int> _IdsTXT;   // the Id data from txt format trajectory data
	 std::vector<int> _FramesTXT;  // the Frame data from txt format trajectory data

     double _scaleX;      // the size of the grid
     double _scaleY;
     double _lowVertexX;  // LOWest vertex of the geometry (x coordinate)
     double _lowVertexY;  //  LOWest vertex of the geometry (y coordinate)
     double _highVertexX; // Highest vertex of the geometry
     double _highVertexY;
     int _numFrames;      // Total number of frames
     int _fps;            // Frame rate of data
     int *_tIn;           //the time for each pedestrian enter the measurement area
     int *_tOut;          //the time for each pedestrian exit the measurement area
     int _deltaF;         // half of the time interval that used to calculate instantaneous velocity of ped i.
     int _maxNumofPed;    //the maximum index of the pedestrian in the trajectory data
     // here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
     double **_xCor;
     double **_yCor;


     int *_firstFrame;   // Record the first frame of each pedestrian
     int *_lastFrame;    // Record the last frame of each pedestrian
     int _deltaT;        // the time interval to calculate the classic flow

     double *DensityPerFrame; // the measured density in each frame
     bool *PassLine;

     bool _flowVelocity;        // Method A (Zhang2011a)
     bool _fundamentalTinTout;  // Method B (Zhang2011a)
     bool _classicMethod;       // Method C //calculate and save results of classic in separate file
     bool _voronoiMethod;       // Method D--Voronoi method

     MeasurementArea_L* _areaForMethod_A;
     MeasurementArea_B* _areaForMethod_B;
     MeasurementArea_B* _areaForMethod_C;
     MeasurementArea_B* _areaForMethod_D;

     bool _cutByCircle;       //Adjust whether cut each original voronoi cell by a circle
     double _cutRadius;
     int _circleEdges;
     bool _getProfile;        // Whether make field analysis or not
     bool _outputGraph;       // Whether output the data for plot the fundamental diagram each frame
     bool _calcIndividualFD;  //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
     char _vComponent;        // to mark whether x, y or x and y coordinate are used when calculating the velocity
     std::vector<int> _accumPedsPassLine; // the accumulative pedestrians pass a line with time
     std::vector<double> _accumVPassLine; // the accumulative instantaneous velocity of the pedestrians pass a line

     std::string _projectRootDir;

     int *IdInFrame;     // save the ped ID in the geometry in this frame, which is the same order with VInFrame and only used for outputting individual density and velocity.
     double *XInFrame;   // save the X coordinates of pedestrian in the geometry in this frame
     double *YInFrame;   // save the Y coordinates of pedestrian in the geometry in this frame
     double *VInFrame;   // save the instantaneous velocity of pedestrians in the geometry in this frame
     int ClassicFlow;    // the number of pedestrians pass a line in a certain time
     double V_deltaT;    // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N
     FileFormat _trajFormat;  // format of the trajectory file

     int min_ID;
     int min_Frame;
     std::map<int , std::vector<int> > peds_t;
};

#endif /*ANALYSIS_H_*/
