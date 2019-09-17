/**
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
 **/
//
// Created by laemmel on 24.03.16.
//
#include "IniFileParser.h"

#include "OutputHandler.h"

#include "general/Filesystem.h"
#include "general/OpenMP.h"
#include "math/GCFMModel.h"
#include "math/GompertzModel.h"
#include "math/GradientModel.h"
#include "math/KrauszModel.h"
#include "math/VelocityModel.h"
#include "pedestrian/Pedestrian.h"
#include "router/ai_router/AIRouter.h"
#include "router/ff_router/ffRouter.h"
#include "router/global_shortest/GlobalRouter.h"
#include "router/quickest/QuickestPathRouter.h"
#include "router/smoke_router/SmokeRouter.h"
#include "direction/DirectionManager.h"
#include "direction/walking/DirectionStrategy.h"
#include "direction/walking/DirectionFloorfield.h"
#include "direction/walking/DirectionGeneral.h"
#include "direction/walking/DirectionInRangeBottleneck.h"
#include "direction/walking/DirectionLocalFloorfield.h"
#include "direction/walking/DirectionMiddlePoint.h"
#include "direction/walking/DirectionMinSeperationShorterLine.h"
#include "direction/walking/DirectionSubLocalFloorfield.h"
#include "direction/walking/DirectionTrain.h"
#include "direction/waiting/WaitingStrategy.h"
#include "direction/waiting/WaitingMiddle.h"
#include "direction/waiting/WaitingRandom.h"

#include <tinyxml.h>


IniFileParser::IniFileParser(Configuration* config)
{
     _config = config;
}

bool IniFileParser::Parse(const fs::path& iniFile)
{
     Log->Write("INFO: \tLoading and parsing the project file <%s>",
               iniFile.string().c_str());
     _config->SetProjectFile(iniFile);//TODO in some locations it is called iniFile and in others project file,
     // and as I just realized, I called it configuration. We should be consistent here anything else
     // is confusing [gl march '16]
     _config->SetProjectRootDir(fs::absolute(iniFile.parent_path()));

     TiXmlDocument doc(iniFile.string());
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \tCould not parse the project file");
          return false;
     }

     // everything is fine. proceed with parsing

     TiXmlElement* xMainNode = doc.RootElement();
     if (!xMainNode) {
          Log->Write("ERROR:\tRoot element does not exist");
          return false;
     }

     if (xMainNode->ValueStr()!="JuPedSim") {
          Log->Write("ERROR:\tRoot element value is not 'JuPedSim'.");
          return false;
     }

     //check the header version
     if (!xMainNode->Attribute("version")) {
          Log->Write("WARNING:\t There is no header version. I am assuming %s",
                    JPS_VERSION);
     }
     else if (std::stod(xMainNode->Attribute("version"))<std::stod(JPS_OLD_VERSION)) {
          Log->Write("ERROR:\t Wrong header version. Only version greater than %s is supported.", JPS_OLD_VERSION);
          return false;
     }

     //check the structure of inifile
     if (xMainNode->FirstChild("header")) {
          TiXmlNode* xHeader = xMainNode->FirstChild("header");
          ParseHeader(xHeader);
     }//if header
     else {
          ParseHeader(xMainNode);
     }//else header




     // JPSfire
     // -------------------------------------
     // read walkingspeed
     #ifdef JPSFIRE
     std::shared_ptr<WalkingSpeed> W( new WalkingSpeed(iniFile.string()) );
     _config->SetWalkingSpeed(W);
     // read  ToxicityAnalysis
     std::shared_ptr<ToxicityAnalysis> T( new ToxicityAnalysis(iniFile.string(), _config->GetFps()));
     _config->SetToxicityAnalysis(T);
     #endif
     // -------------------------------------

     //pick up which model to use
     //get the wanted ped model id
     _model = xmltoi(xMainNode->FirstChildElement("agents")->Attribute("operational_model_id"), -1);
     if (_model==-1) {
          Log->Write("ERROR: \tmissing operational_model_id attribute in the agent section.");
          Log->Write("ERROR: \tplease specify the model id to use");
          return false;
     }

     bool parsingModelSuccessful = false;
     for (TiXmlElement* xModel = xMainNode->FirstChild("operational_models")->FirstChildElement("model");
                                        xModel; xModel = xModel->NextSiblingElement("model")) {
          if (!xModel->Attribute("description")) {
               Log->Write("ERROR: \t missing attribute description in models?");
               return false;
          }

          std::string modelName = std::string(xModel->Attribute("description"));
          int model_id = xmltoi(xModel->Attribute("operational_model_id"), -1);

          if ((_model==MODEL_GCFM) && (model_id==MODEL_GCFM)) {
               if (modelName!="gcfm") {
                    Log->Write("ERROR: \t mismatch model ID and description. Did you mean gcfm?");
                    return false;
               }
               if (!ParseGCFMModel(xModel, xMainNode))
                    return false;
               parsingModelSuccessful = true;
               //only parsing one model
               break;
          }
          else if ((_model==MODEL_GOMPERTZ) && (model_id==MODEL_GOMPERTZ)) {
               if (modelName!="gompertz") {
                    Log->Write("ERROR: \t mismatch model ID and description. Did you mean gompertz?");
                    return false;
               }
               //only parsing one model
               if (!ParseGompertzModel(xModel, xMainNode))
                    return false;
               parsingModelSuccessful = true;
               break;
          }
          else if ((_model==MODEL_GRADIENT) && (model_id==MODEL_GRADIENT)) {
               if (modelName!="gradnav") {
                    Log->Write("ERROR: \t mismatch model ID and description. Did you mean gradnav?");
                    return false;
               }
               //only parsing one model
               if (!ParseGradientModel(xModel, xMainNode))
                    return false;
               parsingModelSuccessful = true;
               break;
          }
          else if ((_model==MODEL_VELOCITY) && (model_id==MODEL_VELOCITY)) {
               if (modelName!="Tordeux2015") {
                    Log->Write("ERROR:\t mismatch model ID and description. Did you mean Tordeux2015?");
                    return false;
               }
               //only parsing one model
               if (!ParseVelocityModel(xModel, xMainNode))
                    return false;
               parsingModelSuccessful = true;
               break;
          }
          if ((_model==MODEL_KRAUSZ) && (model_id==MODEL_KRAUSZ)) {
               if (modelName!="krausz") {
                    Log->Write("ERROR: \t mismatch model ID and description. Did you mean krausz?");
                    return false;
               }
               if (!ParseKrauszModel(xModel, xMainNode))
                    return false;
               parsingModelSuccessful = true;
               //only parsing one model
               break;
          }
     }

     if (!parsingModelSuccessful) {
          Log->Write("ERROR: \tWrong model id [%d]. Choose 1 (GCFM), 2 (Gompertz),  3 (Tordeux2015) or 5 (Krausz)", _model);
          Log->Write("ERROR: \tPlease make sure that all models are specified in the operational_models section");
          Log->Write("ERROR: \tand make sure to use the same ID in the agent section");
          return false;
     }

     //route choice strategy
     TiXmlNode* xRouters = xMainNode->FirstChild("route_choice_models");
     TiXmlNode* xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");

     if (!ParseRoutingStrategies(xRouters, xAgentDistri))
          return false;
     Log->Write("INFO: \tParsing the project file completed");
     return true;
}

