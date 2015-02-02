/**
 * \file        ArgumentParser.cpp
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum Jï¿½ï¿½lich GmbH. All rights reserved.
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
 * The ArgumentParser class define functions reading the input parameters from initial files.
 *
 *
 **/


#include <getopt.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <dirent.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"
#include "../Analysis.h"

using namespace std;

void ArgumentParser::Usage(const std::string file)
{

     Log->Write("Usage: \n");
     Log->Write("\t%s input.xml\n",file.c_str());
     /*
    fprintf(stderr,
    		"Usage: program options\n\n"
    		"With the following options (default values in parenthesis):\n\n"
    		"	[-t/--trajectory <string>]						name of input trajectory file\n"
    		"  	[-I/--input path <filepath>]    				path of the input file(./Inputfiles/)\n"
    		"  	[-O/--output path <filepath>]   				path of the input file(./Outputfiles/)\n"
    		"	[-g/--geometry <string>]        				path to the geometry file (./Inputfiles/geo.xml)\n"
    		"	[-m/--measurement area <int>]    			type of measurement area(1)\n"
    		"                                   							1: Bounding box\n"
    		"                                       						2: Line\n"
    		"	[-b/--bounding box  <double>]				p1.x p1.y p2.x p2.y p3.x p3.y p4.x p4.y (in clockwise)\n"
    		"	[-d/--moving direction <double>]			p1.x p1.y p2.x p2.y \n"
    		"	[-l/--line <double>]								p1.x p1.y p2.x p2.y \n"

    fprintf(stderr, "-c --> set cutbycircle=true (false)\n");
    fprintf(stderr, "-a --> set fieldAnalysis=true (false)\n");
    fprintf(stderr, "-g --> set IsOutputGraph=true (false)\n");
    fprintf(stderr, "-v --> set calcIndividualfunddata=true (false)\n");
    fprintf(stderr, "-s scale (3000)\n");
    fprintf(stderr, "-l --> set IsClassicMethod=true (false)\n");
    fprintf(stderr, "-F --> set IsFundamentalTinTout=true (false). density is classical. So IsClassicMethod should be true\n");
    fprintf(stderr, "-V --> set IsFlowVelocity=true (false)\n");
    fprintf(stderr, "-L x1 y1 x2 y2 (0.0, 300.0, 250.0, 300.0)\n");
    fprintf(stderr, "-y  beginstationary (700)\n");
    fprintf(stderr, "-Y  endstationary (1800)\n");
    fprintf(stderr, "-R Row (65)\n");
    fprintf(stderr, "-C Column (80)\n");
    fprintf(stderr, "-m  Meas. Area ax1 (-300)\n");
    fprintf(stderr, "-n  Meas. Area ay1 (100)\n");
    fprintf(stderr, "-M  Meas. Area ax2 (300)\n");
    fprintf(stderr, "-N  Meas. Area ay2 (200)\n");
    fprintf(stderr, "-o  Outputfile (result.dat)\n");
    fprintf(stderr, "-O  goes in the name of the polygons, speed and point files (dummy)\n");
    fprintf(stderr, "-d --> set use_Vxy false (true)\n");
    fprintf(stderr, "-e --> set use_Vy true (false)\n");
    fprintf(stderr, "-k --> set use_Vx true (false)\n");
    fprintf(stderr, "-p fps (10)\n");
    fprintf(stderr, "-G cor_x cor_y length width (corridor)\n");

      */
     exit(EXIT_SUCCESS);
}

ArgumentParser::ArgumentParser()
{
     // Default parameter values
     _geometryFileName = "geo.xml";

     _vComponent = 'B';
     _isMethodA = false;
     _timeIntervalA = 160;
     _delatTVInst = 5;
     _isMethodB = false;
     _isMethodC =false;
     _isMethodD = false;
     _isCutByCircle = false;
     _isOutputGraph= false;
     _isIndividualFD = false;
     _isGetProfile =false;
     _steadyStart =100;
     _steadyEnd = 1000;
     _scaleX = 10;
     _scaleY = 10;
     _errorLogFile="./Logfile.dat";
     _log=1; //no output wanted
     _trajectoriesLocation="./";
     _trajectoriesFilename="";
     _projectRootDir="./";
     _fileFormat=FORMAT_XML_PLAIN;
     _cutRadius =50;
     _circleEdges=6;
}



