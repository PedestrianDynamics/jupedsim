/**
 * \file        ArgumentParser.cpp
 * \date        Apr 20, 2009
 * \version     v0.8
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

#include "general/OpenMP.h"
#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"
#include "../pedestrian/AgentsParameters.h"
#include "../routing/global_shortest/GlobalRouter.h"
#include "../routing/quickest/QuickestPathRouter.h"
#include "../routing/smoke_router/SmokeRouter.h"
#include "../IO/IniFileParser.h"

void ArgumentParser::Usage(const std::string file)
{
     fprintf(stderr, "Usages: \n");
     fprintf(stderr, "     %s  <path to file>  start the simulation with the specified file.\n", file.c_str());
     fprintf(stderr, "     %s                  search and use the file ini.xml in the current directory.\n",
               file.c_str());
     fprintf(stderr, "     %s  -h/--help       display this text.\n", file.c_str());
#ifdef _JPS_AS_A_SERVICE
     fprintf(stderr, "     %s  --as-a-service -p <port nr> runs jps as a service at port <port nr>.\n", file.c_str());
#endif
     exit(EXIT_SUCCESS);
}

ArgumentParser::ArgumentParser(Configuration* config)
{
     _config = config;
}

ArgumentParser::~ArgumentParser(){}

bool ArgumentParser::ParseArgs(int argc, char** argv)
{
     //special case of the default configuration ini.xml
     if (argc==1) {
          Log->Write(
                     "INFO: \tTrying to load the default configuration from the file <ini.xml>");
     // first logs will go to stdout
          Log->Write("----\nJuPedSim - JPScore\n");
          Log->Write("Current date   : %s %s", __DATE__, __TIME__);
          Log->Write("Version        : %s", JPSCORE_VERSION);
          // Log->Write("Compiler       : %s (%s)", true_cxx.c_str(), true_cxx_ver.c_str());
          Log->Write("Commit hash    : %s", GIT_COMMIT_HASH);
          Log->Write("Commit date    : %s", GIT_COMMIT_DATE);
          Log->Write("Branch         : %s\n----\n", GIT_BRANCH);

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
          bool status = p->Parse(argument);
          delete p;
          return status;
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
                                   _config->SetTrajectoriesFile(_config->GetProjectRootDir()+"tra.xml");
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
     }
#endif

     //more than one argument was supplied
     Usage(argv[0]);
     return false;
}
