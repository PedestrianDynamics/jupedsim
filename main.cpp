/**
 * @file    main.cpp
 * @author  U.Kemloh, A.Portz
 * @version 0.4
 * Created on: Apr 20, 2019
 * Copyright (C) <2009-2012>
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "Simulation.h"

///global unique log variable
OutputHandler* Log;

int main(int argc, char **argv) {

	time_t starttime, endtime;

	//Log = new FileHandler("./Logfile.dat");
	Log = new STDIOHandler();

	// Parsing the arguments
	ArgumentParser* args = new ArgumentParser();
	args->ParseArgs(argc, argv);

	// create and init the simulation engine
	Simulation sim = Simulation();
	sim.InitArgs(args);

	// Simulation
	time(&starttime);
	Log->write("INFO: \tStart runSimulation()\n");
	int evacTime = sim.RunSimulation();
	Log->write("INFO: \tEnd runSimulation()\n");
	time(&endtime);

	//some output
	double execTime = difftime(endtime, starttime);
	char tmp[CLENGTH];
	sprintf(tmp, "\nPedestrians [%d] threads [%d]", sim.GetNPeds(),
			args->GetMaxOpenMPThreads());
	Log->write(tmp);
	sprintf(tmp, "\nExec Time [s]     : %.2f", execTime);
	Log->write(tmp);
	sprintf(tmp, "Evac Time [s]     : %d", evacTime);
	Log->write(tmp);
	sprintf(tmp, "Real Time Factor  : %.2f X\n", evacTime / execTime);
	Log->write(tmp);

	//do the last cleaning
	delete args;
	delete Log;

	return (EXIT_SUCCESS);
}
