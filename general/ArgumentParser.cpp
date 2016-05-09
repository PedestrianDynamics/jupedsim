/**
 * \file        ArgumentParser.cpp
 * \date        Apr 20, 2009
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
 *
 *
 **/
// #ifndef _MSC_VER
// #include <getopt.h>
// #endif
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

#ifdef _OPENMP

#include <omp.h>

#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif

#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"
#include "Macros.h"
#include "../pedestrian/AgentsParameters.h"
#include "../routing/GlobalRouter.h"
#include "../routing/QuickestPathRouter.h"
#include "../routing/CognitiveMapRouter.h"
#include "../IO/IniFileParser.h"
#include <sys/stat.h>

using namespace std;

void ArgumentParser::Usage(const std::string file)
{
     fprintf(stderr, "\n\nYou are actually using JuPedsim version %s  \n\n", JPS_VERSION);
     fprintf(stderr, "Usages: \n");
     fprintf(stderr, "     %s  <path to file>  start the simulation with the specified file.\n", file.c_str());
     fprintf(stderr, "     %s                  search and use the file ini.xml in the current directory.\n",
               file.c_str());
     fprintf(stderr, "     %s  -v/--version    display the current version.\n", file.c_str());
     fprintf(stderr, "     %s  -h/--help       display this text.\n", file.c_str());
#ifdef _JPS_AS_A_SERVICE
     fprintf(stderr, "     %s  --as-a-service -p <port nr> runs jps as a service at port <port nr>.\n", file.c_str());
#endif
     exit(EXIT_SUCCESS);
}

ArgumentParser::ArgumentParser(Configuration* config)
{
     _config = config;
//     // Default parameter values
//     //pNumberFilename = "inputfiles/persons.xml";
//     pSolver = 1;
//     _projectFile = "";
//     pTmax = 900;
//     pfps = 1.0;
//     pdt = 0.01;
//     pExitStrategy = 2;
//     pLinkedCells = false;
//     pLinkedCellSize = 2.2;
//     pV0Mu = 1.24;
//     pV0Sigma = 0.26;
//     pBmaxMu = 0.25;
//     pBmaxSigma = 0.001;
//     pBminMu = 0.2;
//     pBminSigma = 0.001;
//     pAtauMu = 0.53;
//     pAtauSigma = 0.001;
//     pAminMu = 0.18;
//     pAminSigma = 0.001;
//     pNuPed = 0.4;
//     pNuWall = 0.2;
//     pIntPWidthPed = 0.1;
//     pIntPWidthWall = 0.1;
//     pMaxFPed = 3;
//     pMaxFWall = 3;
//     pDistEffMaxPed = 2; //0.8
//     pDistEffMaxWall = 2;
//     pTauMu = 0.5;
//     pTauSigma = 0.001;
//     paPed=1;
//     pbPed=0.25;
//     pcPed=3;
//     paWall=1;
//     pbWall=0.7;
//     pDWall = 0.1;  //Tordeux2015
//     pDPed = 0.1; //Tordeux2015
//     pPeriodic = 0; // use only for Tordeux2015 with "trivial" geometries
//     pcWall=3;
//     pLog = 0;
//     pModel=MODEL_GFCM;
//     pErrorLogFile = "./Logfile.dat";
//     pNavMeshFilename = "";
//     pSeed = 0;
//     pFormat = FORMAT_XML_PLAIN;
//     pPort = -1;
//     _hostname = "localhost";
//     _embedMesh = 0;
//     _maxOpenMPThreads = omp_get_thread_num();
//     _profilingFlag = false;
//     _hpcFlag = 0;
//     _agentsParameters= std::map<int, std::shared_ptr<AgentsParameters> >();
//     _routingengine = std::shared_ptr<RoutingEngine>(new RoutingEngine());
//     _showStatistics=false;
}

