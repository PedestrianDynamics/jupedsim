/**
 * \file        main.cpp
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J��lich GmbH. All rights reserved.
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
 * 	1: <a href="http://www.openpedsim.org">www.openpedsim.org</a> <br>
 * 	2: <a href="http://www.vtk.org">www.vtk.org</a> <br>
 * 	3: <a href="http://www.trolltech.com">www.trolltech.com</a> <br>
 * 	4: <a href="http://www.fz-juelich.de">www.fz-juelich.de</a> <br>
 * 	4: <a href="http://www.jupedsim.org">www.fz-juelich.de</a> <br>
 *
 **/

#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "Analysis.h"

using namespace std;

int main(int argc, char **argv)
{
     Log = new STDIOHandler();

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
