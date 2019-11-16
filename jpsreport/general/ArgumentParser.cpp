/**
 * \file        ArgumentParser.cpp
 * \date        Oct 10, 2014
 * \version     v0.8.3
 * \copyright   <2009-2018> Forschungszentrum Jülich GmbH. All rights reserved.
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



#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <math.h>
#include <ctime>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"
#include "../Analysis.h"
#include <boost/range/iterator_range.hpp>

using namespace std;

/* https://stackoverflow.com/questions/38530981/output-compiler-version-in-a-c-program#38531037 */
std::string ver_string(int a, int b, int c) {
      std::ostringstream ss;
      ss << a << '.' << b << '.' << c;
      return ss.str();
}
//https://sourceforge.net/p/predef/wiki/Compilers/
std::string true_cxx =
#ifdef __clang__
      "clang++";
#elif defined(__GNUC__)
"g++";
#elif defined(__MINGW32__)
   "MinGW";
#elif defined(_MSC_VER)
  "Visual Studio";
#else
"Compiler not identified";
#endif

std::string true_cxx_ver =
#ifdef __clang__
    ver_string(__clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
    ver_string(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(__MINGW32__)
ver_string(__MINGW32__, __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION);
#elif defined( _MSC_VER)
    ver_string(_MSC_VER, _MSC_FULL_VER,_MSC_BUILD);
#else
"";
#endif

void Logs()
{
     time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
     std::ostringstream oss;
     char foo[100];
     if(0 < std::strftime(foo, sizeof(foo), "%a %b %d %X %Y", std::localtime(&now)))
          oss << foo;
     else
          oss << "No time!";
     // else  // hack for g++ < 5
     //      oss << std::put_time(std::localtime(&now), "%a %b %d %X %Y");
    auto currentTime = oss.str();

     // first logs will go to stdout
     Log->Write("----\nJuPedSim - JPSreport\n");
     Log->Write("Current date   : %s", currentTime.c_str());
     // Log->Write("Version        : %s", JPSREPORT_VERSION);
     Log->Write("Compiler       : %s (%s)", true_cxx.c_str(), true_cxx_ver.c_str());
     // Log->Write("Commit hash    : %s", GIT_COMMIT_HASH);
     // Log->Write("Commit date    : %s", GIT_COMMIT_DATE);
     // Log->Write("Branch         : %s", GIT_BRANCH);
     Log->Write("Python         : %s (%s)\n----\n", PYTHON, PYTHON_VERSION);
}

void ArgumentParser::Usage(const std::string file)
{

     Log->Write("Usage: \n");
     Log->Write("%s inifile.xml\n",file.c_str());
     exit(EXIT_SUCCESS);
}

ArgumentParser::ArgumentParser()
{
     // Default parameter values
     _geometryFileName = "geo.xml";

     _vComponent = "B";
     _IgnoreBackwardMovement=false;
     _isMethodA = false;
     _delatTVInst = 5;
     _isMethodB = false;
     _isMethodC =false;
     _isMethodD = false;
     _isMethodI= false;
     _isMethodJ = false;
     _isCutByCircle = false;
     _isOneDimensional=false;
     _isGetProfile =false;
     _steadyStart =100;
     _steadyEnd = 1000;
     _grid_size_X = 10;
     _grid_size_Y = 10;
     _errorLogFile="log.txt";
     _log=2; //no output wanted
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
     }
     if (argument == "-v" || argument == "--version")
     {
          Logs();
          exit(EXIT_SUCCESS);
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

const vector<fs::path>& ArgumentParser::GetTrajectoriesFiles() const
{
     return _trajectoriesFiles;
}

const fs::path& ArgumentParser::GetProjectRootDir() const
{
     return _projectRootDir;
}


bool ArgumentParser::ParseIniFile(const string& inifile)
{
     Logs();
     Log->Write("INFO: \tParsing the ini file <%s>",inifile.c_str());
     //extract and set the project root dir
     fs::path p(inifile);
     _projectRootDir = weakly_canonical(p).parent_path();
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
     if (xMainNode->FirstChild("logfile")) {
          fs::path logfile(xMainNode->FirstChild("logfile")->FirstChild()->Value());
          logfile =  GetProjectRootDir() / logfile;
          this->SetErrorLogFile(logfile);
          this->SetLog(2);
          Log->Write("INFO:\tlogfile <%s>", GetErrorLogFile().string().c_str());
     }
     switch (this->GetLog()) {
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
          sprintf(name,"%s", GetErrorLogFile().string().c_str());
          if(Log) delete Log;
          Log = new FileHandler(name);
     }
          break;
     default:
          Log->Write("ERROR: \tWrong option for Log file!");
          exit(0);
     }
     // from this point if case 2, the logs will go to a logfile
     if(this->GetLog() == 2)
     {
           Logs();
     }
     //geometry
     if(xMainNode->FirstChild("geometry"))
     {
          fs::path pathGeo(xMainNode->FirstChildElement("geometry")->Attribute("file"));
           _geometryFileName = GetProjectRootDir() / pathGeo;
           if(!fs::exists(_geometryFileName)){
                Log->Write("ERROR: \tGeometry File <%s> does not exist",  _geometryFileName.string().c_str());
                return false;
           }
           _geometryFileName = fs::canonical(_geometryFileName);
           Log->Write("INFO: \tGeometry File is: <%s>",  _geometryFileName.string().c_str());
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
               _trajectoriesFilename = fs::path(xFile->Attribute("name"));
               _trajectoriesFiles.push_back(_trajectoriesFilename);

               //check if the given file match the format
               if(boost::algorithm::ends_with(_trajectoriesFilename.string(),fmt))
               {
                     Log->Write("INFO: \tInput trajectory file is <%s>", _trajectoriesFilename.string().c_str());
               }
               else
               {
                     Log->Write("ERROR: \tWrong file extension\t<%s> for file <%s>",fmt.c_str(),_trajectoriesFilename.string().c_str());
                    return false;
               }
          }
          auto xmlpath = xTrajectories->FirstChildElement("path");
          if (xmlpath)
          {
               if(xmlpath->Attribute("location"))
               {
                    _trajectoriesLocation = GetProjectRootDir() / fs::path(xmlpath->Attribute("location"));
                     // _trajectoriesLocation = canonical(_trajectoriesLocation);
               }
          }
          else
          {
               fs::path path_root(GetProjectRootDir());
                path_root = canonical(path_root);
                _trajectoriesLocation=path_root.string();
          }
          Log->Write("INFO: \tInput directory for loading trajectory is <%s>", _trajectoriesLocation.string().c_str());

          // in the case no file was specified, collect all files in the specified directory
          if(_trajectoriesFiles.empty())
          {
               if(exists(_trajectoriesLocation))
               {
                    /* print all the files and directories within directory */
                    fs::path path_traj(GetTrajectoriesLocation());
                    path_traj = canonical(path_traj);
                    for (auto& filename : boost::make_iterator_range(fs::directory_iterator(path_traj), {}))
                    {
                         string s = filename.path().string();
                         int pos = s.find_last_of('/');
                         pos = pos == -1 ? int(s.find_last_of('\\')) : pos;
                         s = pos == -1 ? s : s.substr(pos + 1);
                         if (boost::algorithm::ends_with(s, fmt))
                         {
                              _trajectoriesFiles.push_back(s);
                              Log->Write("INFO: \tInput trajectory file is <%s>", s.c_str());
                         }
                    }
               }
               else
               {
                    /* could not open directory */
                     Log->Write("ERROR: \tcould not open the directory <%s>", _trajectoriesLocation.string().c_str());
                    return false;
               }
          }

     }

     //max CPU
     if(xMainNode->FirstChild("num_threads"))
     {
          TiXmlNode* numthreads = xMainNode->FirstChild("num_threads")->FirstChild();
          if(numthreads)
          {
#ifdef _OPENMP
               omp_set_num_threads(xmltoi(numthreads->Value(),omp_get_max_threads()));
#endif

          }
          Log->Write("INFO: \t Using <%d> threads", omp_get_max_threads());
     }

     // output directory
     _outputDir = GetProjectRootDir() / "Output";
     if(xMainNode->FirstChild("output"))
     {
          string tmp  = xMainNode->FirstChildElement("output")->Attribute("location");
          fs::path tmpPath(tmp);
          _outputDir = tmpPath;
          if(tmp.empty())
          {
               _outputDir = GetProjectRootDir() / "Output";
          }
          if (! _outputDir.is_absolute())
          {
               _outputDir = _projectRootDir / _outputDir;
          }
     }
     else
          Log->Write("INFO: \tDefault output directory");
     if (!exists(_outputDir))
     {
          // does not exist yet. mkdir
          bool res = fs::create_directory(_outputDir);
          if (res == false)
          {
               Log->Write("ERROR: \tcould not create the directory <"+_outputDir.string()+">");
               return false;
          }
          else
               Log->Write("INFO: \tcreated directory <"+_outputDir.string()+">");
     }
     Log->Write("INFO: \tOutput directory for results is:\t<"+_outputDir.string()+">");

     //measurement area
     if(xMainNode->FirstChild("measurement_areas"))
     {
          string unit ="";
          if(xMainNode->FirstChildElement("measurement_areas")->Attribute("unit"))
               unit = xMainNode->FirstChildElement("measurement_areas")->Attribute("unit");
          if(unit!="m")
          {
               Log->Write("WARNING: \tonly <m> unit is supported. Convert your units.");
               return false;
          }

          for(TiXmlNode* xMeasurementArea_B=xMainNode->FirstChild("measurement_areas")->FirstChild("area_B");
              xMeasurementArea_B; xMeasurementArea_B=xMeasurementArea_B->NextSibling("area_B"))
          {
               MeasurementArea_B* areaB = new MeasurementArea_B();
               areaB->_id=xmltoi(xMeasurementArea_B->ToElement()->Attribute("id"));
               areaB->_type=xMeasurementArea_B->ToElement()->Attribute("type");
               if(xMeasurementArea_B->ToElement()->Attribute("zPos"))
               {
                    if(string(xMeasurementArea_B->ToElement()->Attribute("zPos"))!="None")
                    {
                         areaB->_zPos=xmltof(xMeasurementArea_B->ToElement()->Attribute("zPos"));
                    }
                    else
                    {
                         areaB->_zPos=10000001.0;
                    }
               }
               else
               {
                    areaB->_zPos=10000001.0;
               }
               std::map<int, polygon_2d> geoPoly;
               polygon_2d poly;
               Log->Write("INFO: \tMeasure area id  <%d> with type <%s>",areaB->_id, areaB->_type.c_str());
               int num_verteces = 0;
               for(TiXmlElement* xVertex=xMeasurementArea_B->FirstChildElement("vertex"); xVertex; xVertex=xVertex->NextSiblingElement("vertex") )
               {
                    double box_px = xmltof(xVertex->Attribute("x"))*M2CM;
                    double box_py = xmltof(xVertex->Attribute("y"))*M2CM;
                    boost::geometry::append(poly, boost::geometry::make<point_2d>(box_px, box_py));
                    Log->Write("\t\tMeasure area points  < %.3f, %.3f>",box_px*CMtoM,box_py*CMtoM);
                    num_verteces++;
               }
               if(num_verteces < 3 && num_verteces > 0)
                    Log->Write("\tWARNING: Less than 3 measure area points given (%d). At least 3 or nothing at all!!", num_verteces);
               if(num_verteces == 0) // big bounding box
               {
                    Log->Write("\tWARNING: NO measure area points given (%d). default BB!!", num_verteces);
                    // get bounding box
                    // loading geometry is done in  analysis.cpp
                    // so this is done twice, which is not nice.
                    // For big geometries it could be slow.
                    Building*  building  = new Building();
                    building->LoadGeometry(GetGeometryFilename().string());
                    building->InitGeometry();
                    building->AddSurroundingRoom(); // this is a big reactagle
                    // slightly bigger than the
                    // geometry boundaries
                    double geo_minX = building->_xMin;
                    double geo_minY = building->_yMin;
                    double geo_maxX = building->_xMax;
                    double geo_maxY = building->_yMax;
                    Log->Write("INFO: \tBounding box:\n \t\tminX = %.2f\n \t\tmaxX = %.2f \n \t\tminY = %.2f \n\t\tmaxY = %.2f", geo_minX, geo_maxX, geo_minY, geo_maxY);

                    //1
                    double box_px = geo_minX*M2CM;
                    double box_py = geo_minY*M2CM;
                    boost::geometry::append(poly, boost::geometry::make<point_2d>(box_px, box_py));
                    //2
                    box_px = geo_minX*M2CM;
                    box_py = geo_maxY*M2CM;
                    boost::geometry::append(poly, boost::geometry::make<point_2d>(box_px, box_py));
                    //3
                    box_px = geo_maxX*M2CM;
                    box_py = geo_maxY*M2CM;
                    boost::geometry::append(poly, boost::geometry::make<point_2d>(box_px, box_py));
                    //4
                    box_px = geo_maxX*M2CM;
                    box_py = geo_minY*M2CM;
                    boost::geometry::append(poly, boost::geometry::make<point_2d>(box_px, box_py));
               }
               correct(poly); // in the case the Polygone is not closed
               areaB->_poly=poly;

               TiXmlElement* xLength=xMeasurementArea_B->FirstChildElement("length_in_movement_direction");
               if(xLength)
               {
                    areaB->_length=xmltof(xLength->Attribute("distance"));
                    Log->Write("\t\tLength in movement direction %.3f",areaB->_length);
               }
               _measurementAreas[areaB->_id]=areaB;
          }
          for(TiXmlNode* xMeasurementArea_L=xMainNode->FirstChild("measurement_areas")->FirstChild("area_L");
              xMeasurementArea_L; xMeasurementArea_L=xMeasurementArea_L->NextSibling("area_L"))
          {
               MeasurementArea_L* areaL = new MeasurementArea_L();
               areaL->_id=xmltoi(xMeasurementArea_L->ToElement()->Attribute("id"));
               areaL->_type=xMeasurementArea_L->ToElement()->Attribute("type");
               if(xMeasurementArea_L->ToElement()->Attribute("zPos"))
               {
                    if(string(xMeasurementArea_L->ToElement()->Attribute("zPos"))!="None")
                    {
                         areaL->_zPos=xmltof(xMeasurementArea_L->ToElement()->Attribute("zPos"));
                    }
                    else
                    {
                         areaL->_zPos=10000001.0;
                    }
               }
               else
               {
                    areaL->_zPos=10000001.0;
               }
               Log->Write("INFO: \tMeasure area id  <%d> with type <%s>",areaL->_id,areaL->_type.c_str());
               areaL->_lineStartX = xmltof(xMeasurementArea_L->FirstChildElement("start")->Attribute("x"))*M2CM;
               areaL->_lineStartY =xmltof(xMeasurementArea_L->FirstChildElement("start")->Attribute("y"))*M2CM;
               areaL->_lineEndX = xmltof(xMeasurementArea_L->FirstChildElement("end")->Attribute("x"))*M2CM;
               areaL->_lineEndY =xmltof(xMeasurementArea_L->FirstChildElement("end")->Attribute("y"))*M2CM;

               _measurementAreas[areaL->_id]=areaL;
               Log->Write("\t\tMeasurement line starts from  <%.3f, %.3f> to <%.3f, %.3f>",areaL->_lineStartX*CMtoM,areaL->_lineStartY*CMtoM,areaL->_lineEndX*CMtoM,areaL->_lineEndY*CMtoM);
          }
     }
     //instantaneous velocity
     TiXmlNode* xVelocity=xMainNode->FirstChild("velocity");
     if(xVelocity)
     {
          string FrameSteps = "10";
          if(xMainNode->FirstChildElement("velocity")->Attribute("frame_step"))
          {
               FrameSteps =xMainNode->FirstChildElement("velocity")->Attribute("frame_step");
               _delatTVInst = atof(FrameSteps.c_str())/2.0;
          }
          string MovementDirection = "None";
          if(xMainNode->FirstChildElement("velocity")->Attribute("set_movement_direction"))
          {
               MovementDirection = xMainNode->FirstChildElement("velocity")->Attribute("set_movement_direction");
               if(atof(MovementDirection.c_str())<0 && atof(MovementDirection.c_str())>360 && MovementDirection!="None" && MovementDirection!="SeeTraj")
               {
                    Log->Write("WARNING: \tThe movement direction should be set between 0 to 360 or None!");
                    return false;
               }
          }
          if	(xMainNode->FirstChildElement("velocity")->Attribute("ignore_backward_movement"))
          {
               if	(string(xMainNode->FirstChildElement("velocity")->Attribute("ignore_backward_movement"))=="true")
               {
                    _IgnoreBackwardMovement = true;
               }
               else
               {
                    _IgnoreBackwardMovement = false;
               }
          }
          if(MovementDirection=="None")
          {
               _vComponent = "B";  // both components
               _IgnoreBackwardMovement = false;
               Log->Write("INFO: \tBoth x and y-component of coordinates will be used to calculate instantaneous velocity over <"+FrameSteps+" frames>" );
          }
          else if(MovementDirection=="SeeTraj")
          {
               _vComponent = "F";
               Log->Write("INFO: \tThe component defined in the trajectory file will be used to calculate instantaneous velocity over <" + FrameSteps + " frames>");
          }
          else
          {
               _vComponent = MovementDirection;
               Log->Write("INFO: \tThe instantaneous velocity in the direction of <"+MovementDirection+">  will be calculated over <"+FrameSteps+" frames>" );
          }
     }
     // Method A
     TiXmlElement* xMethod_A=xMainNode->FirstChildElement("method_A");
     if(xMethod_A)
     {
          if(string(xMethod_A->Attribute("enabled"))=="true")
          {
               _isMethodA = true;
               Log->Write("INFO: \tMethod A is selected" );
/*               _timeIntervalA = xmltoi(xMethod_A->FirstChildElement("frame_interval")->GetText());
                 Log->Write("INFO: \tFrame interval used for calculating flow in Method A is <%d> frame",_timeIntervalA);*/
               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_A")->FirstChildElement("measurement_area");
                   xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurement_area"))
               {
                    int id = xmltoi(xMeasurementArea->Attribute("id"));

                    if( _measurementAreas[id]->_type == "Line")
                    {
                         _areaIDforMethodA.push_back(id);
                         Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", id);
                    }
                    else
                    {
                         Log->Write("WARNING: \tMeasurement area id <%d> will NOT be used for analysis (Type <%s> is not Line)", id, _measurementAreas[id]->_type.c_str());
                    }

                    if(xMeasurementArea->Attribute("frame_interval"))
                    {
                         if(string(xMeasurementArea->Attribute("frame_interval"))!="None")
                         {
                              _timeIntervalA.push_back(xmltoi(xMeasurementArea->Attribute("frame_interval")));
                              Log->Write("\tFrame interval used for calculating flow is <%d> frame",xmltoi(xMeasurementArea->Attribute("frame_interval")));
                         }
                         else
                         {
                              _timeIntervalA.push_back(100);
                         }
                    }
                    else
                    {
                         _timeIntervalA.push_back(100);
                    }
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
               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_B")->FirstChildElement("measurement_area");
                   xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurement_area"))
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
               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_C")->FirstChildElement("measurement_area");
                   xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurement_area"))
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

               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_D")->FirstChildElement("measurement_area");
                   xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurement_area"))
               {
                    _areaIDforMethodD.push_back(xmltoi(xMeasurementArea->Attribute("id")));
                    Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", xmltoi(xMeasurementArea->Attribute("id")));
                    if(xMeasurementArea->Attribute("start_frame"))
                    {
                         if(string(xMeasurementArea->Attribute("start_frame"))!="None")
                         {
                              _start_frames_MethodD.push_back(xmltoi(xMeasurementArea->Attribute("start_frame")));
                              Log->Write("\tthe analysis starts from frame <%d>",xmltoi(xMeasurementArea->Attribute("start_frame")));
                         }
                         else
                         {
                              _start_frames_MethodD.push_back(-1);
                         }
                    }
                    else
                    {
                         _start_frames_MethodD.push_back(-1);
                    }
                    if(xMeasurementArea->Attribute("stop_frame"))
                    {
                         if(string(xMeasurementArea->Attribute("stop_frame"))!="None")
                         {
                              _stop_frames_MethodD.push_back(xmltoi(xMeasurementArea->Attribute("stop_frame")));
                              Log->Write("\tthe analysis stops from frame <%d>", xmltoi(xMeasurementArea->Attribute("stop_frame")));
                         }
                         else
                         {
                              _stop_frames_MethodD.push_back(-1);
                         }
                    }
                    else
                    {
                         _stop_frames_MethodD.push_back(-1);
                    }

                    if(xMeasurementArea->Attribute("get_individual_FD"))
                    {
                         if(string(xMeasurementArea->Attribute("get_individual_FD"))=="true")
                         {
                              _individual_FD_flags.push_back(true);
                              Log->Write("INFO: \tIndividual FD will be output");
                         }
                         else
                         {
                              _individual_FD_flags.push_back(false);
                         }
                    }
                    else
                    {
                         _individual_FD_flags.push_back(false);
                    }
               }
               if (xMethod_D->FirstChildElement("one_dimensional"))
               {
                    if ( string(xMethod_D->FirstChildElement("one_dimensional")->Attribute("enabled"))=="true")
                    {
                         _isOneDimensional=true;
                         Log->Write("INFO: \tThe data will be analyzed with one dimensional way!!");
                    }
               }

               if ( xMethod_D->FirstChildElement("cut_by_circle"))
               {
                    if ( string(xMethod_D->FirstChildElement("cut_by_circle")->Attribute("enabled"))=="true")
                    {
                         _isCutByCircle=true;
                         _cutRadius=xmltof(xMethod_D->FirstChildElement("cut_by_circle")->Attribute("radius"))*M2CM;
                         _circleEdges=xmltoi(xMethod_D->FirstChildElement("cut_by_circle")->Attribute("edges"));
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

               if(xMethod_D->FirstChildElement("profiles"))
               {
                    if ( string(xMethod_D->FirstChildElement("profiles")->Attribute("enabled"))=="true")
                    {
                         _isGetProfile = true;
                         _grid_size_X =xmltof(xMethod_D->FirstChildElement("profiles")->Attribute("grid_size_x"))*M2CM;
                         _grid_size_Y =xmltof(xMethod_D->FirstChildElement("profiles")->Attribute("grid_size_y"))*M2CM;
                         Log->Write("INFO: \tProfiles will be calculated" );
                         Log->Write("INFO: \tThe discretized grid size in x, y direction is: < %f >m by < %f >m ",_grid_size_X*CMtoM, _grid_size_Y*CMtoM);
                    }
               }
          }
     }
     // method I
     TiXmlElement* xMethod_I=xMainNode->FirstChildElement("method_I");
     if(xMethod_I) {
          if(string(xMethod_I->Attribute("enabled"))=="true")
          {
               _isMethodI = true;
               Log->Write("INFO: \tMethod I is selected" );

               for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_I")->FirstChildElement("measurement_area");
                   xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurement_area"))
               {
                    _areaIDforMethodI.push_back(xmltoi(xMeasurementArea->Attribute("id")));
                    Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", xmltoi(xMeasurementArea->Attribute("id")));
                    if(xMeasurementArea->Attribute("start_frame"))
                    {
                         if(string(xMeasurementArea->Attribute("start_frame"))!="None")
                         {
                              _start_frames_MethodD.push_back(xmltoi(xMeasurementArea->Attribute("start_frame")));
                              Log->Write("\tthe analysis starts from frame <%d>",xmltoi(xMeasurementArea->Attribute("start_frame")));
                         }
                         else
                         {
                              _start_frames_MethodI.push_back(-1);
                         }
                    }
                    else
                    {
                         _start_frames_MethodI.push_back(-1);
                    }
                    if(xMeasurementArea->Attribute("stop_frame"))
                    {
                         if(string(xMeasurementArea->Attribute("stop_frame"))!="None")
                         {
                              _stop_frames_MethodI.push_back(xmltoi(xMeasurementArea->Attribute("stop_frame")));
                              Log->Write("\tthe analysis stops from frame <%d>", xmltoi(xMeasurementArea->Attribute("stop_frame")));
                         }
                         else
                         {
                              _stop_frames_MethodI.push_back(-1);
                         }
                    }
                    else
                    {
                         _stop_frames_MethodI.push_back(-1);
                    }

                    if(xMeasurementArea->Attribute("get_individual_FD"))
                    {
                         if(string(xMeasurementArea->Attribute("get_individual_FD"))=="true")
                         {
                              _individual_FD_flags.push_back(true);
                              Log->Write("INFO: \tIndividual FD will be output");
                         }
                         else
                         {
                              _individual_FD_flags.push_back(false);
                         }
                    }
                    else
                    {
                         _individual_FD_flags.push_back(false);
                    }
               }
               if (xMethod_I->FirstChildElement("one_dimensional"))
               {
                    if ( string(xMethod_I->FirstChildElement("one_dimensional")->Attribute("enabled"))=="true")
                    {
                         _isOneDimensional=true;
                         Log->Write("INFO: \tThe data will be analyzed with one dimensional way!!");
                    }
               }

               if ( xMethod_I->FirstChildElement("cut_by_circle"))
               {
                    if ( string(xMethod_I->FirstChildElement("cut_by_circle")->Attribute("enabled"))=="true")
                    {
                         _isCutByCircle=true;
                         _cutRadius=xmltof(xMethod_I->FirstChildElement("cut_by_circle")->Attribute("radius"))*M2CM;
                         _circleEdges=xmltoi(xMethod_I->FirstChildElement("cut_by_circle")->Attribute("edges"));
                         Log->Write("INFO: \tEach Voronoi cell will be cut by a circle with the radius of < %f > m!!", _cutRadius*CMtoM);
                         Log->Write("INFO: \tThe circle is discretized to a polygon with < %d> edges!!", _circleEdges);
                    }
               }

               if ( xMethod_I->FirstChildElement("steadyState"))
               {
                    _steadyStart =xmltof(xMethod_I->FirstChildElement("steadyState")->Attribute("start"));
                    _steadyEnd =xmltof(xMethod_I->FirstChildElement("steadyState")->Attribute("end"));
                    Log->Write("INFO: \tthe steady state is from  <%f> to <%f> frames", _steadyStart, _steadyEnd);
               }

               if(xMethod_I->FirstChildElement("profiles"))
               {
                    if ( string(xMethod_I->FirstChildElement("profiles")->Attribute("enabled"))=="true")
                    {
                         _isGetProfile = true;
                         _grid_size_X =xmltof(xMethod_I->FirstChildElement("profiles")->Attribute("grid_size_x"))*M2CM;
                         _grid_size_Y =xmltof(xMethod_I->FirstChildElement("profiles")->Attribute("grid_size_y"))*M2CM;
                         Log->Write("INFO: \tProfiles will be calculated" );
                         Log->Write("INFO: \tThe discretized grid size in x, y direction is: < %f >m by < %f >m ",_grid_size_X*CMtoM, _grid_size_Y*CMtoM);
                    }
               }
          }
     }

    // method Voronoi
    TiXmlElement* xMethod_J=xMainNode->FirstChildElement("method_J");
    if(xMethod_J) {
      if(string(xMethod_J->Attribute("enabled"))=="true")
      {
        _isMethodJ = true;
        Log->Write("INFO: \tMethod Voronoi is selected" );

        for(TiXmlElement* xMeasurementArea=xMainNode->FirstChildElement("method_J")->FirstChildElement("measurement_area");
            xMeasurementArea; xMeasurementArea = xMeasurementArea->NextSiblingElement("measurement_area"))
        {
          _areaIDforMethodJ.push_back(xmltoi(xMeasurementArea->Attribute("id")));
          Log->Write("INFO: \tMeasurement area id <%d> will be used for analysis", xmltoi(xMeasurementArea->Attribute("id")));
          if(xMeasurementArea->Attribute("start_frame"))
          {
            if(string(xMeasurementArea->Attribute("start_frame"))!="None")
            {
              _start_frames_MethodJ.push_back(xmltoi(xMeasurementArea->Attribute("start_frame")));
              Log->Write("\tthe analysis starts from frame <%d>",xmltoi(xMeasurementArea->Attribute("start_frame")));
            }
            else
            {
              _start_frames_MethodJ.push_back(-1);
            }
          }
          else
          {
            _start_frames_MethodJ.push_back(-1);
          }
          if(xMeasurementArea->Attribute("stop_frame"))
          {
            if(string(xMeasurementArea->Attribute("stop_frame"))!="None")
            {
              _stop_frames_MethodJ.push_back(xmltoi(xMeasurementArea->Attribute("stop_frame")));
              Log->Write("\tthe analysis stops from frame <%d>", xmltoi(xMeasurementArea->Attribute("stop_frame")));
            }
            else
            {
              _stop_frames_MethodJ.push_back(-1);
            }
          }
          else
          {
            _stop_frames_MethodJ.push_back(-1);
          }

          if(xMeasurementArea->Attribute("get_individual_FD"))
          {
            if(string(xMeasurementArea->Attribute("get_individual_FD"))=="true")
            {
              _individual_FD_flags.push_back(true);
              Log->Write("INFO: \tIndividual FD will be output");
            }
            else
            {
              _individual_FD_flags.push_back(false);
            }
          }
          else
          {
            _individual_FD_flags.push_back(false);
          }
        }
        if (xMethod_J->FirstChildElement("one_dimensional"))
        {
          if ( string(xMethod_J->FirstChildElement("one_dimensional")->Attribute("enabled"))=="true")
          {
            _isOneDimensional=true;
            Log->Write("INFO: \tThe data will be analyzed with one dimensional way!!");
          }
        }

        if ( xMethod_J->FirstChildElement("cut_by_circle"))
        {
          if ( string(xMethod_J->FirstChildElement("cut_by_circle")->Attribute("enabled"))=="true")
          {
            _isCutByCircle=true;
            _cutRadius=xmltof(xMethod_J->FirstChildElement("cut_by_circle")->Attribute("radius"))*M2CM;
            _circleEdges=xmltoi(xMethod_J->FirstChildElement("cut_by_circle")->Attribute("edges"));
            Log->Write("INFO: \tEach Voronoi cell will be cut by a circle with the radius of < %f > m!!", _cutRadius*CMtoM);
            Log->Write("INFO: \tThe circle is discretized to a polygon with < %d> edges!!", _circleEdges);
          }
        }

        if ( xMethod_J->FirstChildElement("steadyState"))
        {
          _steadyStart =xmltof(xMethod_J->FirstChildElement("steadyState")->Attribute("start"));
          _steadyEnd =xmltof(xMethod_J->FirstChildElement("steadyState")->Attribute("end"));
          Log->Write("INFO: \tthe steady state is from  <%f> to <%f> frames", _steadyStart, _steadyEnd);
        }

        if(xMethod_J->FirstChildElement("profiles"))
        {
          if ( string(xMethod_J->FirstChildElement("profiles")->Attribute("enabled"))=="true")
          {
            _isGetProfile = true;
            _grid_size_X =xmltof(xMethod_J->FirstChildElement("profiles")->Attribute("grid_size_x"))*M2CM;
            _grid_size_Y =xmltof(xMethod_J->FirstChildElement("profiles")->Attribute("grid_size_y"))*M2CM;
            Log->Write("INFO: \tProfiles will be calculated" );
            Log->Write("INFO: \tThe discretized grid size in x, y direction is: < %f >m by < %f >m ",_grid_size_X*CMtoM, _grid_size_Y*CMtoM);
          }
        }
      }
    }

     Log->Write("INFO: \tFinish parsing inifile");
     if(!(_isMethodA || _isMethodB || _isMethodC || _isMethodD ||  _isMethodI ||  _isMethodJ))
     {
          Log->Write("WARNING: No measurement method enabled. Nothing to do.");
          exit(EXIT_SUCCESS);
     }
     return true;
}


const fs::path& ArgumentParser::GetErrorLogFile() const
{
     return _errorLogFile;
}

int ArgumentParser::GetLog() const
{
     return _log;
}

const fs::path& ArgumentParser::GetGeometryFilename() const
{
     return _geometryFileName;
}

const FileFormat& ArgumentParser::GetFileFormat() const
{
     return _fileFormat;
}

const fs::path& ArgumentParser::GetTrajectoriesLocation() const
{
     return _trajectoriesLocation;
}

const fs::path& ArgumentParser::GetOutputLocation() const
{
     return _outputDir;
}

const fs::path& ArgumentParser::GetTrajectoriesFilename() const
{
     return _trajectoriesFilename;
}

std::string	ArgumentParser::GetVComponent() const
{
     return _vComponent;
}

bool ArgumentParser::GetIgnoreBackwardMovement() const
{
     return _IgnoreBackwardMovement;
}

int ArgumentParser::GetDelatT_Vins() const
{
     return _delatTVInst;
}


bool ArgumentParser::GetIsMethodA() const
{
     return _isMethodA;
}

vector<int> ArgumentParser::GetTimeIntervalA() const
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
bool ArgumentParser::GetIsMethodI() const
{
     return _isMethodI;
}

bool ArgumentParser::GetIsMethodJ() const
{
  return _isMethodJ;
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

bool ArgumentParser::GetIsOneDimensional() const
{
     return _isOneDimensional;
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


float ArgumentParser::GetGridSizeX() const
{
     return _grid_size_X;
}

float ArgumentParser::GetGridSizeY() const
{
     return _grid_size_Y;
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

vector<int> ArgumentParser::GetAreaIDforMethodI() const
{
     return _areaIDforMethodI;
}

vector<int> ArgumentParser::GetAreaIDforMethodJ() const
{
  return _areaIDforMethodJ;
}

vector<int> ArgumentParser::GetStartFramesMethodD() const
{
     return _start_frames_MethodD;
}

vector<int> ArgumentParser::GetStartFramesMethodI() const
{
     return _start_frames_MethodI;
}

vector<int> ArgumentParser::GetStartFramesMethodJ() const
{
  return _start_frames_MethodJ;
}

vector<int> ArgumentParser::GetStopFramesMethodD() const
{
     return _stop_frames_MethodD;
}

vector<int> ArgumentParser::GetStopFramesMethodI() const
{
     return _stop_frames_MethodI;
}

vector<int> ArgumentParser::GetStopFramesMethodJ() const
{
  return _stop_frames_MethodJ;
}


vector<bool> ArgumentParser::GetIndividualFDFlags() const
{
     return _individual_FD_flags;
}

MeasurementArea* ArgumentParser::GetMeasurementArea(int id)
{
     if (_measurementAreas.count(id) == 0)
     {
          Log->Write("ERROR:\t measurement id [%d] not found.",id);
          Log->Write("      \t check your configuration files");
          exit(EXIT_FAILURE);
          //return NULL;
     }
     return _measurementAreas[id];

}

void ArgumentParser::SetErrorLogFile(fs::path errorLogFile)
{
      _errorLogFile = errorLogFile;
};

void ArgumentParser::SetLog(int log) {
     _log = log;
};