bool ArgumentParser::ParseArgs(int argc, char **argv)
{
     //special case of the default configuration ini.xml
     if (argc == 1)
     {
          Log->Write(
                    "INFO: \tTrying to load the default configuration from the file <ini.xml>");
          if(ParseIniFile("ini.xml")==false)
          {
               Usage(argv[0]);
          }
          return true;
     }

     string argument = argv[1];
     if (argument == "-h" || argument == "--help")
     {
          Usage(argv[0]);
          return false;
     }
     else if(argument == "-v" || argument == "--version")
     {
          fprintf(stderr,"You are actually using JuPedsim (jpsreport) version %s  \n\n",JPS_VERSION);
          return false;
     }

     // other special case where a single configuration file is submitted
     //check if inifile options are given
     if (argc == 2)
     {
          string prefix1 = "--ini=";
          string prefix2 = "--inifile=";

          if (!argument.compare(0, prefix2.size(), prefix2)) {
               argument.erase(0, prefix2.size());
          } else if (!argument.compare(0, prefix1.size(), prefix1)) {
               argument.erase(0, prefix1.size());
          }
          return ParseIniFile(argument);
     }

     //more than one argument was supplied
     Usage(argv[0]);
     return false;
}

const vector<string>& ArgumentParser::GetTrajectoriesFiles() const
{
     return _trajectoriesFiles;
}

const string& ArgumentParser::ArgumentParser::GetProjectRootDir() const
{
     return _projectRootDir;
}