bool IniFileParser::ParseHeader(TiXmlNode* xHeader)
{
     //logfile
     if (xHeader->FirstChild("logfile")) {
          const fs::path logPath( xHeader->FirstChild("logfile")->FirstChild()->Value());
          const fs::path& root(_config->GetProjectRootDir()); // returns an absolute path already
          const fs::path canonicalPath = fs::weakly_canonical(root / logPath);
          _config->SetErrorLogFile(canonicalPath);
          _config->SetLog(2);
          Log->Write("INFO:\tlogfile <%s>", _config->GetErrorLogFile().string().c_str());
     }
     Log->Write("----\nJuPedSim - JPScore\n");
     Log->Write("Current date   : %s %s", __DATE__, __TIME__);
     Log->Write("Version        : %s", JPSCORE_VERSION);
     Log->Write("Commit hash    : %s", GIT_COMMIT_HASH);
     Log->Write("Commit date    : %s", GIT_COMMIT_DATE);
     Log->Write("Branch         : %s\n----\n", GIT_BRANCH);


     //seed
     if (xHeader->FirstChild("seed")) {
          TiXmlNode* seedNode = xHeader->FirstChild("seed")->FirstChild();
          if (seedNode) {
               const char* seedValue = seedNode->Value();
               _config->SetSeed((unsigned int)atoi(seedValue));//strtol
          }
          else {
               _config->SetSeed((unsigned int)time(NULL));
          }
     }
     // srand(_config->GetSeed());
     Log->Write("INFO:\trandom seed <%d>", _config->GetSeed());

     // max simulation time
     if (xHeader->FirstChild("max_sim_time")) {
          const char* tmax =
               xHeader->FirstChildElement("max_sim_time")->FirstChild()->Value();
          _config->SetTmax(atof(tmax));
          Log->Write("INFO: \tMaximal simulation time <%.2f> seconds", _config->GetTmax());
     }

     // Progressbar
     if (xHeader->FirstChild("progressbar")) {
          _config->SetPRB(true);
          Log->Write("INFO: \tUse Progressbar");
     }

     // geometry file name
     if (xHeader->FirstChild("geometry")) {
          std::string filename = xHeader->FirstChild("geometry")->FirstChild()->Value();
          _config->SetGeometryFile(filename);
          Log->Write("INFO: \tgeometry <%s>", filename.c_str());
     }


     //max CPU
     int max_threads = 1;
#ifdef _OPENMP
     max_threads = omp_get_max_threads();
#endif
     if (xHeader->FirstChild("num_threads")) {
          TiXmlNode* numthreads = xHeader->FirstChild("num_threads")->FirstChild();
          if (numthreads) {
#ifdef _OPENMP
               omp_set_num_threads(xmltoi(numthreads->Value()));
#endif
          }
     }
     _config->SetMaxOpenMPThreads(omp_get_max_threads());
     Log->Write("INFO:\tUsing num_threads <%d> threads (%d available)", _config->GetMaxOpenMPThreads(), max_threads);

     //display statistics
     if (xHeader->FirstChild("show_statistics")) {
          std::string value = xHeader->FirstChild("show_statistics")->FirstChild()->Value();
          _config->SetShowStatistics(value=="true");
          Log->Write("INFO: \tShow statistics: %s", value.c_str());
     }

     //trajectories
     TiXmlNode* xTrajectories = xHeader->FirstChild("trajectories");
     if (xTrajectories) {
          double fps;
          xHeader->FirstChildElement("trajectories")->Attribute("fps", &fps);
          _config->SetFps(fps);

          std::string format =
               xHeader->FirstChildElement("trajectories")->Attribute(
                    "format") ?
               xHeader->FirstChildElement("trajectories")->Attribute(
                    "format") :
               "xml-plain";
          int embedMesh = 0;
          if (xHeader->FirstChildElement("trajectories")->Attribute(
                   "embed_mesh")) {
               embedMesh =
                    std::string(xHeader->FirstChildElement("trajectories")->Attribute("embed_mesh"))=="true" ? 1 : 0;
          }
          if (format=="xml-plain")
               _config->SetFileFormat(FORMAT_XML_PLAIN);
          if (format=="xml-plain" && embedMesh==1)
               _config->SetFileFormat(FORMAT_XML_PLAIN_WITH_MESH);
          if (format=="xml-bin")
               _config->SetFileFormat(FORMAT_XML_BIN);
          if (format=="plain")
               _config->SetFileFormat(FORMAT_PLAIN);
          if (format=="vtk")
               _config->SetFileFormat(FORMAT_VTK);

          //color mode
          std::string color_mode =
               xHeader->FirstChildElement("trajectories")->Attribute(
                    "color_mode") ?
               xHeader->FirstChildElement("trajectories")->Attribute(
                    "color_mode") :
               "velocity";

          if (color_mode=="velocity")
               Pedestrian::SetColorMode(
                    AgentColorMode::BY_VELOCITY); //TODO: config parameter! does not belong to the pedestrian model, we should create a pedestrian config instead. [gl march '16]
          if (color_mode=="spotlight") Pedestrian::SetColorMode(AgentColorMode::BY_SPOTLIGHT);
          if (color_mode=="group") Pedestrian::SetColorMode(AgentColorMode::BY_GROUP);
          if (color_mode=="knowledge") Pedestrian::SetColorMode(AgentColorMode::BY_KNOWLEDGE);
          if (color_mode=="router") Pedestrian::SetColorMode(AgentColorMode::BY_ROUTER);
          if (color_mode=="final_goal") Pedestrian::SetColorMode(AgentColorMode::BY_FINAL_GOAL);
          if (color_mode=="intermediate_goal") Pedestrian::SetColorMode(AgentColorMode::BY_INTERMEDIATE_GOAL);




          //a file descriptor was given
          if (xTrajectories->FirstChild("file")) {
               const fs::path trajLoc( xTrajectories->FirstChildElement("file")->Attribute("location"));
               if (!trajLoc.empty())
               {
                    const fs::path& root(_config->GetProjectRootDir()); // returns an absolute path already
                    const fs::path canonicalTrajPath = fs::weakly_canonical(root / trajLoc);
                    _config->SetTrajectoriesFile(canonicalTrajPath);
                    _config->SetOriginalTrajectoriesFile(canonicalTrajPath);
               }

               Log->Write("INFO: \toutput file  <%s>", _config->GetTrajectoriesFile().string().c_str());
               Log->Write("INFO: \tin format <%s> at <%.0f> frames per seconds",format.c_str(), _config->GetFps());
          }

          if (xTrajectories->FirstChild("socket")) {
               std::string tmp =
                    xTrajectories->FirstChildElement("socket")->Attribute("hostname");
               if (tmp.c_str())
                    _config->SetHostname(tmp);
               int port;
               xTrajectories->FirstChildElement("socket")->Attribute("port", &port);
               _config->SetPort(port);
               Log->Write("INFO: \tStreaming results to output [%s:%d] ",
                          _config->GetHostname().c_str(), _config->GetPort());
          }
     }
     return true;
}

bool IniFileParser::ParseGCFMModel(TiXmlElement* xGCFM, TiXmlElement* xMainNode)
{
     Log->Write("\nINFO:\tUsing the GCFM model");
     Log->Write("INFO:\tParsing the model parameters");

     TiXmlNode* xModelPara = xGCFM->FirstChild("model_parameters");
     if (!xModelPara) {
          Log->Write("ERROR: \t !!!! Changes in the operational model section !!!");
          Log->Write("ERROR: \t !!!! The new version is in inputfiles/ship_msw/ini_ship2.xml !!!");
          return false;
     }

     // For convenience. This moved to the header as it is not model specific
     if (xModelPara->FirstChild("tmax")) {
          Log->Write(
                    "ERROR: \tthe maximal simulation time section moved to the header!!!");
          Log->Write("ERROR: \t\t <max_sim_time> </max_sim_time>\n");
          return false;
     }

     //solver
     if (!ParseNodeToSolver(*xModelPara))
          return false;

     //stepsize
     if (!ParseStepSize(*xModelPara))
          return false;

     //exit crossing strategy
     if (!ParseStrategyNodeToObject(*xModelPara))
          return false;

     //linked-cells
     if (!ParseLinkedCells(*xModelPara))
          return false;

     //force_ped
         if (xModelPara->FirstChild("force_ped")) {
                 std::string nu = xModelPara->FirstChildElement("force_ped")->Attribute("nu");
                 std::string dist_max = xModelPara->FirstChildElement("force_ped")->Attribute(
                         "dist_max");
                 std::string disteff_max =
                         xModelPara->FirstChildElement("force_ped")->Attribute(
                                 "disteff_max"); // @todo: rename disteff_max to force_max
                 std::string interpolation_width =
                         xModelPara->FirstChildElement("force_ped")->Attribute(
                                 "interpolation_width");

                 _config->SetMaxFPed(atof(dist_max.c_str()));
                 _config->SetNuPed(atof(nu.c_str()));
                 _config->SetDistEffMaxPed(atof(disteff_max.c_str()));
                 _config->SetIntPWidthPed(atof(interpolation_width.c_str()));
                 Log->Write(
                         "INFO: \tfrep_ped nu=%.3f, dist_max=%.3f, disteff_max=%.3f, interpolation_width=%.3f",
                         atof(nu.c_str()), atof(dist_max.c_str()), atof(disteff_max.c_str()), atof(interpolation_width.c_str()));
         }

     //force_wall
     if (xModelPara->FirstChild("force_wall")) {
          std::string nu = xModelPara->FirstChildElement("force_wall")->Attribute("nu");
          std::string dist_max = xModelPara->FirstChildElement("force_wall")->Attribute(
                    "dist_max");
          std::string disteff_max =
                    xModelPara->FirstChildElement("force_wall")->Attribute(
                              "disteff_max");
          std::string interpolation_width =
                    xModelPara->FirstChildElement("force_wall")->Attribute(
                              "interpolation_width");
          _config->SetMaxFWall(atof(dist_max.c_str()));
          _config->SetNuWall(atof(nu.c_str()));
          _config->SetDistEffMaxWall(atof(disteff_max.c_str()));
          _config->SetIntPWidthWall(atof(interpolation_width.c_str()));
          Log->Write(
                    "INFO: \tfrep_wall mu=%.3f, dist_max=%.3f, disteff_max=%.3f, interpolation_width=%.3f",
                          atof(nu.c_str()), atof(dist_max.c_str()), atof(disteff_max.c_str()), atof(interpolation_width.c_str()));
     }

     //Parsing the agent parameters
     TiXmlNode* xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
     ParseAgentParameters(xGCFM, xAgentDistri);

     //TODO: models do not belong in a configuration container [gl march '16]
     _config->SetModel(std::shared_ptr<OperationalModel>(new GCFMModel(_directionManager, _config->GetNuPed(),
               _config->GetNuWall(), _config->GetDistEffMaxPed(),
               _config->GetDistEffMaxWall(), _config->GetIntPWidthPed(),
               _config->GetIntPWidthWall(), _config->GetMaxFPed(),
               _config->GetMaxFWall())));

     return true;
}