bool ArgumentParser::ParseArgs(int argc, char** argv)
{
     //special case of the default configuration ini.xml
     if (argc==1) {
          Log->Write(
                    "INFO: \tTrying to load the default configuration from the file <ini.xml>");
          IniFileParser* p = new IniFileParser(_config);
          if (!p->Parse("ini.xml")) {
               Usage(argv[0]);
          }
          return true;
     }

     string argument = argv[1];
     if (argument=="-h" || argument=="--help") {
          Usage(argv[0]);
          return false;
     }
     else if (argument=="-v" || argument=="--version") {
          fprintf(stderr, "You are actually using JuPedsim version %s  \n\n", JPS_VERSION);
          return false;
     }

     // other special case where a single configuration file is submitted
     //check if inifile options are given
     if (argc==2) {
          string prefix1 = "--ini=";
          string prefix2 = "--inifile=";

          if (!argument.compare(0, prefix2.size(), prefix2)) {
               argument.erase(0, prefix2.size());
          }
          else if (!argument.compare(0, prefix1.size(), prefix1)) {
               argument.erase(0, prefix1.size());
          }
          IniFileParser* p = new IniFileParser(_config);
          return p->Parse(argument);
     }

#ifdef _JPS_AS_A_SERVICE //TODO try to avoid macros!
     if (argc==4 || argc==6) {

          string argument1 = argv[1];
          if (argument1=="--as-a-service") { //runs jps as a service
               string argument2 = argv[2];
               if (argument2=="-p") { //port
                    string argument3 = argv[3];  //port number at which jps is listening
                    int port = std::atoi(argument3.c_str());
                    _config->SetRunAsService(true);
                    _config->SetServicePort(port);
                    if (argc==6) {
                         string argument4 = argv[4];
                         string argument5 = argv[5];
                         if (argument4=="--dump-scenario") {
                              struct stat sb;
                              if (stat(argument5.c_str(), &sb)==0 && S_ISDIR(sb.st_mode)) {
                                   if (*(argument5.end()-1)!='/') {
                                        argument5.append("/");
                                   }
                                   _config->SetProjectRootDir(argument5);
                                   _config->SetGeometryFile("geo.xml");
                                   _config->SetProjectFile("ini.xml");
                                   _config->SetTrjectoriesFile(_config->GetProjectRootDir()+"tra.xml");
                                   _config->SetFileFormat(FileFormat::FORMAT_XML_PLAIN);
                                   _config->SetFps(25);
                                   _config->SetDumpScenario(true);
                                   return true;
                              }
                              else {
                                   std::cerr << "unable to write to: " << argument5 << std::endl;
                                   return false;
                              }
                         }
                    }
                    else {
                         return true;
                    }
               }
          }
          else {
               Log->Write("ERROR: \twrong value for routing strategy [%s]!!!\n",
                         strategy.c_str());
               return false;
          }
     }
     return true;
}

//todo: parse this in Cognitive map router
bool ArgumentParser::ParseCogMapOpts(TiXmlNode *routerNode)
{
     TiXmlNode* sensorNode=routerNode->FirstChild();

     if (!sensorNode)
     {
          Log->Write("ERROR:\tNo sensors found.\n");
          return false;
     }

     /// static_cast to get access to the method 'addOption' of the CognitiveMapRouter
     CognitiveMapRouter* r = static_cast<CognitiveMapRouter*>(_routingengine->GetAvailableRouters().back());

     std::vector<std::string> sensorVec;
     for (TiXmlElement* e = sensorNode->FirstChildElement("sensor"); e;
               e = e->NextSiblingElement("sensor"))
     {
          string sensor = e->Attribute("description");

          sensorVec.push_back(sensor);

          Log->Write("INFO: \tSensor "+ sensor + " added");
     }


     r->addOption("Sensors",sensorVec);

     TiXmlElement* cogMap=routerNode->FirstChildElement("cognitive_map");

     if (!cogMap)
     {
          Log->Write("ERROR:\tCognitive Map not specified.\n");
          return false;
     }

     std::vector<std::string> cogMapStatus;
     cogMapStatus.push_back(cogMap->Attribute("status"));
     Log->Write("INFO: \tAll pedestrian starting with a(n) "+cogMapStatus[0]+" cognitive map\n");
     r->addOption("CognitiveMap",cogMapStatus);


     return true;

}

bool ArgumentParser::ParseFfRouterOps(TiXmlNode* routingNode) {
     //set defaults
     std::string mode = "global_shortest";
     FFRouter* r = static_cast<FFRouter*>(_routingengine->GetAvailableRouters().back());

     //parse ini-file-information
     if (routingNode->FirstChild("parameters")) {
          TiXmlNode* pParameters = routingNode->FirstChild("parameters");
          if (pParameters->FirstChild("mode")) {
               mode = pParameters->FirstChild("mode")->FirstChild()->Value();
          }
     }
#endif

     //more than one argument was supplied
     Usage(argv[0]);
     return false;
}


