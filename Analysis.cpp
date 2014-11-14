/**
 * \file        Analysis.cpp
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J¨¹lich GmbH. All rights reserved.
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

#include "Analysis.h"
#include "geometry/Room.h"
#include "geometry/SubRoom.h"

#include "VoronoiDiagram.h"

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef __linux__
#include <sys/stat.h>
#include <dirent.h>
#else
#include <direct.h>
#endif



using namespace std;


/************************************************
 // Konstruktoren
 ************************************************/

Analysis::Analysis()
{
     _building = NULL;
     _numFrames = 0;

     _tIn = NULL;
     _tOut = NULL;
     _maxNumofPed =0;  //the maximum index of the pedestrian in the trajectory data
     _deltaF=5;											// half of the time interval that used to calculate instantaneous velocity of ped i.
     // here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
     _xCor = NULL;
     _yCor = NULL;
     _firstFrame = NULL;   // Record the first frame of each pedestrian
     _lastFrame = NULL;	// Record the last frame of each pedestrian
     _deltaT =160;   // the time interval to calculate the classic flow
     _flowVelocity = false; 						// Method A (Zhang2011a)
     _fundamentalTinTout = false; 			// Method B (Zhang2011a)
     _classicMethod = false; 					// Method C //calculate and save results of classic in separate file
     _voronoiMethod = false;  					// Method D--Voronoi method
     _cutByCircle = false;  //Adjust whether cut each original voronoi cell by a circle
     _getProfile = false;   // Whether make field analysis or not
     _outputGraph = false;   // Whether output the data for plot the fundamental diagram each frame
     _calcIndividualFD = false; //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
     _vComponent = 'B'; // to mark whether x, y or x and y coordinate are used when calculating the velocity
     //AccumPedsPassLine = NULL; // the accumulative pedestrians pass a line with time
     //AccumVPassLine = NULL;  // the accumulative instantaneous velocity of the pedestrians pass a line
     _fps = 16;												// Frame rate of data
     _fClassicRhoV = NULL;
     _fVoronoiRhoV = NULL;
     _individualFD = NULL;
     _fN_t = NULL;
     DensityPerFrame = NULL; // the measured density in each frame
     PassLine = NULL;

     _scaleX = 10;   // the size of the grid
     _scaleY = 10;
     _lowVertexX = 0;// LOWest vertex of the geometry (x coordinate)
     _lowVertexY = 0; //  LOWest vertex of the geometry (y coordinate)
     _highVertexX = 10; // Highest vertex of the geometry
     _highVertexY = 10;

     _areaForMethod_A=NULL;
     _areaForMethod_B=NULL;
     _areaForMethod_C=NULL;
     _areaForMethod_D=NULL;

     IdInFrame = NULL;
     XInFrame = NULL;
     YInFrame = NULL;
     VInFrame = NULL;
     ClassicFlow = 0;
     V_deltaT = 0;
}

Analysis::~Analysis()
{
     delete _building;
     delete  [] _firstFrame;
     delete  [] _lastFrame;
     delete  [] _tIn;
     delete  [] _tOut;

     for (int i=0; i<_maxNumofPed; i++) {
          delete [] _xCor[i];
          delete [] _yCor[i];
     }
     delete [] _xCor;
     delete [] _yCor;
}


// file.txt ---> file
std::string Analysis::GetBasename (const std::string& str)
{
     std::cout << "Splitting: " << str << '\n';
     unsigned found = str.find_last_of(".");
     return  str.substr(0,found);
}
// c:\\windows\\winhelp.exe ---> winhelp.exe
std::string Analysis::GetFilename (const std::string& str)
{
     std::cout << "GetFilename: " << str << '\n';
     unsigned found = str.find_last_of("/\\");
     return   str.substr(found+1);
}


void Analysis::InitArgs(ArgumentParser* args)
{
     string s = "Parameter:\n";

     switch (args->GetLog()) {
     case 0:
          // no log file
          //Log = new OutputHandler();
          break;
     case 1:
          if(Log) delete Log;
          Log = new STDIOHandler();
          break;
     case 2: {
          char name[CLENGTH]="";
          sprintf(name,"%s.P0.dat",args->GetErrorLogFile().c_str());
          if(Log) delete Log;
          Log = new FileHandler(name);
     }
     break;
     default:
          printf("Wrong option for Logfile!\n\n");
          exit(0);
     }

     Log->Write("INFO: \tOptionen an Simulation geben\n");


     if(args->GetIsMethodA()) {
          _flowVelocity = true;
          _areaForMethod_A = dynamic_cast<MeasurementArea_L*>( args->GetMeasurementArea(args->GetAreaIDforMethodA()) );
     }

     if(args->GetIsMethodB()) {
          _fundamentalTinTout = true;
          _classicMethod = true;
          _areaForMethod_B = dynamic_cast<MeasurementArea_B*>( args->GetMeasurementArea(args->GetAreaIDforMethodB()) );
     }

     if(args ->GetIsMethodC()) {
          _classicMethod = true;
          _areaForMethod_C = dynamic_cast<MeasurementArea_B*>( args->GetMeasurementArea(args->GetAreaIDforMethodC()) );
     }

     if(args ->GetIsMethodD()) {
          _voronoiMethod = true;
          _areaForMethod_D = dynamic_cast<MeasurementArea_B*>( args->GetMeasurementArea(args->GetAreaIDforMethodD()) );
     }

     _deltaF = args->GetDelatT_Vins();
     _deltaT = args->GetTimeIntervalA();
     _cutByCircle = args->GetIsCutByCircle();
     _getProfile = args->GetIsGetProfile();
     _outputGraph = args->GetIsOutputGraph();
     _calcIndividualFD = args->GetIsIndividualFD();
     _vComponent = args->GetVComponent();
     _scaleX = args->GetScaleX();
     _scaleY = args->GetScaleY();
     _geoPoly = ReadGeometry(args->GetGeometryFilename());

     Log->Write("INFO: \tGeometrie file: [%s]\n", args->GetGeometryFilename().c_str());

}