bool IniFileParser::ParseKrauszModel(TiXmlElement* xKrausz, TiXmlElement* xMainNode)
{
     Log->Write("\nINFO:\tUsing the Krausz model");
     Log->Write("INFO:\tParsing the model parameters");

     TiXmlNode* xModelPara = xKrausz->FirstChild("model_parameters");
     if (!xModelPara) {
          Log->Write("ERROR: \t !!!! Changes in the operational model section !!!");
          Log->Write("ERROR: \t !!!! The new version is in inputfiles/ship_msw/ini_ship2.xml !!!");
          return false;
     }

     // For convenience. This moved to the header as it is not model specific
     if (xModelPara->FirstChild("tmax")) {
          Log->Write(
                  "ERROR: \tthe maximal simulation time section moved to the header!!!");
          Log->Write("ERROR: \t\t <max_sim_time> </max_sim_time>\n");
          return false;
     }

     //solver
     if (!ParseNodeToSolver(*xModelPara))
          return false;

     //stepsize
     if (!ParseStepSize(*xModelPara))
          return false;

     //exit crossing strategy
     if (!ParseStrategyNodeToObject(*xModelPara))
          return false;

     //linked-cells
     if (!ParseLinkedCells(*xModelPara))
          return false;

     //force_ped
     if (xModelPara->FirstChild("force_ped")) {
          std::string nu = xModelPara->FirstChildElement("force_ped")->Attribute("nu");
          std::string dist_max = xModelPara->FirstChildElement("force_ped")->Attribute(
                  "dist_max");
          std::string disteff_max =
                  xModelPara->FirstChildElement("force_ped")->Attribute(
                          "disteff_max"); // @todo: rename disteff_max to force_max
          std::string interpolation_width =
                  xModelPara->FirstChildElement("force_ped")->Attribute(
                          "interpolation_width");

          _config->SetMaxFPed(atof(dist_max.c_str()));
          _config->SetNuPed(atof(nu.c_str()));
          _config->SetDistEffMaxPed(atof(disteff_max.c_str()));
          _config->SetIntPWidthPed(atof(interpolation_width.c_str()));
          Log->Write(
                  "INFO: \tfrep_ped nu=%.3f, dist_max=%.3f, disteff_max=%.3f, interpolation_width=%.3f",
                          atof(nu.c_str()), atof(dist_max.c_str()), atof(disteff_max.c_str()), atof(interpolation_width.c_str()));
     }

     //force_wall
     if (xModelPara->FirstChild("force_wall")) {
          std::string nu = xModelPara->FirstChildElement("force_wall")->Attribute("nu");
          std::string dist_max = xModelPara->FirstChildElement("force_wall")->Attribute(
                  "dist_max");
          std::string disteff_max =
                  xModelPara->FirstChildElement("force_wall")->Attribute(
                          "disteff_max");
          std::string interpolation_width =
                  xModelPara->FirstChildElement("force_wall")->Attribute(
                          "interpolation_width");
          _config->SetMaxFWall(atof(dist_max.c_str()));
          _config->SetNuWall(atof(nu.c_str()));
          _config->SetDistEffMaxWall(atof(disteff_max.c_str()));
          _config->SetIntPWidthWall(atof(interpolation_width.c_str()));
          Log->Write(
                  "INFO: \tfrep_wall mu=%.3f, dist_max=%.3f, disteff_max=%.3f, interpolation_width=%.3f",
                          atof(nu.c_str()), atof(dist_max.c_str()), atof(disteff_max.c_str()), atof(interpolation_width.c_str()));
     }

     //Parsing the agent parameters
     TiXmlNode* xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
     ParseAgentParameters(xKrausz, xAgentDistri);

     //TODO: models do not belong in a configuration container [gl march '16]
     _config->SetModel(std::shared_ptr<OperationalModel>(new KrauszModel(_directionManager, _config->GetNuPed(),
                                                                         _config->GetNuWall(), _config->GetDistEffMaxPed(),
                                                                         _config->GetDistEffMaxWall(), _config->GetIntPWidthPed(),
                                                                         _config->GetIntPWidthWall(), _config->GetMaxFPed(),
                                                                         _config->GetMaxFWall())));
     return true;
}

bool IniFileParser::ParseGompertzModel(TiXmlElement* xGompertz, TiXmlElement* xMainNode)
{
     //parsing the model parameters
     Log->Write("\nINFO:\tUsing the Gompertz model");

     Log->Write("INFO:\tParsing the model parameters");

     TiXmlNode* xModelPara = xGompertz->FirstChild("model_parameters");
     if (!xModelPara) {
          Log->Write("ERROR: \t !!!! Changes in the operational model section !!!");
          Log->Write("ERROR: \t !!!! The new version is in inputfiles/ship_msw/ini_ship3.xml !!!");
          return false;
     }

     // For convenience. This moved to the header as it is not model specific
     if (xModelPara->FirstChild("tmax")) {
          Log->Write("ERROR: \tthe maximal simulation time section moved to the header!!!");
          Log->Write("ERROR: \t\t <max_sim_time> </max_sim_time>\n");
          return false;
     }

     //solver
     if (!ParseNodeToSolver(*xModelPara))
          return false;

     //stepsize
     if (!ParseStepSize(*xModelPara))
          return false;

     //exit crossing strategy
     if (!ParseStrategyNodeToObject(*xModelPara))
          return false;

     //linked-cells
     if (!ParseLinkedCells(*xModelPara))
          return false;

     //force_ped
     if (xModelPara->FirstChild("force_ped")) {
          std::string nu = xModelPara->FirstChildElement("force_ped")->Attribute("nu");
          _config->SetNuPed(atof(nu.c_str()));

          if (!xModelPara->FirstChildElement("force_ped")->Attribute("a"))
               _config->SetaPed(1.0); // default value
          else {
               std::string a = xModelPara->FirstChildElement("force_ped")->Attribute("a");
               _config->SetaPed(atof(a.c_str()));
          }
          if (!xModelPara->FirstChildElement("force_ped")->Attribute("b"))
               _config->SetbPed(0.25); // default value
          else {
               std::string b = xModelPara->FirstChildElement("force_ped")->Attribute("b");
               _config->SetbPed(atof(b.c_str()));
          }
          if (!xModelPara->FirstChildElement("force_ped")->Attribute("c"))
               _config->SetcPed(3.0); // default value
          else {
               std::string c = xModelPara->FirstChildElement("force_ped")->Attribute("c");
               _config->SetcPed(atof(c.c_str()));
          }
          Log->Write("INFO: \tfrep_ped mu=%s, a=%0.2f, b=%0.2f c=%0.2f", nu.c_str(), _config->GetaPed(),
                    _config->GetbPed(), _config->GetcPed());
     }
     //force_wall
     if (xModelPara->FirstChild("force_wall")) {
          std::string nu = xModelPara->FirstChildElement("force_wall")->Attribute("nu");
          _config->SetNuWall(atof(nu.c_str()));
          if (!xModelPara->FirstChildElement("force_wall")->Attribute("a"))
               _config->SetaWall(1.0); // default value
          else {
               std::string a = xModelPara->FirstChildElement("force_wall")->Attribute("a");
               _config->SetaWall(atof(a.c_str()));
          }
          if (!xModelPara->FirstChildElement("force_wall")->Attribute("b"))
               _config->SetbWall(0.7); // default value
          else {
               std::string b = xModelPara->FirstChildElement("force_wall")->Attribute("b");
               _config->SetbWall(atof(b.c_str()));
          }
          if (!xModelPara->FirstChildElement("force_wall")->Attribute("c"))
               _config->SetcWall(3.0); // default value
          else {
               std::string c = xModelPara->FirstChildElement("force_wall")->Attribute("c");
               _config->SetcWall(atof(c.c_str()));
          }

          Log->Write("INFO: \tfrep_wall mu=%s, a=%0.2f, b=%0.2f c=%0.2f", nu.c_str(), _config->GetaWall(),
                    _config->GetbWall(), _config->GetcWall());
     }

     //Parsing the agent parameters
     TiXmlNode* xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
     ParseAgentParameters(xGompertz, xAgentDistri);

     //TODO: models do not belong in a configuration container [gl march '16]
     _config->SetModel(std::shared_ptr<OperationalModel>(new GompertzModel(_directionManager, _config->GetNuPed(),
               _config->GetaPed(), _config->GetbPed(), _config->GetcPed(),
               _config->GetNuWall(), _config->GetaWall(), _config->GetbWall(),
               _config->GetcWall())));

     return true;
}

