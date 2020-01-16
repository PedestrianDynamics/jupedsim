/**
 * \file        main.cpp
 * \date        Oct 10, 2014
 * \version     v0.8.3
 * \copyright   <2009-2018> Forschungszentrum Juelich GmbH. All rights reserved.
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

#include "Analysis.h"
#include "general/ArgumentParser.h"
#include "geometry/Building.h"

#include <chrono>

using namespace std;
using namespace std::chrono;

int main(int argc, char ** argv)
{
    Log = new STDIOHandler();
    // Parsing the arguments
    ArgumentParser * args                       = new ArgumentParser();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    if(args->ParseArgs(argc, argv)) {
        // get the number of file to analyse
        const vector<fs::path> & files = args->GetTrajectoriesFiles();
        const fs::path & Path          = args->GetTrajectoriesLocation();
        // create and initialize the analysis engine
        for(unsigned int i = 0; i < files.size(); i++) {
            const fs::path & File = files[i];
            Analysis analysis     = Analysis();
            Log->Write("\nINFO: \tStart Analysis for the file: %s", File.string().c_str());
            Log->Write("**********************************************************************");
            analysis.InitArgs(args);
            analysis.RunAnalysis(File, Path);
            Log->Write("**********************************************************************");
            Log->Write("INFO: \tEnd Analysis for the file: %s\n", File.string().c_str());
            std::cout << "INFO: \tEnd Analysis for the file: " << File.string().c_str() << "\n";
        }
    } else {
        //Log->Write("INFO:\tFail to parse the ini file");
        Log->Write("INFO:\tFinishing...");
    }

    //do the last cleaning
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    float duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0;
    Log->Write("Time elapsed:\t %0.2f [s]\n", duration);

    std::cout << "Time elapsed:\t " << duration << " [s]\n";

    delete args;
    delete Log;
    return (EXIT_SUCCESS);
}