polygon_2d Analysis::ReadGeometry(const string& geometryFile)
{

     _building = new Building();
     _building->LoadGeometry(geometryFile);
     // create the polygons
     _building->InitGeometry();

     double geo_minX  = FLT_MAX;
     double geo_minY  = FLT_MAX;
     double geo_maxX  = -FLT_MAX;
     double geo_maxY  = -FLT_MAX;

     polygon_2d geoPoly;
     vector<Obstacle*> GeoObst;
     for(int i=0; i<_building->GetNumberOfRooms(); i++) {
          Room* room=_building->GetRoom(i);

          for( int j=0; j<room->GetNumberOfSubRooms(); j++) {
               SubRoom* subroom = room->GetSubRoom(i);
               const vector<Point>& temp_GeoPoly = subroom->GetPolygon();
               for (unsigned int j = 0; j< temp_GeoPoly.size(); j++) {
                    append(geoPoly, make<point_2d>(temp_GeoPoly[j]._x*M2CM, temp_GeoPoly[j]._y*M2CM));
                    geo_minX = (temp_GeoPoly[j]._x*M2CM<=geo_minX) ? (temp_GeoPoly[j]._x*M2CM) : geo_minX;
                    geo_minY = (temp_GeoPoly[j]._y*M2CM<=geo_minY) ? (temp_GeoPoly[j]._y*M2CM) : geo_minY;
                    geo_maxX = (temp_GeoPoly[j]._x*M2CM>=geo_maxX) ? (temp_GeoPoly[j]._x*M2CM) : geo_maxX;
                    geo_maxY = (temp_GeoPoly[j]._y*M2CM>=geo_maxY) ? (temp_GeoPoly[j]._y*M2CM) : geo_maxY;

               }
               correct(geoPoly);
               GeoObst = subroom->GetAllObstacles();
          }
          for (unsigned int k = 0; k < GeoObst.size(); k++) {
               const vector<Point>& temp_obst = GeoObst[k]->GetPolygon();

               geoPoly.inners().resize(k+1);
               geoPoly.inners().back();
               model::ring<point_2d>& inner = geoPoly.inners().back();
               for (unsigned int j = 0; j< temp_obst.size(); j++) {
                    append(inner, make<point_2d>(temp_obst[j]._x*M2CM, temp_obst[j]._y*M2CM));
               }
               correct(geoPoly);
          }
     }

     _highVertexX = geo_maxX;
     _highVertexY = geo_maxY;
     _lowVertexX = geo_minX;
     _lowVertexY = geo_minY;
     std::cout<<dsv(geoPoly)<<"\n";
     return geoPoly;
}

// initialize the global variables variables
void Analysis::InitializeVariables(TiXmlElement* xRootNode)
{

     //counting the number of frames
     _numFrames=0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame")) {
          _numFrames++;
     }
     Log->Write("INFO:\tnum Frames = %d",_numFrames);

     TiXmlNode*  xHeader = xRootNode->FirstChild("header"); // header
     //Number of agents
     if(xHeader->FirstChild("agents")) {
          _maxNumofPed=atoi(xHeader->FirstChild("agents")->FirstChild()->Value());
          Log->Write("INFO:\tmax num of peds N=%d", _maxNumofPed);
     }

     //framerate
     if(xHeader->FirstChild("frameRate")) {
          _fps=atoi(xHeader->FirstChild("frameRate")->FirstChild()->Value());
          Log->Write("INFO:\tFrame rate fps=%d", _fps);
     }

     _xCor = new double* [_maxNumofPed];
     _yCor = new double* [_maxNumofPed];
     for (int i=0; i<_maxNumofPed; i++) {
          _xCor[i] = new double [_numFrames];
          _yCor[i] = new double [_numFrames];
     }
     _firstFrame = new int[_maxNumofPed];  // Record the first frame of each pedestrian
     _lastFrame = new int[_maxNumofPed];  // Record the last frame of each pedestrian
     _tIn = new int[_maxNumofPed];				// Record the time of each pedestrian entering measurement area
     _tOut = new int[_maxNumofPed];				// Record the time of each pedestrian exiting measurement area
     bool IsinMeasurezone[_maxNumofPed];  // Record whether pedestrian i is in measurement area or not

     for(int i = 0; i <_maxNumofPed; i++) {
          for (int j = 0; j < _numFrames; j++) {
               _xCor[i][j] = 0;
               _yCor[i][j] = 0;
          }
          _firstFrame[i] = INT_MAX;
          _lastFrame[i] = INT_MIN;
          _tIn[i] = 0;
          _tOut[i] = 0;
          IsinMeasurezone[i] = false;
     }

     //processing the frames node
     TiXmlNode*  xFramesNode = xRootNode->FirstChild("frame");
     if (!xFramesNode) {
          Log->Write("ERROR: \tThe geometry should have at least one frame");
          exit(EXIT_FAILURE);
     }

     int frameNr=0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame")) {

          //todo: can be parallelized with OpenMP
          for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
                    xAgent = xAgent->NextSiblingElement("agent")) {

               //get agent id, x, y
               double x= atof(xAgent->Attribute("xPos"));
               double y= atof(xAgent->Attribute("yPos"));
               int ID= atoi(xAgent->Attribute("ID"))-1;

               _xCor[ID][frameNr] =  x*M2CM;
               _yCor[ID][frameNr] =  y*M2CM;
               if(frameNr < _firstFrame[ID]) {
                    _firstFrame[ID] = frameNr;
               }
               if(frameNr > _lastFrame[ID]) {
                    _lastFrame[ID] = frameNr;
               }
               if(_fundamentalTinTout==true) {
                    if(within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly)&&!(IsinMeasurezone[ID])) {
                         _tIn[ID]=frameNr;
                         IsinMeasurezone[ID] = true;
                    }
                    if((!within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly))&&IsinMeasurezone[ID]) {
                         _tOut[ID]=frameNr;
                         IsinMeasurezone[ID] = false;
                    }
               }
          }
          frameNr++;
     }

     DensityPerFrame = new double[_numFrames];
     for(int i=0; i<_numFrames; i++) {
          DensityPerFrame[i]=0;
     }
     PassLine = new bool[_maxNumofPed];
     for(int i=0; i<_maxNumofPed; i++) {
          PassLine[i] = false;
     }

}

