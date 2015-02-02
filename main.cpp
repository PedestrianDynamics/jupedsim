/**
 * \file        main.cpp
 * \date        Jan 15, 2013
 * \version     v0.6
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
 * main function
 *
 *
 **/

#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "./Simulation.h"
// #include "logging.h"

int main(int argc, char **argv)
{
     //gathering some statistics about the runtime
     time_t starttime, endtime;

     // Log = new FileHandler("./Logfile.dat");
     Log = new STDIOHandler();

     // Parsing the arguments
     ArgumentParser* args = new ArgumentParser();
     bool status=args->ParseArgs(argc, argv);

     // create and initialize the simulation engine
     // Simulation
     time(&starttime);

     Simulation sim(*args);

     if(status&&sim.InitArgs(*args))
     {
          Log->Write("INFO: \tStart runSimulation()");
          int evacTime = sim.RunSimulation();
          Log->Write("\nINFO: \tEnd runSimulation()");
          time(&endtime);

          // some output
          if(args->ShowStatistics())
          {
               sim.PrintStatistics();
          }

          if (sim.GetPedsNumber())
          {
               Log->Write("WARNING: \nPedestrians not evacuated [%d] using [%d] threads",
                         sim.GetPedsNumber(), args->GetMaxOpenMPThreads());
          }

          double execTime = difftime(endtime, starttime);
          Log->Write("\nExec Time [s]   : %.2f", execTime);
          Log->Write("Evac Time [s]     : %d", evacTime);
          Log->Write("Realtime Factor   : %.2f X", evacTime / execTime);
          Log->Write("Number of Threads : %d", args->GetMaxOpenMPThreads());
          Log->Write("Warnings          : %d", Log->GetWarnings());
          Log->Write("Errors            : %d", Log->GetErrors());

          if (NULL == dynamic_cast<STDIOHandler*>(Log))
          {
               printf("\nExec Time [s]     : %4.2f\n", execTime);
               printf("Evac Time [s]       : %d\n", evacTime);
               printf("Realtime Factor     : %.2f (X)\n", evacTime / execTime);
               printf("Number of Threads   : %d\n", args->GetMaxOpenMPThreads());
               printf("Warnings            : %d\n", Log->GetWarnings());
               printf("Errors              : %d\n", Log->GetErrors());
          }
     }
     else
     {
          Log->Write("INFO:\tFinishing...");
     }
     // do the last cleaning
     delete args;
     delete Log;

     return (EXIT_SUCCESS);
}