bool ArgumentParser::ParseIniFile(const string& inifile)
{

     Log->Write("INFO: \tParsing the ini file <%s>",inifile.c_str());

     //extract and set the project root dir
     size_t found = inifile.find_last_of("/\\");
     if (found != string::npos)
          _projectRootDir = inifile.substr(0, found) + "/";


     TiXmlDocument doc(inifile);
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \tCould not parse the ini file");
          return false;
     }


     TiXmlElement* xMainNode = doc.RootElement();
     if( ! xMainNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          return false;
     }

     if( xMainNode->ValueStr () != "JPSreport" )
     {
          Log->Write("ERROR:\tRoot element value is not 'JPSreport'.");
          return false;
     }

     //geometry
     if(xMainNode->FirstChild("geometry"))
     {
          _geometryFileName=_projectRootDir+xMainNode->FirstChildElement("geometry")->Attribute("file");
          Log->Write("INFO: \tGeometry File is: <"+_geometryFileName+">");
     }

     //trajectories
     TiXmlNode* xTrajectories = xMainNode->FirstChild("trajectories");
     if (xTrajectories)
     {
          //add the extension point
          string fmt = "."+string (xmltoa(xMainNode->FirstChildElement("trajectories")->Attribute("format")));
          Log->Write("INFO: \tFormat of the trajectory file is: <%s>", fmt.c_str());
          if (fmt == ".xml")
          {
               _fileFormat = FORMAT_XML_PLAIN;
          }
          else if (fmt == ".txt")
          {
               _fileFormat = FORMAT_PLAIN;
          }
          else
          {
               Log->Write("Error: \tthe given trajectory format is not supported. Supply '.xml' or '.txt' format!");
               return false;
          }

          string unit = xmltoa(xMainNode->FirstChildElement("trajectories")->Attribute("unit"), "m");
          if (unit != "m")
          {
               Log->Write("WARNING: \tonly <m> unit is supported. Convert your units.");
               return false;
          }
          //a file descriptor was given
          for (TiXmlElement* xFile = xTrajectories->FirstChildElement("file");
                    xFile; xFile = xFile->NextSiblingElement("file"))
          {
               //collect all the files given
               _trajectoriesFilename =
                         + xFile->Attribute("name");
               _trajectoriesFiles.push_back(_trajectoriesFilename);

               //check if the given file match the format
               if(boost::algorithm::ends_with(_trajectoriesFilename,fmt))
               {
                    Log->Write("INFO: \tInput trajectory file is\t<"+ (_trajectoriesFilename)+">");
               }
               else
               {
                    Log->Write("ERROR: \tWrong file extension\t<%s> for file <%s>",fmt.c_str(),_trajectoriesFilename.c_str());
                    return false;
               }
          }

          if (xTrajectories->FirstChildElement("path"))
          {
               _trajectoriesLocation = xTrajectories->FirstChildElement("path")->Attribute("location");
               if(_trajectoriesLocation.empty())
                    _trajectoriesLocation="./";


               //hack to find if it is an absolute path
               // ignore the project root in this case
               if ( (boost::algorithm::contains(_trajectoriesLocation,":")==false) && //windows
                         (boost::algorithm::starts_with(_trajectoriesLocation,"/") ==false)) //linux
                    // &&() osx
               {
                    _trajectoriesLocation=_projectRootDir+_trajectoriesLocation;
               }

               // in the case no file was specified, collect all xml files in the specified directory
               if(_trajectoriesFiles.empty())
               {
                    DIR *dir;
                    struct dirent *ent;
                    if ((dir = opendir (_trajectoriesLocation.c_str())) != NULL)
                    {
                         /* print all the files and directories within directory */
                         while ((ent = readdir (dir)) != NULL)
                         {
                              string filename=ent->d_name;

                              if (boost::algorithm::ends_with(filename, fmt))
                                   //if (filename.find(fmt)!=std::string::npos)
                              {
                                   //_trajectoriesFiles.push_back(_projectRootDir+filename);
                                   _trajectoriesFiles.push_back(filename);
                                   Log->Write("INFO: \tInput trajectory file is\t<"+ (filename)+">");
                              }
                         }
                         closedir (dir);
                    }
                    else
                    {
                         /* could not open directory */
                         Log->Write("ERROR: \tcould not open the directory <"+_trajectoriesLocation+">");
                         return false;
                    }
               }
          }
          Log->Write("INFO: \tInput directory for loading trajectory is:\t<"+ (_trajectoriesLocation)+">");
     }

     //measurement area
     if(xMainNode->FirstChild("measurementAreas")) {

          string unit = xMainNode->FirstChildElement("measurementAreas")->Attribute("unit");
          if(unit!="m")
          {
               Log->Write("WARNING: \tonly <m> unit is supported. Convert your units.");
               return false;
          }

          for(TiXmlNode* xMeasurementArea_B=xMainNode->FirstChild("measurementAreas")->FirstChild("area_B");
                    xMeasurementArea_B; xMeasurementArea_B=xMeasurementArea_B->NextSibling("area_B"))
          {
               MeasurementArea_B* areaB = new MeasurementArea_B();
               areaB->_id=xmltoi(xMeasurementArea_B->ToElement()->Attribute("id"));
               areaB->_type=xMeasurementArea_B->ToElement()->Attribute("type");

               polygon_2d poly;
               Log->Write("INFO: \tMeasure area id  < %d> with type <%s>",areaB->_id, areaB->_type.c_str());
               for(TiXmlElement* xVertex=xMeasurementArea_B->FirstChildElement("vertex"); xVertex; xVertex=xVertex->NextSiblingElement("vertex") )
               {
                    double box_px = xmltof(xVertex->Attribute("x"))*M2CM;
                    double box_py = xmltof(xVertex->Attribute("y"))*M2CM;
                    boost::geometry::append(poly, boost::geometry::make<point_2d>(box_px, box_py));
                    Log->Write("\t\tMeasure area points  < %.3f, %.3f>",box_px*CMtoM,box_py*CMtoM);
               }
               correct(poly); // in the case the Polygone is not closed
               areaB->_poly=poly;

               TiXmlElement* xLength=xMeasurementArea_B->FirstChildElement("Length_in_movement_direction");
               if(xLength)
               {
                    areaB->_length=xmltof(xLength->Attribute("distance"));
               }
               _measurementAreas[areaB->_id]=areaB;
          }


          for(TiXmlNode* xMeasurementArea_L=xMainNode->FirstChild("measurementAreas")->FirstChild("area_L");
                    xMeasurementArea_L; xMeasurementArea_L=xMeasurementArea_L->NextSibling("area_L"))
          {
               MeasurementArea_L* areaL = new MeasurementArea_L();
               areaL->_id=xmltoi(xMeasurementArea_L->ToElement()->Attribute("id"));
               areaL->_type=xMeasurementArea_L->ToElement()->Attribute("type");
               Log->Write("INFO: \tMeasure area id  <%d> with type <%s>",areaL->_id,areaL->_type.c_str());
               areaL->_lineStartX = xmltof(xMeasurementArea_L->FirstChildElement("start")->Attribute("x"))*M2CM;
               areaL->_lineStartY =xmltof(xMeasurementArea_L->FirstChildElement("start")->Attribute("y"))*M2CM;
               areaL->_lineEndX = xmltof(xMeasurementArea_L->FirstChildElement("end")->Attribute("x"))*M2CM;
               areaL->_lineEndY =xmltof(xMeasurementArea_L->FirstChildElement("end")->Attribute("y"))*M2CM;

               _measurementAreas[areaL->_id]=areaL;
               Log->Write("\t\tMeasurement line starts from  <%f, %f> to <%f, %f>",areaL->_lineStartX*CMtoM,areaL->_lineStartY*CMtoM,areaL->_lineEndX*CMtoM,areaL->_lineEndY*CMtoM);
          }
     }

     //instantaneous velocity
     TiXmlNode* xVelocity=xMainNode->FirstChild("velocity");
     if(xVelocity) {
          string UseXComponent = xVelocity->FirstChildElement("useXComponent")->GetText();
          string UseYComponent = xVelocity->FirstChildElement("useYComponent")->GetText();
          string HalfFrameNumberToUse = xVelocity->FirstChildElement("halfFrameNumberToUse")->GetText();

          _delatTVInst = atof(HalfFrameNumberToUse.c_str());
          if(UseXComponent == "true"&&UseYComponent == "false")
          {
               _vComponent = 'X';
               Log->Write("INFO: \tOnly x-component coordinates will be used in velocity calculation within  2* <"+HalfFrameNumberToUse+" frames>" );
          }
          else if(UseXComponent == "false"&&UseYComponent == "true")
          {
               _vComponent = 'Y';
               Log->Write("INFO: \tOnly y-component coordinates will be used in velocity calculation within  2* <"+HalfFrameNumberToUse+" frames>" );
          }
          else if(UseXComponent == "true"&&UseYComponent == "true")
          {
               _vComponent = 'B';  // both components
               Log->Write("INFO: \tBoth x and y-component of coordinates will be used in velocity calculation within  2* <"+HalfFrameNumberToUse+" frames>" );
          }
          else
          {
               Log->Write("Error: \tType of velocity is not selected, please check it !!! " );
               return false;
          }
     }

     // method A
     TiXmlElement* xMethod_A=xMainNode->FirstChildElement("method_A");
     if(xMethod_A)
     {
          if(string(xMethod_A->Attribute("enabled"))=="true")
          {
               _isMethodA = true;
               Log->Write("INFO: \tMethod A is selected" );
               _timeIntervalA = xmltoi(xMethod_A->FirstChildElement("timeInterval")->GetText());
               Log->Write("INFO: \ttime interval used in Method A is <%d> frame",_timeIntervalA);
               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_A")->FirstChildElement("measurementArea");
                         xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurementArea"))
               {
                    _areaIDforMethodA.push_back(xmltoi(xMeasurementArea->Attribute("id")));
                    Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", xmltoi(xMeasurementArea->Attribute("id")));
               }
          }
     }
     // method B
     TiXmlElement* xMethod_B=xMainNode->FirstChildElement("method_B");
     if(xMethod_B)

          if(string(xMethod_B->Attribute("enabled"))=="true")
          {
               _isMethodB = true;
               Log->Write("INFO: \tMethod B is selected" );
               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_B")->FirstChildElement("measurementArea");
                         xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurementArea"))
               {
                    _areaIDforMethodB.push_back(xmltoi(xMeasurementArea->Attribute("id")));
                    Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", xmltoi(xMeasurementArea->Attribute("id")));
               }
          }
     // method C
     TiXmlElement* xMethod_C=xMainNode->FirstChildElement("method_C");
     if(xMethod_C)
          if(string(xMethod_C->Attribute("enabled"))=="true")
          {
               _isMethodC = true;
               Log->Write("INFO: \tMethod C is selected" );
               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_C")->FirstChildElement("measurementArea");
                         xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurementArea"))
               {
                    _areaIDforMethodC.push_back(xmltoi(xMeasurementArea->Attribute("id")));
                    Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", xmltoi(xMeasurementArea->Attribute("id")));
               }
          }
     // method D
     TiXmlElement* xMethod_D=xMainNode->FirstChildElement("method_D");
     if(xMethod_D) {
          if(string(xMethod_D->Attribute("enabled"))=="true")
          {
               _isMethodD = true;
               Log->Write("INFO: \tMethod D is selected" );
               _isOutputGraph =  (string(xMethod_D->Attribute("outputGraph")) == "true");
               if(_isOutputGraph)
               {
                    Log->Write("INFO: \tVoronoi graph is asked to output" );
               }
               _isIndividualFD = (string(xMethod_D->Attribute("individualFDdata")) == "true");
               if(_isIndividualFD)
               {
                    Log->Write("INFO: \tIndividual fundamental diagram data will be calculated" );
               }
               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_D")->FirstChildElement("measurementArea");
                         xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurementArea"))
               {
                    _areaIDforMethodD.push_back(xmltoi(xMeasurementArea->Attribute("id")));
                    Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", xmltoi(xMeasurementArea->Attribute("id")));
               }
               if ( xMethod_D->FirstChildElement("cutByCircle"))
               {
                    if ( string(xMethod_D->FirstChildElement("cutByCircle")->Attribute("enabled"))=="true")
                    {
                         _isCutByCircle=true;
                         _cutRadius=xmltof(xMethod_D->FirstChildElement("cutByCircle")->Attribute("radius"))*M2CM;
                         _circleEdges=xmltoi(xMethod_D->FirstChildElement("cutByCircle")->Attribute("edges"));
                         Log->Write("INFO: \tEach Voronoi cell will be cut by a circle with the radius of < %f > m!!", _cutRadius*CMtoM);
                         Log->Write("INFO: \tThe circle is discretized to a polygon with < %d> edges!!", _circleEdges);
                    }
               }

               if ( xMethod_D->FirstChildElement("steadyState"))
               {
                    _steadyStart =xmltof(xMethod_D->FirstChildElement("steadyState")->Attribute("start"));
                    _steadyEnd =xmltof(xMethod_D->FirstChildElement("steadyState")->Attribute("end"));
                    Log->Write("INFO: \tthe steady state is from  <%f> to <%f> frames", _steadyStart, _steadyEnd);
               }

               if(xMethod_D->FirstChildElement("getProfile"))
                    if ( string(xMethod_D->FirstChildElement("getProfile")->Attribute("enabled"))=="true")
                    {
                         _isGetProfile = true;
                         _scaleX =xmltof(xMethod_D->FirstChildElement("getProfile")->Attribute("scale_x"))*M2CM;
                         _scaleY =xmltof(xMethod_D->FirstChildElement("getProfile")->Attribute("scale_y"))*M2CM;
                         Log->Write("INFO: \tProfiles will be calculated" );
                         Log->Write("INFO: \tThe scale of the discretized cell in x, y direction are: < %f >m and < %f >m ",_scaleX*CMtoM, _scaleY*CMtoM);
                    }
          }
     }
     Log->Write("INFO: \tFinish parsing inifile");
     return true;
}