bool IniFileParser::ParseGradientModel(TiXmlElement* xGradient, TiXmlElement* xMainNode)
{
     //parsing the model parameters
     Log->Write("\nINFO:\tUsing the Gradient model");

     Log->Write("INFO:\tParsing the model parameters");

     TiXmlNode* xModelPara = xGradient->FirstChild("model_parameters");

     if (!xModelPara) {
          Log->Write("ERROR: \t !!!! Changes in the operational model section !!!");
          Log->Write("ERROR: \t !!!! The new version is in inputfiles/ship_msw/ini_ship3.xml !!!");
          return false;
     }

     // For convenience. This moved to the header as it is not model specific
     if (xModelPara->FirstChild("tmax")) {
          Log->Write("ERROR: \tthe maximal simulation time section moved to the header!!!");
          Log->Write("ERROR: \t\t <max_sim_time> </max_sim_time>\n");
          return false;
     }

     //solver
     if (!ParseNodeToSolver(*xModelPara))
          return false;

     //stepsize
     if (!ParseStepSize(*xModelPara))
          return false;

     //exit crossing strategy
     if (!ParseStrategyNodeToObject(*xModelPara))
          return false;

     //floorfield
     double pDeltaH = 0., pWallAvoidDistance = 0., pSlowDownDistance = 0.; //TODO: should be moved to configuration [gl march '16]
     bool pUseWallAvoidance = false;
     if (xModelPara->FirstChild("floorfield")) {
          if (!xModelPara->FirstChildElement("floorfield")->Attribute("delta_h"))
               pDeltaH = 0.0625; // default value
          else {
               std::string delta_h = xModelPara->FirstChildElement("floorfield")->Attribute("delta_h");
               pDeltaH = atof(delta_h.c_str());
          }
          _config->set_deltaH(pDeltaH);

          if (!xModelPara->FirstChildElement("floorfield")->Attribute("wall_avoid_distance"))
               pWallAvoidDistance = .8; // default value
          else {
               std::string wall_avoid_distance = xModelPara->FirstChildElement("floorfield")->Attribute(
                         "wall_avoid_distance");
               pWallAvoidDistance = atof(wall_avoid_distance.c_str());
          }
          _config->set_wall_avoid_distance(pWallAvoidDistance);

          if (!xModelPara->FirstChildElement("floorfield")->Attribute("use_wall_avoidance"))
               pUseWallAvoidance = true; // default value
          else {
               std::string use_wall_avoidance = xModelPara->FirstChildElement("floorfield")->Attribute("use_wall_avoidance");
               pUseWallAvoidance = !(use_wall_avoidance=="false");
          }
          _config->set_use_wall_avoidance(pUseWallAvoidance);
          Log->Write("INFO: \tfloorfield <delta h=%0.4f, wall avoid distance=%0.2f>", pDeltaH, pWallAvoidDistance);
          Log->Write("INFO: \tfloorfield <use wall avoidance=%s>", pUseWallAvoidance ? "true" : "false");
     }

     //linked-cells
     if (!ParseLinkedCells(*xModelPara))
          return false;


     //force_ped
     if (xModelPara->FirstChild("force_ped")) {
          std::string nu = xModelPara->FirstChildElement("force_ped")->Attribute("nu");
          _config->SetNuPed(atof(nu.c_str()));

          if (!xModelPara->FirstChildElement("force_ped")->Attribute("a"))
               _config->SetaPed(1.0); // default value
          else {
               std::string a = xModelPara->FirstChildElement("force_ped")->Attribute("a");
               _config->SetaPed(atof(a.c_str()));
          }

          if (!xModelPara->FirstChildElement("force_ped")->Attribute("b"))
               _config->SetbPed(0.25); // default value
          else {
               std::string b = xModelPara->FirstChildElement("force_ped")->Attribute("b");
               _config->SetbPed(atof(b.c_str()));
          }
          if (!xModelPara->FirstChildElement("force_ped")->Attribute("c"))
               _config->SetcPed(3.0); // default value
          else {
               std::string c = xModelPara->FirstChildElement("force_ped")->Attribute("c");
               _config->SetcPed(atof(c.c_str()));
          }
          Log->Write("INFO: \tfrep_ped mu=%s, a=%0.2f, b=%0.2f c=%0.2f", nu.c_str(), _config->GetaPed(),
                    _config->GetbPed(), _config->GetcPed());
     }
     //force_wall
     if (xModelPara->FirstChild("force_wall")) {
          std::string nu = xModelPara->FirstChildElement("force_wall")->Attribute("nu");
          _config->SetNuWall(atof(nu.c_str()));

          if (!xModelPara->FirstChildElement("force_wall")->Attribute("a"))
               _config->SetaWall(1.0); // default value
          else {
               std::string a = xModelPara->FirstChildElement("force_wall")->Attribute("a");
               _config->SetaWall(atof(a.c_str()));
          }

          if (!xModelPara->FirstChildElement("force_wall")->Attribute("b"))
               _config->SetbWall(0.7); // default value
          else {
               std::string b = xModelPara->FirstChildElement("force_wall")->Attribute("b");
               _config->SetbWall(atof(b.c_str()));
          }
          if (!xModelPara->FirstChildElement("force_wall")->Attribute("c"))
               _config->SetcWall(3.0); // default value
          else {
               std::string c = xModelPara->FirstChildElement("force_wall")->Attribute("c");
               _config->SetcWall(atof(c.c_str()));
          }
          Log->Write("INFO: \tfrep_wall mu=%s, a=%0.2f, b=%0.2f c=%0.2f", nu.c_str(), _config->GetaWall(),
                    _config->GetbWall(), _config->GetcWall());
     }
     //anti_clipping
     if (xModelPara->FirstChild("anti_clipping")) {
          if (!xModelPara->FirstChildElement("anti_clipping")->Attribute("slow_down_distance"))
               pSlowDownDistance = .2; //default value
          else {
               std::string slow_down_distance = xModelPara->FirstChildElement("anti_clipping")->Attribute(
                         "slow_down_distance");
               pSlowDownDistance = atof(slow_down_distance.c_str());
          }
          _config->set_slow_down_distance(pSlowDownDistance);
          Log->Write("INFO: \tAnti Clipping: SlowDown Distance=%0.2f", pSlowDownDistance);
     }

     //Parsing the agent parameters
     TiXmlNode* xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
     ParseAgentParameters(xGradient, xAgentDistri);

     //TODO: models do not belong in a configuration container [gl march '16]
     _config->SetModel(std::shared_ptr<OperationalModel>(new GradientModel(_directionManager, _config->GetNuPed(),
               _config->GetaPed(), _config->GetbPed(), _config->GetcPed(),
               _config->GetNuWall(), _config->GetaWall(), _config->GetbWall(),
               _config->GetcWall(), _config->get_deltaH(), _config->get_wall_avoid_distance(), _config->get_use_wall_avoidance(),
               _config->get_slow_down_distance())));

     return true;
}