void Analysis::InitializeFiles(const string& trajectoriesFilename)
{
     if(_classicMethod) {
          string results_C= "Output/Fundamental_Diagram/Classical_Voronoi/rho_v_Classic_"+trajectoriesFilename+".dat";
          if((_fClassicRhoV=CreateFile(results_C))==NULL) {
               Log->Write("cannot open file %s to write classical density and velocity\n", results_C.c_str());
               exit(EXIT_FAILURE);
          }
          fprintf(_fClassicRhoV,"#Frame \tclassical density(m^(-2))\t	classical velocity(m/s)\n");
     }
     if(_voronoiMethod) {
          string results_V=  "Output/Fundamental_Diagram/Classical_Voronoi/rho_v_Voronoi_"+trajectoriesFilename+".dat";
          if((_fVoronoiRhoV=CreateFile(results_V))==NULL) {
               Log->Write("cannot open the file to write Voronoi density and velocity\n");
               exit(EXIT_FAILURE);
          }
          fprintf(_fVoronoiRhoV,"#Frame \t Voronoi density(m^(-2))\t	Voronoi velocity(m/s)\n");
     }

     if(_calcIndividualFD) {
          string Individualfundment="Output/Fundamental_Diagram/Individual_FD/IndividualFD"+trajectoriesFilename+".dat";
          if((_individualFD=CreateFile(Individualfundment))==NULL) {
               Log->Write("cannot open the file individual\n");
               exit(EXIT_FAILURE);
          }
          fprintf(_individualFD,"#Frame	\t	PedId	\t	Individual density(m^(-2))\t	Individual velocity(m/s)\n");
     }

     if(_flowVelocity) {
          string N_t= "Output/Fundamental_Diagram/FlowVelocity/Flow_NT_"+trajectoriesFilename+"_Out.dat";
          if((_fN_t=CreateFile(N_t))==NULL) {
               Log->Write("cannot open the file %s  t\n", N_t.c_str() );
               exit(EXIT_FAILURE);
          }
          fprintf(_fN_t,"#Frame\t	Cumulative pedestrians\n");
     }

}


int Analysis::getPedsNumInFrame(TiXmlElement* xFrame) //counting the agents in the frame
{
     int numPedsInFrame=0;
     for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
               xAgent = xAgent->NextSiblingElement("agent")) {
          numPedsInFrame++;
     }
     return numPedsInFrame;
}

/**
 * From this function, Some pedestrian parameters in this frame including the instantaneous velocity, x and y coordinates,
 * as well as the corresponding PedID will be determined.
 */
void Analysis::getPedsParametersInFrame(int PedNum, TiXmlElement* xFrame, int frameNr)
{
     IdInFrame = new int[PedNum];
     XInFrame = new double[PedNum];
     YInFrame = new double[PedNum];
     VInFrame = new double[PedNum];

     int agentCnt=0;
     for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
               xAgent = xAgent->NextSiblingElement("agent")) {

          //get agent id, x, y
          double x= atof(xAgent->Attribute("xPos"));
          double y= atof(xAgent->Attribute("yPos"));
          int ID= atoi(xAgent->Attribute("ID"))-1;

          XInFrame[agentCnt] = x*M2CM;
          YInFrame[agentCnt] = y*M2CM;
          int Tpast = frameNr - _deltaF;
          int Tfuture = frameNr + _deltaF;
          VInFrame[agentCnt] = GetVinFrame(frameNr, Tpast, Tfuture, ID, _firstFrame, _lastFrame, _xCor, _yCor, _vComponent);
          IdInFrame[agentCnt] = ID+1;

          if(_flowVelocity) {
               bool IspassLine=false;
               if(frameNr >_firstFrame[ID]&&!PassLine[ID]) {
                    IspassLine = IsPassLine(_areaForMethod_A->_lineStartX,
                              _areaForMethod_A->_lineStartY,
                              _areaForMethod_A->_lineEndX,
                              _areaForMethod_A->_lineEndY, _xCor[ID][frameNr - 1],
                              _yCor[ID][frameNr - 1], _xCor[ID][frameNr],
                              _yCor[ID][frameNr]);
               }
               if(IspassLine==true) {
                    PassLine[ID] = true;
                    ClassicFlow++;
                    V_deltaT+=VInFrame[agentCnt];
               }
          }
          agentCnt++;
     }//agent
}

