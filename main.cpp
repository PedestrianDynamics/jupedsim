/**
 * \file        main.cpp
 * \date        Oct 10, 2014
 * \version     v0.8.1
 * \copyright   <2009-2015> Forschungszentrum Juelich GmbH. All rights reserved.
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
 * main function
 *
 * Some useful links:
 *
 *      1: <a href="http://jupedsim.org">jupedsim.org</a> <br>
 *
 **/

#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "Analysis.h"

using namespace std;
/* https://stackoverflow.com/questions/38530981/output-compiler-version-in-a-c-program#38531037 */
std::string ver_string(int a, int b, int c) {
      std::ostringstream ss;
      ss << a << '.' << b << '.' << c;
      return ss.str();
}

  std::string true_cxx =
#ifdef __clang__
   "clang++";
#else
   "g++";
#endif

  std::string true_cxx_ver =
#ifdef __clang__
    ver_string(__clang_major__, __clang_minor__, __clang_patchlevel__);
#else
    ver_string(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif

// todo: handle Visual Studio
/* #ifdef _MSC_VER */
/*     std::to_string(_MSC_VER) */
/* #endif */


int main(int argc, char **argv)
{
     Log->Write("INFO:\tCOMPILER : %s %s\n----", true_cxx.c_str(), true_cxx_ver.c_str());
     Log = new STDIOHandler();
     Log->Write("INFO:\tCOMMIT   : %s", GIT_COMMIT_HASH);
     Log->Write("INFO:\tDATE     : %s", GIT_COMMIT_DATE);
     Log->Write("INFO:\tSUBJECT  : %s", GIT_COMMIT_SUBJECT);
     Log->Write("INFO:\tBRANCH   : %s\n----", GIT_BRANCH);
     // Parsing the arguments
     ArgumentParser* args = new ArgumentParser();


     if(args->ParseArgs(argc, argv))
     {
          // get the number of file to analyse
          const vector<string>& files = args->GetTrajectoriesFiles();
          const string& path = args->GetTrajectoriesLocation();
          //path="";

          // create and initialize the analysis engine
          for (unsigned int i = 0; i < files.size(); i++)
          {
               const string& file = files[i];
               Analysis analysis = Analysis();
               Log->Write("\nINFO: \tStart Analysis for the file: %s",file.c_str());
               Log->Write("**********************************************************************");
               analysis.InitArgs(args);
               analysis.RunAnalysis(file, path);
               Log->Write("**********************************************************************");
               Log->Write("INFO: \tEnd Analysis for the file: %s\n",file.c_str());
          }
     }
     else
     {
          //Log->Write("INFO:\tFail to parse the ini file");
          Log->Write("INFO:\tFinishing...");
     }
     //do the last cleaning
     delete args;
     delete Log;

     return (EXIT_SUCCESS);
}