const string& ArgumentParser::GetErrorLogFile() const
{
     return _errorLogFile;
}

int ArgumentParser::GetLog() const
{
     return _log;
}

const string& ArgumentParser::GetGeometryFilename() const
{
     return _geometryFileName;
}

const FileFormat& ArgumentParser::GetFileFormat() const
{
     return _fileFormat;
}

const string& ArgumentParser::GetTrajectoriesLocation() const
{
     return _trajectoriesLocation;
}

const string& ArgumentParser::GetTrajectoriesFilename() const
{
     return _trajectoriesFilename;
}

char	ArgumentParser::GetVComponent() const
{
     return _vComponent;
}

int ArgumentParser::GetDelatT_Vins() const
{
     return _delatTVInst;
}


bool ArgumentParser::GetIsMethodA() const
{
     return _isMethodA;
}

int ArgumentParser::GetTimeIntervalA() const
{
     return _timeIntervalA;
}

bool ArgumentParser::GetIsMethodB() const
{
     return _isMethodB;
}

bool ArgumentParser::GetIsMethodC() const
{
     return _isMethodC;
}

bool ArgumentParser::GetIsMethodD() const
{
     return _isMethodD;
}

bool ArgumentParser::GetIsCutByCircle() const
{
     return _isCutByCircle;
}