int Analysis::RunAnalysis(const string& filename, const string& path)
{
     string fullTrajectoriesPathName= path+"/"+filename;

     TiXmlDocument docGeo(fullTrajectoriesPathName);
     if (!docGeo.LoadFile()) {
          Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
          Log->Write("ERROR: \t could not parse the trajectories file <%s>",fullTrajectoriesPathName.c_str());
          exit(EXIT_FAILURE);
     }

     xRootNode = docGeo.RootElement();
     if( ! xRootNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          exit(EXIT_FAILURE);
     }
     if( xRootNode->ValueStr () != "trajectoriesDataset" ) {
          Log->Write("ERROR:\tRoot element value is not 'geometry'.");
          exit(EXIT_FAILURE);
     }

     InitializeVariables(xRootNode);	//initialize some global variables
     InitializeFiles(filename);   //initialize the files
     ClassicFlow=0; // the number of pedestrians pass a line in a certain time
     V_deltaT=0;   // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N

     int frameNr=0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame")) {
          int frid = atoi(xFrame->Attribute("ID"));
          if(!(frid%100)) {
               Log->Write("frame ID = %d",frid);
          }
          int numPedsInFrame = getPedsNumInFrame(xFrame);
          getPedsParametersInFrame(numPedsInFrame, xFrame, frameNr);

          if(_flowVelocity) {
               OutputFlow_NT(frid);
          }

          if(_classicMethod) {
               OutputClassicalResults(frameNr, frid, numPedsInFrame);
          }

          //------------------Voronoi Method---------------------------------
          if(_voronoiMethod) {
               if(numPedsInFrame>2) {
                    vector<polygon_2d> polygons = GetPolygons(numPedsInFrame);
                    OutputVoronoiResults(polygons, frid);
                    if(_calcIndividualFD) {
                         // if(i>beginstationary&&i<endstationary)
                         {
                              GetIndividualFD(polygons,VInFrame, IdInFrame, _areaForMethod_D->_poly, frid);
                         }
                    }
                    if(_getProfile) { //	field analysis
                         GetProfiles(boost::lexical_cast<string>(frid), polygons, VInFrame,filename);
                    }
                    if(_outputGraph) { // output the Voronoi polygons of a frame
                         OutputVoroGraph(boost::lexical_cast<string>(frid), polygons, numPedsInFrame,XInFrame, YInFrame,VInFrame,filename);
                    }
               } else {
                    cout<<" the number of the pedestrians is less than 2 !!"<< endl;
               }
          }

          frameNr++;
          delete []XInFrame;
          delete []YInFrame;
          delete []VInFrame;
          delete []IdInFrame;

     }// getFrame number j

     //--------------------Fundamental diagram based on Tin and Tout----------------------------------------------------------------------
     if(_fundamentalTinTout) {
          string FD_TinTout=  "Output/Fundamental_Diagram/TinTout/FDTinTout_"+filename+".dat";
          Log->Write("Fundamental diagram based on Tin and Tout will be calculated!");
          GetFundamentalTinTout(_tIn,_tOut,DensityPerFrame, _fps, _areaForMethod_B->_length,_maxNumofPed, FD_TinTout);
     }
     //-----------------------------------------------------------------------------------------------------------------------------------
     if(_flowVelocity) {
          string FD_FlowVelocity=  "Output/Fundamental_Diagram/FlowVelocity/FDFlowVelocity_"+filename+".dat";
          FlowRate_Velocity(_deltaT,_fps, _accumPedsPassLine,_accumVPassLine,FD_FlowVelocity);
     }


     if(_classicMethod)
          fclose(_fClassicRhoV);
     if(_voronoiMethod)
          fclose(_fVoronoiRhoV);
     if(_flowVelocity)
          fclose(_fN_t);
     delete [] PassLine;
     delete [] DensityPerFrame;

     return 0;
}

/**
 * i) get Voronoi polygons without considering the geometry
 * ii) cut these polygons by the geometry.
 * iii) if necessary, the polygons can be cut by a circle or polygon with certain raduis.
 */
vector<polygon_2d> Analysis::GetPolygons(int NrInFrm)
{
     VoronoiDiagram vd;
     vector<polygon_2d>  polygons = vd.getVoronoiPolygons(XInFrame, YInFrame, VInFrame,IdInFrame, NrInFrm);
     polygons = vd.cutPolygonsWithGeometry(polygons, _geoPoly, XInFrame, YInFrame);
     if(_cutByCircle) {
          polygons = vd.cutPolygonsWithCircle(polygons, XInFrame, YInFrame, 100);
     }
     return polygons;
}

/**
 * Output the classical density and velocity in the corresponding file
 */
void Analysis::OutputClassicalResults(int frmNr, int frmId, int numPedsInFrame)
{
     double ClassicDensity = GetClassicalDensity(XInFrame, YInFrame, numPedsInFrame, _areaForMethod_C->_poly);
     double ClassicVelocity = GetClassicalVelocity(XInFrame, YInFrame, VInFrame, numPedsInFrame, _areaForMethod_C->_poly);
     DensityPerFrame[frmNr]=ClassicDensity;
     fprintf(_fClassicRhoV,"%d\t%.3f\t%.3f\n", frmId, ClassicDensity,ClassicVelocity);
}

/**
 * Output the Voronoi density and velocity in the corresponding file
 */
void Analysis::OutputVoronoiResults(vector<polygon_2d>  polygons, int frid)
{
     double VoronoiVelocity = GetVoronoiVelocity(polygons,VInFrame,_areaForMethod_D->_poly);
     double VoronoiDensity=GetVoronoiDensity(polygons, _areaForMethod_D->_poly);
     fprintf(_fVoronoiRhoV,"%d\t%.3f\t%.3f\n",frid,VoronoiDensity, VoronoiVelocity);
}

