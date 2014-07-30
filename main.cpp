/**
 * \file        main.cpp
 * \date        Jan 15, 2013
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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
 *
 **/


#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "./Simulation.h"




int main(int argc, char **argv)
{
     time_t starttime, endtime;
     // Log = new FileHandler("./Logfile.dat");
     Log = new STDIOHandler();
     // Parsing the arguments

     ArgumentParser* args = new ArgumentParser();
     args->ParseArgs(argc, argv);

     // create and init the simulation engine
     // Simulation
     time(&starttime);
     Log->Write("INFO: \tStart runSimulation()");

     Simulation sim = Simulation();
     sim.InitArgs(args);
     int evacTime = sim.RunSimulation();
     Log->Write("\nINFO: \tEnd runSimulation()");
     time(&endtime);

     // some output
     double execTime = difftime(endtime, starttime);

     if (sim.GetPedsNumber())
          Log->Write("\nPedestrians not evacuated [%d] using [%d] threads",
                     sim.GetPedsNumber(),
                     args->GetMaxOpenMPThreads());

     Log->Write("\nExec Time [s]   : %.2f", execTime);
     Log->Write("Evac Time [s]     : %d", evacTime);
     Log->Write("Real Time Factor  : %.2f X", evacTime / execTime);
     Log->Write("Number of Threads Used  : %d", args->GetMaxOpenMPThreads());
     Log->Write("Warnings          : %d", Log->GetWarnings() );
     Log->Write("Errors            : %d", Log->GetErrors() );
     // sim.PrintStatistics();
     if (NULL == dynamic_cast<STDIOHandler*>(Log)) {
          printf("\nExec Time [s]       : %4.2f\n", execTime);
          printf("Evac Time [s]       : %d\n", evacTime);
          printf("Real Time Factor    : %.2f (X)\n", evacTime / execTime);
          printf("Number of Threads Used  : %d\n", args->GetMaxOpenMPThreads());
          printf("Warnings            : %d\n", Log->GetWarnings() );
          printf("Errors              : %d\n", Log->GetErrors() );
     }

     // do the last cleaning
     delete args;
     delete Log;

     return (EXIT_SUCCESS);
}