bool IniFileParser::ParseVelocityModel(TiXmlElement* xVelocity, TiXmlElement* xMainNode)
{
     //parsing the model parameters
     Log->Write("\nINFO:\tUsing Tordeux2015 model");
     Log->Write("INFO:\tParsing the model parameters");

     TiXmlNode* xModelPara = xVelocity->FirstChild("model_parameters");

     if (!xModelPara) {
          Log->Write("ERROR: \t !!!! Changes in the operational model section !!!");
          Log->Write("ERROR: \t !!!! The new version is in inputfiles/ship_msw/ini_ship3.xml !!!");
          return false;
     }

     // For convenience. This moved to the header as it is not model specific
     if (xModelPara->FirstChild("tmax")) {
          Log->Write("ERROR: \tthe maximal simulation time section moved to the header!!!");
          Log->Write("ERROR: \t\t <max_sim_time> </max_sim_time>\n");
          return false;
     }

     //solver
     if (!ParseNodeToSolver(*xModelPara))
          return false;

     //stepsize
     if (!ParseStepSize(*xModelPara))
          return false;

     //exit crossing strategy
     if (!ParseStrategyNodeToObject(*xModelPara))
          return false;

     //linked-cells
     if (!ParseLinkedCells(*xModelPara))
          return false;

     //periodic
     if (!ParsePeriodic(*xModelPara))
          return false;

     //force_ped
     if (xModelPara->FirstChild("force_ped")) {

          if (!xModelPara->FirstChildElement("force_ped")->Attribute("a"))
               _config->SetaPed(1.0); // default value
          else {
               std::string a = xModelPara->FirstChildElement("force_ped")->Attribute("a");
               _config->SetaPed(atof(a.c_str()));
          }

          if (!xModelPara->FirstChildElement("force_ped")->Attribute("D"))
               _config->SetDPed(0.1); // default value in [m]
          else {
               std::string D = xModelPara->FirstChildElement("force_ped")->Attribute("D");
               _config->SetDPed(atof(D.c_str()));
          }
          Log->Write("INFO: \tfrep_ped a=%0.2f, D=%0.2f", _config->GetaPed(), _config->GetDPed());

     }
     //force_wall
     if (xModelPara->FirstChild("force_wall")) {

          if (!xModelPara->FirstChildElement("force_wall")->Attribute("a"))
               _config->SetaWall(1.0); // default value
          else {
               std::string a = xModelPara->FirstChildElement("force_wall")->Attribute("a");
               _config->SetaWall(atof(a.c_str()));
          }

          if (!xModelPara->FirstChildElement("force_wall")->Attribute("D"))
               _config->SetDWall(0.1); // default value in [m]
          else {
               std::string D = xModelPara->FirstChildElement("force_wall")->Attribute("D");
               _config->SetDWall(atof(D.c_str()));
          }
          Log->Write("INFO: \tfrep_wall a=%0.2f, D=%0.2f", _config->GetaWall(), _config->GetDWall());
     }

     //Parsing the agent parameters
     TiXmlNode* xAgentDistri = xMainNode->FirstChild("agents")->FirstChild("agents_distribution");
     ParseAgentParameters(xVelocity, xAgentDistri);
     _config->SetModel(std::shared_ptr<OperationalModel>(new VelocityModel(_directionManager, _config->GetaPed(),
               _config->GetDPed(), _config->GetaWall(),
               _config->GetDWall())));

     return true;
}

void IniFileParser::ParseAgentParameters(TiXmlElement* operativModel, TiXmlNode* agentsDistri)
{
     //Parsing the agent parameters
     Log->Write("\nINFO:\tParsing agents  parameters");
     //first get list of actually used router
     std::vector<int> usedAgentParams;
     usedAgentParams.clear();
     for (TiXmlElement* e = agentsDistri->FirstChildElement("group"); e;
          e = e->NextSiblingElement("group")) {
          int agentsParams = -1;
          if (e->Attribute("agent_parameter_id")) {
               agentsParams = atoi(e->Attribute("agent_parameter_id"));
               if(std::find(usedAgentParams.begin(), usedAgentParams.end(), agentsParams) == usedAgentParams.end()) {
                    usedAgentParams.emplace_back(agentsParams);
               }
          }
     }
     for(TiXmlElement* xAgentPara = operativModel->FirstChildElement("agent_parameters"); xAgentPara;
         xAgentPara = xAgentPara->NextSiblingElement("agent_parameters")) {

          //get the group ID
          int para_id= xmltoi(xAgentPara->Attribute("agent_parameter_id"),-1);
          if (std::find(usedAgentParams.begin(), usedAgentParams.end(), para_id) != usedAgentParams.end() ) {
               Log->Write("INFO: \tParsing the group parameter id [%d]", para_id);

               auto agentParameters = std::shared_ptr<AgentsParameters>(new AgentsParameters(para_id, _config->GetSeed()));
               _config->AddAgentsParameters(agentParameters, para_id);

               //desired speed
               if (xAgentPara->FirstChild("v0")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("v0")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("v0")->Attribute("sigma"));
                    agentParameters->InitV0(mu, sigma);
                    agentParameters->InitV0DownStairs(mu, sigma);
                    agentParameters->InitV0UpStairs(mu, sigma);
                    Log->Write("INFO: \tdesired speed mu=%f , sigma=%f", mu, sigma);
               }

               if (xAgentPara->FirstChild("v0_upstairs")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("v0_upstairs")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("v0_upstairs")->Attribute("sigma"));
                    agentParameters->InitV0UpStairs(mu, sigma);
                    Log->Write("INFO: \tdesired speed upstairs mu=%f , sigma=%f", mu, sigma);
               }

               if (xAgentPara->FirstChild("v0_downstairs")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("v0_downstairs")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("v0_downstairs")->Attribute("sigma"));
                    agentParameters->InitV0DownStairs(mu, sigma);
                    Log->Write("INFO: \tdesired speed downstairs mu=%f , sigma=%f", mu, sigma);
               }//------------------------------------------------------------------------
               if (xAgentPara->FirstChild("escalator_upstairs")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("escalator_upstairs")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("escalator_upstairs")->Attribute("sigma"));
                    agentParameters->InitEscalatorUpStairs(mu, sigma);
                    Log->Write("INFO: \tspeed of escalator upstairs mu=%f , sigma=%f", mu, sigma);
               }
               if (xAgentPara->FirstChild("escalator_downstairs")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("escalator_downstairs")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("escalator_downstairs")->Attribute("sigma"));
                    agentParameters->InitEscalatorDownStairs(mu, sigma);
                    Log->Write("INFO: \tspeed of escalator downstairs mu=%f , sigma=%f", mu, sigma);
               }
               if (xAgentPara->FirstChild("v0_idle_escalator_upstairs")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("v0_idle_escalator_upstairs")->Attribute("mu"));
                    double sigma = xmltof(
                              xAgentPara->FirstChildElement("v0_idle_escalator_upstairs")->Attribute("sigma"));
                    agentParameters->InitV0IdleEscalatorUpStairs(mu, sigma);
                    Log->Write("INFO: \tdesired speed idle escalator upstairs mu=%f , sigma=%f", mu, sigma);
               }
               if (xAgentPara->FirstChild("v0_idle_escalator_downstairs")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("v0_idle_escalator_downstairs")->Attribute("mu"));
                    double sigma = xmltof(
                              xAgentPara->FirstChildElement("v0_idle_escalator_downstairs")->Attribute("sigma"));
                    agentParameters->InitV0IdleEscalatorDownStairs(mu, sigma);
                    Log->Write("INFO: \tdesired speed idle escalator downstairs mu=%f , sigma=%f", mu, sigma);
               }
               //------------------------------------------------------------------------

               //bmax
               if (xAgentPara->FirstChild("bmax")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("bmax")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("bmax")->Attribute("sigma"));
                    agentParameters->InitBmax(mu, sigma);
                    Log->Write("INFO: \tBmax mu=%f , sigma=%f", mu, sigma);
               }

               //bmin
               if (xAgentPara->FirstChild("bmin")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("bmin")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("bmin")->Attribute("sigma"));
                    agentParameters->InitBmin(mu, sigma);
                    Log->Write("INFO: \tBmin mu=%f , sigma=%f", mu, sigma);
               }

               //amin
               if (xAgentPara->FirstChild("amin")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("amin")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("amin")->Attribute("sigma"));
                    agentParameters->InitAmin(mu, sigma);
                    Log->Write("INFO: \tAmin mu=%f , sigma=%f", mu, sigma);
               }
               //tau
               if (xAgentPara->FirstChild("tau")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("tau")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("tau")->Attribute("sigma"));
                    agentParameters->InitTau(mu, sigma);
                    Log->Write("INFO: \tTau mu=%f , sigma=%f", mu, sigma);
               }
               //atau
               if (xAgentPara->FirstChild("atau")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("atau")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("atau")->Attribute("sigma"));
                    agentParameters->InitAtau(mu, sigma);
                    Log->Write("INFO: \tAtau mu=%f , sigma=%f", mu, sigma);
               }
               // T
               if (xAgentPara->FirstChild("T")) {
                    double mu = xmltof(xAgentPara->FirstChildElement("T")->Attribute("mu"));
                    double sigma = xmltof(xAgentPara->FirstChildElement("T")->Attribute("sigma"));
                    agentParameters->InitT(mu, sigma);
                    Log->Write("INFO: \tT mu=%f , sigma=%f", mu, sigma);
               }
               // swaying parameters
               if (xAgentPara->FirstChild("sway")) {
                    double freqA = xmltof(xAgentPara->FirstChildElement("sway")->Attribute("freqA"));
                    double freqB = xmltof(xAgentPara->FirstChildElement("sway")->Attribute("freqB"));
                    double ampA = xmltof(xAgentPara->FirstChildElement("sway")->Attribute("ampA"));
                    double ampB = xmltof(xAgentPara->FirstChildElement("sway")->Attribute("ampB"));
                    agentParameters->SetSwayParams(freqA, freqB, ampA, ampB);
                    Log->Write("INFO: \tSwaying parameters freqA=%f , freqB=%f , ampA=%f, ampB=%f");
               }

               if (_model == 2) { // Gompertz
                    double beta_c = 1; /// @todo quick and dirty
                    double max_Ea = agentParameters->GetAmin() + agentParameters->GetAtau() * agentParameters->GetV0();
                    double max_Eb = 0.5 * (agentParameters->GetBmin() +
                                           0.49); /// @todo hard-coded value should be the same as in pedestrians GetEB
                    double max_Ea_Eb = (max_Ea > max_Eb) ? max_Ea : max_Eb;
                    _config->SetDistEffMaxPed(2 * beta_c * max_Ea_Eb);
                    _config->SetDistEffMaxWall(_config->GetDistEffMaxPed());
               }

               if (_model == 4) { //  Gradient
                    double beta_c = 2; /// @todo quick and dirty
                    double max_Ea = agentParameters->GetAmin() + agentParameters->GetAtau() * agentParameters->GetV0();
                    double max_Eb = 0.5 * (agentParameters->GetBmin() +
                                           0.49); /// @todo hard-coded value should be the same as in pedestrians GetEB
                    double max_Ea_Eb = (max_Ea > max_Eb) ? max_Ea : max_Eb;
                    _config->SetDistEffMaxPed(2 * beta_c * max_Ea_Eb);
                    _config->SetDistEffMaxWall(_config->GetDistEffMaxPed());
               }

               if (_model == 3) { // Tordeux2015
                    double max_Eb = 2 * agentParameters->GetBmax();
                    _config->SetDistEffMaxPed(max_Eb+agentParameters->GetT()*agentParameters->GetV0());
                    _config->SetDistEffMaxWall(_config->GetDistEffMaxPed());
               }

               if (_model == 5) // Krausz
               {
                    agentParameters->EnableStretch(false);
               }
          }
     }
}