/**
 * Output the time series of pedestrian number N passing the reference line.
 */
void Analysis::OutputFlow_NT(int frmId)
{
     _accumPedsPassLine.push_back(ClassicFlow);
     _accumVPassLine.push_back(V_deltaT);
     fprintf(_fN_t,"%d\t%d\n",frmId, ClassicFlow);
}

/*
 *  according to the location of a pedestrian in adjacent frame (pt1_X,pt1_Y) and (pr2_X,pt2_Y), we
 *  adjust whether he pass the line from Line_start to Line_end
 */
bool Analysis::IsPassLine(double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y)
{
     double x1=Line_startX;
     double y1=Line_startY;
     double x2=Line_endX;
     double y2=Line_endY;
     double x3=pt1_X;
     double y3=pt1_Y;
     double x4=pt2_X;
     double y4=pt2_Y;

     double d=(y2-y1)*(x4-x3)-(y4-y3)*(x2-x1);
     if(d==0.0) {
          return false;
     } else {
          double x0=((x2-x1)*(x4-x3)*(y3-y1)+(y2-y1)*(x4-x3)*x1-(y4-y3)*(x2-x1)*x3)/d;
          double y0=((y2-y1)*(y4-y3)*(x3-x1)+(x2-x1)*(y4-y3)*y1-(x4-x3)*(y2-y1)*y3)/(-1.0*d);
          double temp1=(x0-x1)*(x0-x2);
          double temp2=(x0-x3)*(x0-x4);
          double temp3=(y0-y1)*(y0-y2);
          double temp4=(y0-y3)*(y0-y4);
          if(temp1<10.0E-16) {
               temp1=0.0;
          }
          if(temp2<10.0E-16) {
               temp2=0.0;
          }
          if(temp3<10.0E-16) {
               temp3=0.0;
          }
          if(temp4<10.0E-16) {
               temp4=0.0;
          }
          if(temp1<=0.0&&temp2<=0.0&&temp3<=0.0&&temp4<=0.0) {
               return true;
          } else {
               return false;
          }
     }
}

//-----------------------------------------------------------------------------------------------------------------------------
/*
 * output: the fundamental diagram based on the time a pedestrian enter and exit the measurement area
 * MC 15.08.2012
 * input: outputfile is given not anymore "datafile"
 */
void Analysis::GetFundamentalTinTout(int *Tin, int *Tout, double *DensityPerFrame, int fps, double LengthMeasurementarea,int Nped, const string & ofile)
{
     FILE *fFD_TinTout;
     string fdTinTout=ofile;
     if((fFD_TinTout=CreateFile(fdTinTout))==NULL) {
          Log->Write("cannot open the file to write the TinTout data\n");
          exit(0);
     }
     fprintf(fFD_TinTout,"#person Index\t	density_i(m^(-2))\t	velocity_i(m/s)\n");
     for(int i=0; i<Nped; i++) {
          double velocity_temp=fps*CMtoM*LengthMeasurementarea/(Tout[i]-Tin[i]);
          double density_temp=0;
          for(int j=Tin[i]; j<Tout[i]; j++) {
               density_temp+=DensityPerFrame[j];
          }
          density_temp/=(Tout[i]-Tin[i]);
          fprintf(fFD_TinTout,"%d\t%f\t%f\n",i+1,density_temp,velocity_temp);
     }
     fclose(fFD_TinTout);
}

//----------------------------------------------------------------------------------------------------------------------------
/*
 * Calculate the Flow rate during a certain time interval DeltaT and the mean velocity passing a line.
 * Note: here the time interval in calculating the flow rate is modified. it is the actual time between the first person and last person
 * passing the line in DeltaT.
 * input: outputfile is given not anymore "datafile"
 */
void Analysis::FlowRate_Velocity(int DeltaT, int fps, const vector<int>& AccumPeds, const vector<double>& AccumVelocity, const string& ofile)
{

     FILE *fFD_FlowVelocity;
     string fdFlowVelocity = ofile;  //"Fundamental Diagram\\FlowVelocity\\FundamentalFlowVelocity"+datafile+".dat";
     if((fFD_FlowVelocity=CreateFile(fdFlowVelocity))==NULL) {
          Log->Write("cannot open the file to write the Flow-Velocity data\n");
          exit(0);
     }
     fprintf(fFD_FlowVelocity,"#Flow rate(1/s)	Mean velocity(m/s)\n");
     int TotalTime=AccumPeds.size();  // the total Frame of in the data file
     int TotalPeds=AccumPeds[TotalTime-1];  //the total pedestrians in the data file
     int firstPassT=-1;  // the first time that there are pedestrians pass the line
     int *pedspassT=new int[TotalPeds+1]; // the time for certain pedestrian passing the line
     for(int i=0; i<=TotalPeds; i++) {
          pedspassT[i]=-1;
     }

     for(int ix=0; ix<TotalTime; ix++) {
          if(AccumPeds[ix]>0 && firstPassT<0) {
               firstPassT=ix;
          }
          if(pedspassT[AccumPeds[ix]]<0) {
               pedspassT[AccumPeds[ix]]=ix;
          }
     }
     for(int i=firstPassT+DeltaT; i<TotalTime; i+=DeltaT) {
          double flow_rate=fps*(AccumPeds[i]-AccumPeds[i-DeltaT])*1.00/(pedspassT[AccumPeds[i]]-pedspassT[AccumPeds[i-DeltaT]]);
          double MeanV=(AccumVelocity[i]-AccumVelocity[i-DeltaT])/(AccumPeds[i]-AccumPeds[i-DeltaT]);
          fprintf(fFD_FlowVelocity,"%.3f\t%.3f\n",flow_rate,MeanV);

     }
     fclose(fFD_FlowVelocity);
     delete []pedspassT;
}