double ArgumentParser::GetCutRadius() const
{
     return _cutRadius;
}

int ArgumentParser::GetCircleEdges() const
{
     return _circleEdges;
}
bool ArgumentParser::GetIsOutputGraph() const
{
     return _isOutputGraph;
}

bool ArgumentParser::GetIsIndividualFD() const
{
     return _isIndividualFD;
}

bool ArgumentParser::GetIsGetProfile() const
{
     return _isGetProfile;
}

double ArgumentParser::GetSteadyStart() const
{
     return _steadyStart;
}

double ArgumentParser::GetSteadyEnd() const
{
     return _steadyEnd;
}


float ArgumentParser::GetScaleX() const
{
     return _scaleX;
}

float ArgumentParser::GetScaleY() const
{
     return _scaleY;
}

vector<int> ArgumentParser::GetAreaIDforMethodA() const
{
     return _areaIDforMethodA;
}

vector<int> ArgumentParser::GetAreaIDforMethodB() const
{
     return _areaIDforMethodB;
}

vector<int> ArgumentParser::GetAreaIDforMethodC() const
{
     return _areaIDforMethodC;
}

vector<int> ArgumentParser::GetAreaIDforMethodD() const
{
     return _areaIDforMethodD;
}

MeasurementArea* ArgumentParser::GetMeasurementArea(int id)
{
     if (_measurementAreas.count(id) == 0)
          return NULL;
     return _measurementAreas[id];

}