//shared_ptr<OperationalModel> ArgumentParser::GetModel() const {
//    return p_op_model;
//}
//
//const FileFormat &ArgumentParser::GetFileFormat() const {
//    return pFormat;
//}
//
//const string &ArgumentParser::GetHostname() const {
//    return _hostname;
//}
//
//void ArgumentParser::SetHostname(const string &hostname) {
//    _hostname = hostname;
//}
//
//int ArgumentParser::GetPort() const {
//    return pPort;
//}
//
//void ArgumentParser::SetPort(int port) {
//    pPort = port;
//}
//
//int ArgumentParser::GetSolver() const {
//    return pSolver;
//}
//
//double ArgumentParser::GetTmax() const {
//    return pTmax;
//}
//
//double ArgumentParser::Getdt() const {
//    return pdt;
//}
//
//int ArgumentParser::IsPeriodic() const {
//    return pPeriodic;
//}
//
//double ArgumentParser::Getfps() const {
//    return pfps;
//}
//
//const string &ArgumentParser::GetProjectFile() const {
//    return _projectFile;
//}
//
///// @deprecated
//const string &ArgumentParser::GetNavigationMesh() const {
//    return pNavMeshFilename;
//}
//
//std::shared_ptr<DirectionStrategy> ArgumentParser::GetExitStrategy() const {
//    return p_exit_strategy;
//}
//
//bool ArgumentParser::GetLinkedCells() const {
//    return pLinkedCells;
//}
//
//std::shared_ptr<RoutingEngine> ArgumentParser::GetRoutingEngine() const {
//    return _routingengine;
//}
//
//vector<pair<int, RoutingStrategy> > ArgumentParser::GetRoutingStrategy() const {
//    return pRoutingStrategies;
//}
//
//#ifdef _HYBRID_SIMULATION
//
//std::shared_ptr<HybridSimulationManager> ArgumentParser::GetHybridSimManager() const {
//    return _hybridSimManager;
//}
//
//#endif
//
//double ArgumentParser::GetV0Mu() const {
//    return pV0Mu;
//}
//
//double ArgumentParser::GetV0Sigma() const {
//    return pV0Sigma;
//}
//
//double ArgumentParser::GetBmaxMu() const {
//    return pBmaxMu;
//}
//
//double ArgumentParser::GetBmaxSigma() const {
//    return pBmaxSigma;
//}
//
//double ArgumentParser::GetBminMu() const {
//    return pBminMu;
//}
//
//double ArgumentParser::GetBminSigma() const {
//    return pBminSigma;
//}
//
//double ArgumentParser::GetAtauMu() const {
//    return pAtauMu;
//}
//
//double ArgumentParser::GetAtauSigma() const {
//    return pAtauSigma;
//}
//
//double ArgumentParser::GetAminMu() const {
//    return pAminMu;
//}
//
//double ArgumentParser::GetAminSigma() const {
//    return pAminSigma;
//}
//
//double ArgumentParser::GetNuPed() const {
//    return pNuPed;
//}
//
//double ArgumentParser::GetaPed() const {
//    return paPed;
//}
//
//double ArgumentParser::GetbPed() const {
//    return pbPed;
//}
//
//double ArgumentParser::GetcPed() const {
//    return pcPed;
//}
//
//double ArgumentParser::GetNuWall() const {
//    return pNuWall;
//}
//
//double ArgumentParser::GetaWall() const {
//    return paWall;
//}
//
//double ArgumentParser::GetbWall() const {
//    return pbWall;
//}
//
//double ArgumentParser::GetDWall() const {
//    return pDWall;
//}
//
//double ArgumentParser::GetDPed() const {
//    return pDPed;
//}
//
//
//double ArgumentParser::GetcWall() const {
//    return pcWall;
//}
//
//double ArgumentParser::GetIntPWidthPed() const {
//    return pIntPWidthPed;
//}
//
//double ArgumentParser::GetIntPWidthWall() const {
//    return pIntPWidthWall;
//}
//
//double ArgumentParser::GetMaxFPed() const {
//    return pMaxFPed;
//}
//
//double ArgumentParser::GetMaxFWall() const {
//    return pMaxFWall;
//}
//
//double ArgumentParser::GetDistEffMaxPed() const {
//    return pDistEffMaxPed;
//}
//
//double ArgumentParser::GetDistEffMaxWall() const {
//    return pDistEffMaxWall;
//}
//
//double ArgumentParser::GetTauMu() const {
//    return pTauMu;
//}
//
//double ArgumentParser::GetTauSigma() const {
//    return pTauSigma;
//}
//
//int ArgumentParser::GetLog() const {
//    return pLog;
//}
//
//double ArgumentParser::GetLinkedCellSize() const {
//    if (pLinkedCells)
//        return pLinkedCellSize;
//    return -1;
//}
//
//unsigned int ArgumentParser::GetSeed() const {
//    return pSeed;
//}
//
//int ArgumentParser::GetEmbededMesh() const {
//    return _embedMesh;
//}
//
//const string &ArgumentParser::GetErrorLogFile() const {
//    return pErrorLogFile;
//}
//
//int ArgumentParser::GetMaxOpenMPThreads() const {
//    return _maxOpenMPThreads;
//}
//
//const string &ArgumentParser::GetTrajectoriesFile() const {
//    return pTrajectoriesFile;
//}
//
//void ArgumentParser::SetTrajectoriesFile(const string &trajectoriesFile) {
//    pTrajectoriesFile = trajectoriesFile;
//}
//
//const string &ArgumentParser::GetProjectRootDir() const {
//    return _projectRootDir;
//}
//
//bool ArgumentParser::GetProfileFlag() {
//    return _profilingFlag;
//}
//
//int ArgumentParser::GetHPCFlag() const {
//    return _hpcFlag;
//}
//
//bool ArgumentParser::ShowStatistics() const {
//    return _showStatistics;
//}