//-----------------------------------------------------------------------------------------------------------------
/*
 *  calculate individual density and velocity using voronoi method. the individual density is defined as the inverse
 *  of the area of the pedestrian's voronoi cell. The individual velocity is his instantaneous velocity at this time.
 *  note that, Only the pedestrians in the measurement area are considered.
 */
void Analysis::GetIndividualFD(const vector<polygon_2d>& polygon, double* Velocity, int* Id, const polygon_2d& measureArea, int frid)
{
     double uniquedensity=0;
     double uniquevelocity=0;
     int uniqueId=0;
     int temp=0;

     for(vector<polygon_2d>::const_iterator polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end(); polygon_iterator++) {
          typedef std::vector<polygon_2d> polygon_list;
          polygon_list v;
          intersection(measureArea, *polygon_iterator, v);
          if(!v.empty()) {
               //TODO: Check me
               uniquedensity=1.0/(area(*polygon_iterator)*CMtoM*CMtoM);
               uniquevelocity=Velocity[temp];
               uniqueId=Id[temp];
               fprintf(_individualFD,"%d\t%d\t%.3f\t%.3f\n",frid, uniqueId, uniquedensity,uniquevelocity);
          }
          temp++;
     }
}

double Analysis::Distance(double x1, double y1, double x2, double y2)
{
     return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
     //return sqrt(pow((x1-x2),2)+pow((y1-y2),2));
}

///---------------------------------------------------------------------------------------------------------------------
/*this function is to obtain the frequency distribution of the pedestrian movement through a line from (Line_startX,Line_startY)
 * to (Line_endX, Line_endY) according to the coordination of a person in two adjacent frames (pt1_X,pt1_Y) and (pt2_X,pt2_Y)
 * input: a empty array that will be used to save the results. "fraction" is the number of same parts that the line will be divided into.
 * output: the number of persons in each part of the line "frequency".
 */
void Analysis::DistributionOnLine(int *frequency,int fraction, double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y)
{
     /* in this function, we firstly calculate the intersection point between the trajectory (the line from point (pt1_X,pt1_Y)
      *  to (pt2_X,pt2_Y)) of a pedestrian and the Line. then, we determine which part of the line he or she belongs to
      */
     double sumdistance=Distance(Line_startX,Line_startY,Line_endX, Line_endY);
     double A=Line_endY-Line_startY;
     double B=Line_startX-Line_endX;
     double C=Line_endX*Line_startY-Line_startX*Line_endY;
     double d1=A*pt1_X+B*pt1_Y+C;
     double d2=A*pt2_X+B*pt2_Y+C;
     if(d1*d2<0) {
          double x1=Line_startX;
          double y1=Line_startY;
          double x2=Line_endX;
          double y2=Line_endY;
          double x3=pt1_X;
          double y3=pt1_Y;
          double x4=pt2_X;
          double y4=pt2_Y;
          double x =((x1 - x2) * (x3 * y4 - x4 * y3) - (x3 - x4) * (x1 * y2 - x2 * y1))
                            /((x3 - x4) * (y1 - y2) - (x1 - x2) * (y3 - y4));

          double y =((y1 - y2) * (x3 * y4 - x4 * y3) - (x1 * y2 - x2 * y1) * (y3 - y4))
                            /((y1 - y2) * (x3 - x4) - (x1 - x2) * (y3 - y4));
          int index=(int)floor(Distance(x,y,x1,y1)*fraction/sumdistance);
          frequency[index]++;
     } else if(d1*d2==0) {
          if(d1==0) {
               int index=(int)floor(Distance(Line_startX,Line_startY,pt1_X,pt1_Y)*fraction/sumdistance);
               frequency[index]++;
          }
     }

}

/*
 * calculate the voronoi density according to the area of the voronoi cell and the measurement area.
 * input: the list of the voronoi polygons and the measurement area
 * output: the voronoi density in the measurement area
 * note the unit of the polygons
 */
double Analysis::GetVoronoiDensity(const vector<polygon_2d>& polygon, const polygon_2d& measureArea)
{

     double density=0;
     for(vector<polygon_2d>::const_iterator polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end(); polygon_iterator++) {
          typedef std::vector<polygon_2d > polygon_list;
          polygon_list v;
          // double areaPolygon = area(*polygon_iterator);
          intersection(measureArea, *polygon_iterator, v);
          polygon_2d a,b;
          a = measureArea;
          b = *polygon_iterator;
          polygon_list v1;
          if(!v.empty()) {
               density+=area(v[0])/area(*polygon_iterator);
               if((area(v[0])/area(*polygon_iterator))>1.00001) {
                    std::cout<<dsv(v[0])<<"\n";
                    std::cout<<dsv(measureArea)<<"\n";
                    std::cout<<dsv(*polygon_iterator)<<"\n";
                    std::cout<<"this is a wrong result "<<area(v[0])<<'\t'<<area(*polygon_iterator)<<"\n";
                    correct(a);
                    correct(b);
                    intersection(a,b,v1);
                    std::cout<<dsv(v1[0])<<"\n";
                    std::cout<<dsv(a)<<"\n";
                    std::cout<<dsv(b)<<"\n";
                    //exit(EXIT_FAILURE);
               }
          }
     }
     return density/(area(measureArea)*CMtoM*CMtoM);
}