bool IniFileParser::ParseRoutingStrategies(TiXmlNode* routingNode, TiXmlNode* agentsDistri)
{
     if (!routingNode) {
          Log->Write("ERROR: \t route_choice_models section is missing");
          return false;
     }

     if (!agentsDistri) {
          Log->Write("ERROR: \t Agent Distribution section is missing");
          return false;
     }
     //first get list of actually used router
     std::vector<int> usedRouter;
     usedRouter.clear();
     bool hasSpecificGoals = false;
     for (TiXmlElement* e = agentsDistri->FirstChildElement("group"); e;
          e = e->NextSiblingElement("group")) {
          int router = -1;
          if (e->Attribute("router_id")) {
               router = atoi(e->Attribute("router_id"));
               if(std::find(usedRouter.begin(), usedRouter.end(), router) == usedRouter.end()) {
                    usedRouter.emplace_back(router);
               }
          }
          int goal = -1;
          if (e->Attribute("goal_id")) {
               goal = atoi(e->Attribute("goal_id"));
               if (goal != -1) {
                    hasSpecificGoals = true;
               }
          }
     }
     _config->set_has_specific_goals(hasSpecificGoals);
     for (TiXmlElement* e = routingNode->FirstChildElement("router"); e;
          e = e->NextSiblingElement("router")) {

          std::string strategy = e->Attribute("description");
          std::cout << "Description: <" <<strategy << ">" << std::endl;
          int id = atoi(e->Attribute("router_id"));

          if ((strategy == "local_shortest") &&
              (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
               //pRoutingStrategies.push_back(make_pair(id, ROUTING_LOCAL_SHORTEST));
               Router *r = new GlobalRouter(id, ROUTING_LOCAL_SHORTEST);
               _config->GetRoutingEngine()->AddRouter(r);
          }
          else if ((strategy == "global_shortest") &&
                   (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
               //pRoutingStrategies.push_back(make_pair(id, ROUTING_GLOBAL_SHORTEST));
               Router *r = new GlobalRouter(id, ROUTING_GLOBAL_SHORTEST);
               _config->GetRoutingEngine()->AddRouter(r);
          }
          else if ((strategy == "quickest")  &&
                   (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
               //pRoutingStrategies.push_back(make_pair(id, ROUTING_QUICKEST));
               Router *r = new QuickestPathRouter(id, ROUTING_QUICKEST);
               _config->GetRoutingEngine()->AddRouter(r);
          }
          else if ((strategy == "smoke") &&
                   (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
#ifndef JPSFIRE
               std::cerr << "\nCan not use smoke router without jpsfire. Rerun cmake with option  -DJPSFIRE=true and recompile.\n";
               exit(EXIT_FAILURE);
#endif
               Router *r = new SmokeRouter(id, ROUTING_SMOKE);
               _config->GetRoutingEngine()->AddRouter(r);

               Log->Write("\nINFO: \tUsing SmokeRouter");
               ///Parsing additional options
               if (!ParseCogMapOpts(e))
                    return false;
          }
          else if ((strategy == "AI") &&
                   (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
     #ifdef AIROUTER
               Router *r = new AIRouter(id, ROUTING_AI);
               _config->GetRoutingEngine()->AddRouter(r);

               Log->Write("\nINFO: \tUsing AIRouter");
               ///Parsing additional options
               if (!ParseAIOpts(e))
                    return false;
     #else
               std::cerr << "\nCan not use AI Router. Rerun cmake with option  -DAIROUTER=true and recompile.\n";
               exit(EXIT_FAILURE);
     #endif
          }
          else if ((strategy == "ff_global_shortest") &&
                    (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
               //pRoutingStrategies.push_back(make_pair(id, ROUTING_FF_GLOBAL_SHORTEST));
               Router *r = new FFRouter(id, ROUTING_FF_GLOBAL_SHORTEST, hasSpecificGoals, _config);
               _config->GetRoutingEngine()->AddRouter(r);

               if ((_exit_strat_number == 8) || (_exit_strat_number == 9)){
                    Log->Write("\nINFO: \tUsing FF Global Shortest Router");
               }
               else {
                    Log->Write("\nWARNING: \tExit Strategy Number is not 8 or 9!!!");
                    // config object holds default values, so we omit any set operations
               }

               ///Parsing additional options
               if (!ParseFfRouterOps(e, ROUTING_FF_GLOBAL_SHORTEST)) {
                    return false;
               }
          }

          else if ((strategy == "ff_local_shortest") &&
                   (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
               //pRoutingStrategies.push_back(make_pair(id, ROUTING_FF_GLOBAL_SHORTEST));
               Router *r = new FFRouter(id, ROUTING_FF_LOCAL_SHORTEST, hasSpecificGoals, _config);
               _config->GetRoutingEngine()->AddRouter(r);
               Log->Write("\nINFO: \tUsing FF Local Shortest Router");
               Log->Write("\nWARNING: \tFF Local Shortest is bugged!!!!");

               if ((_exit_strat_number == 8) || (_exit_strat_number == 9)){
                   Log->Write("\nINFO: \tUsing FF Global Shortest Router");
               }
               else {
                   Log->Write("\nWARNING: \tExit Strategy Number is not 8 or 9!!!");
                   // config object holds default values, so we omit any set operations
               }

               //check if the exit strat is [8 | 9]

               ///Parsing additional options
//               if (!ParseFfRouterOps(e)) {
//                    return false;
//               }
          }
          else if ((strategy == "ff_quickest") &&
                   (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) ) {
               Router *r = new FFRouter(id, ROUTING_FF_QUICKEST, hasSpecificGoals, _config);
               _config->GetRoutingEngine()->AddRouter(r);
               Log->Write("\nINFO: \tUsing FF Quickest Router");

               if (!ParseFfRouterOps(e, ROUTING_FF_QUICKEST)) {
                    return false;
               }
          }
          else if (std::find(usedRouter.begin(), usedRouter.end(), id) != usedRouter.end()) {
               Log->Write("ERROR: \twrong value for routing strategy [%s]!!!\n",
                         strategy.c_str());
               return false;
          }
     }
     return true;
}

bool IniFileParser::ParseFfRouterOps(TiXmlNode* routingNode, RoutingStrategy s) {
     //set defaults
     std::string mode;
     if (s == ROUTING_FF_GLOBAL_SHORTEST) {
          mode = "global_shortest";
     }
     if (s == ROUTING_FF_QUICKEST) {
          mode = "quickest";
          //parse ini-file-information
          if (routingNode->FirstChild("parameters")) {
               TiXmlNode* pParameters = routingNode->FirstChild("parameters");
               if (pParameters->FirstChild("recalc_interval")) { //@todo: ar.graf: test ini file with recalc value
                    _config->set_recalc_interval(atof(pParameters->FirstChild("recalc_interval")->FirstChild()->Value()));
               }
          }
     }
     _config->set_write_VTK_files(false);
     if (routingNode->FirstChild("parameters")) {
          TiXmlNode* pParametersForAllFF = routingNode->FirstChild("parameters");
          if (pParametersForAllFF->FirstChild("write_VTK_files"))  {
               //remark: std::strcmp returns 0 if the strings are equal
               bool tmp_write_VTK = !std::strcmp(pParametersForAllFF->FirstChild("write_VTK_files")->FirstChild()->Value(), "true");
               _config->set_write_VTK_files(tmp_write_VTK);
          }
     }
     FFRouter* r = static_cast<FFRouter*>(_config->GetRoutingEngine()->GetAvailableRouters().back());


     r->SetMode(mode);
     return true;
}

bool IniFileParser::ParseCogMapOpts(TiXmlNode* routingNode)
{

     TiXmlNode* sensorNode = routingNode->FirstChild();
     if (!sensorNode) {
          Log->Write("ERROR:\tNo sensors found.\n");
          return false;
     }

     /// static_cast to get access to the method 'addOption' of the SmokeRouter
     SmokeRouter* r = static_cast<SmokeRouter*>(_config->GetRoutingEngine()->GetAvailableRouters().back());

     std::vector<std::string> sensorVec;
     for (TiXmlElement* e = sensorNode->FirstChildElement("sensor"); e;
          e = e->NextSiblingElement("sensor")) {
          std::string sensor = e->Attribute("description");
          //adding Smoke Sensor specific parameters is executed in the class FDSFIreMeshStorage
          sensorVec.push_back(sensor);

          Log->Write("INFO: \tSensor <%s> added.", sensor.c_str());
     }

     r->addOption("Sensors", sensorVec);

     TiXmlElement* cogMap = routingNode->FirstChildElement("cognitive_map");

     if (!cogMap) {
          Log->Write("ERROR:\tCognitive Map not specified.\n");
          return false;
     }

     std::vector<std::string> cogMapStatus;
     cogMapStatus.push_back(cogMap->Attribute("status"));
     Log->Write("INFO: \tAll pedestrian starting with a(n) %s cognitive maps", cogMapStatus[0].c_str());
     r->addOption("CognitiveMap", cogMapStatus);

     std::vector<std::string> cogMapFiles;
     if (!cogMap->Attribute("files"))
     {
        Log->Write("WARNING:\tNo input files for the cognitive map specified!");
        return true;
     }
     cogMapFiles.push_back(cogMap->Attribute("files"));
     r->addOption("CognitiveMapFiles",cogMapFiles);

     return true;
}
#ifdef AIROUTER
bool IniFileParser::ParseAIOpts(TiXmlNode* routingNode) {
     TiXmlNode *sensorNode = routingNode->FirstChild();

     if (!sensorNode) {
          Log->Write("ERROR:\tNo sensors found.\n");
          return false;
     }

     /// static_cast to get access to the method 'addOption' of the AIRouter
     AIRouter *r = static_cast<AIRouter *>(_config->GetRoutingEngine()->GetAvailableRouters().back());

     std::vector<std::string> sensorVec;
     for (TiXmlElement *e = sensorNode->FirstChildElement("sensor"); e;
          e = e->NextSiblingElement("sensor")) {
          std::string sensor = e->Attribute("description");
          sensorVec.push_back(sensor);

          Log->Write("INFO: \tSensor <%s> added.", sensor.c_str());
     }

     r->addOption("Sensors", sensorVec);

     TiXmlElement *cogMap = routingNode->FirstChildElement("cognitive_map");

     if (!cogMap) {
          Log->Write("ERROR:\tCognitive Map not specified.\n");
          return false;
     }

     //std::vector<std::string> cogMapStatus;
     //cogMapStatus.push_back(cogMap->Attribute("status"));
     //Log->Write("INFO: \tAll pedestrian starting with a(n) %s cognitive maps", cogMapStatus[0].c_str());
     //r->addOption("CognitiveMap", cogMapStatus);

     std::vector<std::string> cogMapFiles;
     if (!cogMap->Attribute("files")) {
          Log->Write("WARNING:\tNo input files for the cognitive map specified!");
     }
     else
     {
          cogMapFiles.push_back(cogMap->Attribute("files"));
          r->addOption("CognitiveMapFiles", cogMapFiles);
          Log->Write("INFO:\tinput files for the cognitive map specified!");
     }

     //Signs
     TiXmlElement *signs = routingNode->FirstChildElement("signage");

     if (!signs)
     {
          Log->Write("INFO: \tNo signage specified");
     }
     else
     {
          r->addOption("SignFiles",std::vector<std::string>{signs->Attribute("file")});
     }

     return true;
}
#endif


bool IniFileParser::ParseLinkedCells(const TiXmlNode& linkedCellNode)
{
     if (linkedCellNode.FirstChild("linkedcells")) {
          std::string linkedcells = linkedCellNode.FirstChildElement("linkedcells")->Attribute(
                    "enabled");
          std::string cell_size = linkedCellNode.FirstChildElement("linkedcells")->Attribute(
                    "cell_size");

          if (linkedcells=="true") {
               _config->SetLinkedCellSize(atof(cell_size.c_str()));
               Log->Write(
                         "INFO: \tlinked cells enabled with size  <%.2f>", _config->GetLinkedCellSize());
               return true;
          }
          else {
               _config->SetLinkedCellSize(-1.0);
               Log->Write("WARNING: \tinvalid parameters for linkedcells");
               return false;
          }
     }
     return false;
}

bool IniFileParser::ParseStepSize(TiXmlNode& stepNode)
{
     if (stepNode.FirstChild("stepsize")) {
          const char* stepsize = stepNode.FirstChild("stepsize")->FirstChild()->Value();
          if (stepsize) {
               double tmp = 1. / _config->GetFps();
               double stepsizeDBL = atof(stepsize);
               if ( (stepNode.FirstChildElement("stepsize")->Attribute("fix")) &&
                         (std::string(stepNode.FirstChildElement("stepsize")->Attribute("fix")) == "yes") ) {
                    _config->Setdt(atof(stepsize));
                    Log->Write("INFO: \tstepsize <%f>", _config->Getdt());
                    if (tmp < _config->Getdt()) {
                         Log->Write("WARNING: \tStepsize dt = %f > %f = frameinterval.\nWARNING: \tYou should decrease stepsize or fps!", _config->Getdt(), tmp);
                    }
                    return true;
               }
               //find a stepsize, that can be multiplied by (int) to get framerate
               for (int i = 1; i < 2000; ++i) {
                    if ((tmp / i) <= stepsizeDBL) {
                         _config->Setdt(tmp / i);
                         if ((tmp/i) < stepsizeDBL) {
                              Log->Write("WARNING: \tDecreased stepsize from <%f> to <%f> to match fps", stepsizeDBL, (tmp/i));
                         }
                         Log->Write("INFO: \tstepsize <%f>", _config->Getdt());
                         return true;
                    }
               }
               //below should never execute
               _config->Setdt(stepsizeDBL);
               Log->Write("INFO: \tstepsize <%f>", _config->Getdt());
               if (tmp < _config->Getdt()) {
                    Log->Write("WARNING: \tStepsize dt = %f > %f = frameinterval. You should decrease stepsize or fps!", _config->Getdt(), tmp);
               }
               return true;
          }
     }
     return false;
}

bool IniFileParser::ParsePeriodic(TiXmlNode& Node)
{
     if (Node.FirstChild("periodic")) {
          const char* periodic = Node.FirstChild("periodic")->FirstChild()->Value();
          if (periodic)
               _config->SetIsPeriodic(atoi(periodic));
          Log->Write("INFO: \tperiodic <%d>", _config->IsPeriodic());
          return true;
     }
     else {
          _config->SetIsPeriodic(0);
     }
     return true; //default is periodic=0. If not specified than is OK
}

bool IniFileParser::ParseNodeToSolver(const TiXmlNode& solverNode)
{
     if (solverNode.FirstChild("solver")) {
          std::string solver = solverNode.FirstChild("solver")->FirstChild()->Value();
          if (solver=="euler") {
               _config->SetSolver(1);
          }
          else if (solver=="verlet")
               _config->SetSolver(2);
          else if (solver=="leapfrog")
               _config->SetSolver(3);
          else {
               Log->Write("ERROR: \twrong value [%s] for solver type\n", solver.c_str());
               return false;
          }
          Log->Write("INFO: \tpSolver <%d>", _config->GetSolver());
          return true;
     }
     return false;
}

bool IniFileParser::ParseStrategyNodeToObject(const TiXmlNode& strategyNode)
{
     // Init DirectionManager
     _directionManager = std::shared_ptr<DirectionManager>(new DirectionManager);
    //Read direction strategy
    std::string query = "exit_crossing_strategy";
     if (!strategyNode.FirstChild(query.c_str())) {
          query = "exitCrossingStrategy";
          Log->Write(
                    "ERROR:\t the keyword exitCrossingStrategy is deprecated. Please consider using \"exit_crossing_strategy\" in the ini file");
          return false;
     }

     if (strategyNode.FirstChild(query.c_str())) {
          const char* tmp =
                    strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
          int pExitStrategy;
          if (tmp) {
               pExitStrategy = atoi(tmp);

              //check for ff router to avoid exit strat <> router mismatch
              const TiXmlNode* agentsDistri = strategyNode.GetDocument()->RootElement()->FirstChild("agents")->FirstChild("agents_distribution");
              std::vector<int> usedRouter;
              for (const TiXmlElement* e = agentsDistri->FirstChildElement("group"); e;
                   e = e->NextSiblingElement("group")) {
                  int router = -1;
                  if (e->Attribute("router_id")) {
                      router = atoi(e->Attribute("router_id"));
                      if(std::find(usedRouter.begin(), usedRouter.end(), router) == usedRouter.end()) {
                          usedRouter.emplace_back(router);
                      }
                  }
              }
               //continue: check for ff router to avoid exit strat <> router mismatch
               const TiXmlNode* routeChoice = strategyNode.GetDocument()->RootElement()->FirstChild("route_choice_models");
               for (const TiXmlElement* e = routeChoice->FirstChildElement("router"); e;
                    e = e->NextSiblingElement("router")) {
                    int router_id = atoi(e->Attribute("router_id"));
                    if (!(std::find(usedRouter.begin(), usedRouter.end(), router_id) == usedRouter.end())) {
                         std::string router_descr = e->Attribute("description");
                         if (  (pExitStrategy != 9)
                               && (pExitStrategy != 8)
                               && ((router_descr == "ff_global_shortest") || (router_descr == "ff_local_shortest")
                                                                          || (router_descr == "ff_quickest") ) ) {
                             pExitStrategy = 8;
                              Log->Write("WARNING: \tChanging Exit Strategie to work with floorfield!");
                         }

                    }

               }
              _exit_strat_number = pExitStrategy;

               if (pExitStrategy == 8 || pExitStrategy ==9){
                    _config->set_write_VTK_files_direction(false);
                   if (strategyNode.FirstChild("write_VTK_files"))  {
                       const char* tmp1 =
                               strategyNode.FirstChild("write_VTK_files")->FirstChild()->Value();
                       //remark: std::strcmp returns 0 if the strings are equal
                       bool tmp_write_VTK = !std::strcmp(tmp1, "true");
                        _config->set_write_VTK_files_direction(tmp_write_VTK);
                   }

               }
               switch (pExitStrategy) {
               case 1:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionMiddlePoint());
                    break;
               case 2:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
                    break;
               case 3:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionInRangeBottleneck());
                    break;
               case 4:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionGeneral());
                    break;
               case 6:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                         return false;
                    };
                    break;
               case 7:
                    // dead end -> not supported anymore (global ff needed, but not available in 3d)
                    Log->Write("ERROR: \tExit Strategy 7 is not supported any longer. Please refer to www.jupedsim.org");
                    Log->Write("WARNING: \tChanging Exit-Strategy to #9 (Floorfields with targets within subroom)");
                    pExitStrategy = 9;
                    _exit_strat_number = 9;
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionSubLocalFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                        return false;
                    };
                    break;
               case 8:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionLocalFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                         return false;
                    };
                    break;
               case 9:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionSubLocalFloorfield());
                    if(!ParseFfOpts(strategyNode)) {
                         return false;
                    };
                    break;
               case 12:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionTrain());
                    break;

               default:
                    _directionStrategy = std::shared_ptr<DirectionStrategy>(new DirectionMinSeperationShorterLine());
                    Log->Write("ERROR:\t unknown exit_crossing_strategy <%d>", pExitStrategy);
                    Log->Write("     :\t the default <%d> will be used", 2);
                    return true;
                    break;
               }
          }
          else {
               return false;
          }
          Log->Write("INFO: \texit_crossing_strategy < %d >", pExitStrategy);
          _config->set_exit_strat(_exit_strat_number);
          _directionManager->SetDirectionStrategy(_directionStrategy);

     }

     // Read waiting
     std::string queryWaiting = "waiting_strategy";
     int waitingStrategyIndex = -1;
     if (strategyNode.FirstChild(queryWaiting)){
          if(const char* attribute = strategyNode.FirstChild(queryWaiting)->FirstChild()->Value(); attribute) {
               if(waitingStrategyIndex = xmltoi(attribute, -1); waitingStrategyIndex > -1
                    && attribute == std::to_string(waitingStrategyIndex)) {
                    switch (waitingStrategyIndex) {
                    case 1:
                         _waitingStrategy = std::shared_ptr<WaitingStrategy>(new WaitingMiddle());
                         break;
                    case 2:
                         _waitingStrategy = std::shared_ptr<WaitingStrategy>(new WaitingRandom());
                         break;
                    default:
                         _waitingStrategy = std::shared_ptr<WaitingStrategy>(new WaitingRandom());
                         Log->Write("ERROR:\t unknown waiting_strategy <%d>", waitingStrategyIndex);
                         Log->Write("     :\t the default <%d> will be used", 2);
                    }
               }
          }
     }

     if (waitingStrategyIndex < 0){
          _waitingStrategy = nullptr;

          Log->Write("INFO:\tcould not parse waiting_strategy, no waiting_strategy is used");
     }
     Log->Write("INFO:\twaiting_strategy < %d >", waitingStrategyIndex);

     _directionManager->SetWaitingStrategy(_waitingStrategy);
     _config->SetDirectionManager(_directionManager);

     return true;
}

bool IniFileParser::ParseFfOpts(const TiXmlNode &strategyNode) {

     std::string query = "delta_h";
     if (strategyNode.FirstChild(query.c_str())) {
          const char *tmp =
                    strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
          double pDeltaH = atof(tmp);
          _config->set_deltaH(pDeltaH);
          Log->Write("INFO: \tdeltaH:\t %f", pDeltaH);
     }


     query = "wall_avoid_distance";
     if (strategyNode.FirstChild(query.c_str())) {
          const char *tmp =
                    strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
          double pWallAvoidance = atof(tmp);
          _config->set_wall_avoid_distance(pWallAvoidance);
          Log->Write("INFO: \tWAD:\t %f", pWallAvoidance);
     }


     query = "use_wall_avoidance";
     if (strategyNode.FirstChild(query.c_str())) {
          std::string tmp =
                    strategyNode.FirstChild(query.c_str())->FirstChild()->Value();
          bool pUseWallAvoidance = !(tmp=="false");
          _config->set_use_wall_avoidance(pUseWallAvoidance);
         if(pUseWallAvoidance)
             Log->Write("INFO: \tUseWAD:\t yes");
         else
             Log->Write("INFO: \tUseWAD:\t no");
     }
     return true;
}