double Analysis::GetVoronoiDensity2(const vector<polygon_2d>& polygon, double* XInFrame, double* YInFrame, const polygon_2d& measureArea)
{
	double area_i=0;
	int pedsinMeasureArea=0;
	int temp=0;

	for(vector<polygon_2d>::const_iterator polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		if(within(make<point_2d>(XInFrame[temp], YInFrame[temp]), measureArea))
		{
			area_i += (area(*polygon_iterator)*CMtoM*CMtoM);
			pedsinMeasureArea++;
		}
		temp++;
	}
	if(area_i==0)
	{
		return 0;
	}
	return pedsinMeasureArea/area_i;
}

//---------------------------------------------------------------------------------------------

/*
 * calculate the classical density according to the coordinate of pedestrians (xs,ys) and the measurement area.
 * input: xs,ys, the number of pedestrians in the geometry and the measurement area
 * output: the classical density in the measurement area
 * note that the number of pedestrians should be the pedestrians in the the geometry used to cut the voronoi diagram.this
 * is very important. because sometimes the selected
 * geometry is smaller than the actual one. in this case, some pedestrian can not included in the geometry.
 */
double Analysis::GetClassicalDensity(double *xs, double *ys, int pednum, const polygon_2d& measureArea)
{
     int pedsinMeasureArea=0;
     for(int i=0; i<pednum; i++) {
          if(within(make<point_2d>(xs[i], ys[i]), measureArea)) {
               pedsinMeasureArea++;
          }
     }

     return pedsinMeasureArea/(area(measureArea)*CMtoM*CMtoM);
}

double Analysis::GetClassicalVelocity(double *xs, double *ys, double *VInFrame, int pednum, const polygon_2d& measureArea)
{
     int pedsinMeasureArea=0;
     double velocity = 0;
     for(int i=0; i<pednum; i++) {
          if(within(make<point_2d>(xs[i], ys[i]), measureArea)) {
               velocity+=VInFrame[i];
               pedsinMeasureArea++;
          }
     }
     if(pedsinMeasureArea!=0) {
          velocity /= (1.0*pedsinMeasureArea);
     } else {
          velocity = 0;
     }
     return velocity;

}
//---------------------------------------------------------------------------------------------

/*
 * calculate the voronoi velocity according to voronoi cell of each pedestrian and their instantaneous velocity "Velocity".
 * input: voronoi cell and velocity of each pedestrian and the measurement area
 * output: the voronoi velocity in the measurement area
 */
double Analysis::GetVoronoiVelocity(const vector<polygon_2d>& polygon, double* Velocity, const polygon_2d& measureArea)
{
     double meanV=0;
     int temp=0;
     for(vector<polygon_2d>::const_iterator polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end(); polygon_iterator++) {
          typedef std::vector<polygon_2d > polygon_list;
          polygon_list v;
          intersection(measureArea, *polygon_iterator, v);
          if(!v.empty()) {
               meanV+=(Velocity[temp]*area(v[0])/area(measureArea));
               if((area(v[0])/area(*polygon_iterator))>1.00001) {
                    std::cout<<"this is a wrong result"<<area(v[0])<<'\t'<<area(*polygon_iterator);;
               }
          }
          temp++;
     }
     return meanV;
}

/*
 * this function is to calculate the instantaneous velocity of ped ID in Frame Tnow based on his coordinates and his state.
 */

double Analysis::GetVinFrame(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor, char VComponent)
{

     double v=0.0;

     if(VComponent == 'X') {
          if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Xcor[ID][Tfuture] - Xcor[ID][Tpast])/(2.0 * _deltaF);  //one dimensional velocity
          } else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Xcor[ID][Tfuture] - Xcor[ID][Tnow])/(_deltaF);  //one dimensional velocity
          } else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID])) {
               v = _fps*CMtoM*(Xcor[ID][Tnow] - Xcor[ID][Tpast])/( _deltaF);  //one dimensional velocity
          }
     }
     if(VComponent == 'Y') {
          if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Ycor[ID][Tfuture] - Ycor[ID][Tpast])/(2.0 * _deltaF);  //one dimensional velocity
          } else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Ycor[ID][Tfuture] - Ycor[ID][Tnow])/(_deltaF);  //one dimensional velocity
          } else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID])) {
               v = _fps*CMtoM*(Ycor[ID][Tnow] - Ycor[ID][Tpast])/( _deltaF);  //one dimensional velocity
          }
     }
     if(VComponent == 'B') {
          if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][Tpast]),2))/(2.0 * _deltaF);  //two dimensional velocity
          } else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][Tnow]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][Tnow]),2))/(_deltaF);
          } else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID])) {
               v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tnow] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][Tnow] - Ycor[ID][Tpast]),2))/(_deltaF);  //two dimensional velocity
          }
     }

     return fabs(v);
}

void Analysis::GetProfiles(const string& frameId, const vector<polygon_2d>& polygons, double * velocity, const string& filename)
{
     string Prfvelocity="./Output/Fundamental_Diagram/Classical_Voronoi/field/velocity/Prf_v_"+filename+"_"+frameId+".dat";
     string Prfdensity="./Output/Fundamental_Diagram/Classical_Voronoi/field/density/Prf_d_"+filename+"_"+frameId+".dat";

     FILE *Prf_velocity;
     if((Prf_velocity=CreateFile(Prfvelocity))==NULL) {
          Log->Write("cannot open the file <%s> to write the field data\n",Prfvelocity.c_str());
          exit(0);
     }
     FILE *Prf_density;
     if((Prf_density=CreateFile(Prfdensity))==NULL) {
          Log->Write("cannot open the file to write the field density\n");
          exit(0);
     }

     int NRow = (int)ceil((_highVertexY-_lowVertexY)/_scaleY); // the number of rows that the geometry will be discretized for field analysis
     int NColumn = (int)ceil((_highVertexX-_lowVertexX)/_scaleX); //the number of columns that the geometry will be discretized for field analysis
     for(int row_i=0; row_i<NRow; row_i++) { //
          for(int colum_j=0; colum_j<NColumn; colum_j++) {
               polygon_2d measurezoneXY;
               {
                    const double coor[][2] = {
                              {_lowVertexX+colum_j*_scaleX,_lowVertexY+row_i*_scaleY}, {_lowVertexX+colum_j*_scaleX+_scaleX,_lowVertexY+row_i*_scaleY}, {_lowVertexX+colum_j*_scaleX+_scaleX, _lowVertexY+row_i*_scaleY+_scaleY},
                              {_lowVertexX+colum_j*_scaleX, _lowVertexY+row_i*_scaleY+_scaleY},
                              {_lowVertexX+colum_j*_scaleX,_lowVertexY+row_i*_scaleY} // closing point is opening point
                    };
                    assign_points(measurezoneXY, coor);
               }
               correct(measurezoneXY);     // Polygons should be closed, and directed clockwise. If you're not sure if that is the case, call this function
               double densityXY=GetVoronoiDensity(polygons,measurezoneXY);
               fprintf(Prf_density,"%.3f\t",densityXY);
               double velocityXY = GetVoronoiVelocity(polygons,velocity,measurezoneXY);
               fprintf(Prf_velocity,"%.3f\t",velocityXY);
          }
          fprintf(Prf_density,"\n");
          fprintf(Prf_velocity,"\n");
     }
     fclose(Prf_velocity);
     fclose(Prf_density);
}

void Analysis::OutputVoroGraph(const string & frameId, const vector<polygon_2d>& polygons, int numPedsInFrame, double* XInFrame, double* YInFrame,double* VInFrame, const string& filename)
{
/* -------TODO-----------------------
	string dir_path = "./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/";
	boost::filesystem::path dir(dir_path.c_str());
	if((boost::filesystem::create_directory(dir))==NULL)
	{
		Log->Write("cannot open the file <%s> to write the polygon data\n",dir_path.c_str());
				exit(0);
	}
	//boost::filesystem::create_directories("./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/");
*/


#if defined(_WIN32)
     mkdir("./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/");
#else 
     mkdir("./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/", 0777);
#endif


     string polygon="./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/polygon"+filename+"_"+frameId+".dat";

     //TODO: the method will fail if the directory does not not exits
     //use the CREATE File instead combined with
     //std::stringstream ss;

     ofstream polys (polygon.c_str());

     if(polys.is_open()) {
          for(vector<polygon_2d>::const_iterator polygon_iterator=polygons.begin(); polygon_iterator!=polygons.end(); polygon_iterator++) {
               polys << dsv(*polygon_iterator) << endl;
          }
     } else {
          Log->Write("ERROR:\tcannot create the file <%s>",polygon.c_str());
          exit(EXIT_FAILURE);
     }


     string v_individual="./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/speed"+filename+"_"+frameId+".dat";
     ofstream velo (v_individual.c_str());

     if(velo.is_open()) {
          for(int pts=0; pts<numPedsInFrame; pts++) {
               velo << fabs(VInFrame[pts]) << endl;
          }
     } else {
          Log->Write("ERROR:\tcannot create the file <%s>",v_individual.c_str());
          exit(EXIT_FAILURE);
     }


     string point="./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/points"+filename+"_"+frameId+".dat";
     ofstream points (point.c_str());
     if( points.is_open())  {
          for(int pts=0; pts<numPedsInFrame; pts++) {
               points << XInFrame[pts] << "\t" << YInFrame[pts] << endl;
          }
     } else {
          Log->Write("ERROR:\tcannot create the file <%s>",point.c_str());
          exit(EXIT_FAILURE);
     }


     points.close();
     polys.close();
     velo.close();

}

FILE* Analysis::CreateFile(const string& filename)
{
     //first try to create the file
     FILE* fHandle= fopen(filename.c_str(),"w");
     if(fHandle) return fHandle;

     unsigned int found=filename.find_last_of("/\\");
     string dir = filename.substr(0,found)+"/";
     //string file= filename.substr(found+1);

     // the directories are probably missing, create it
     if (mkpath((char*)dir.c_str())==-1) {
          Log->Write("ERROR:\tcannot create the directory <%s>",dir.c_str());
          return NULL;
     }
     //second and last attempt
     return fopen(filename.c_str(),"w");
}


#ifdef __linux__

int Analysis::mkpath(char* file_path, mode_t mode)
{
     assert(file_path && *file_path);
     char* p;
     for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
          *p='\0';

          if (mkdir(file_path, mode)==-1) {

               if (errno!=EEXIST) {
                    *p='/';
                    return -1;
               }
          }
          *p='/';
     }
     return 0;
}

#else

int Analysis::mkpath(char* file_path)
{
     assert(file_path && *file_path);
     char* p;
     for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
          *p='\0';

          if (_mkdir(file_path)==-1) {

               if (errno!=EEXIST) {
                    *p='/';
                    return -1;
               }
          }
          *p='/';
     }
     return 0;
}

#endif



