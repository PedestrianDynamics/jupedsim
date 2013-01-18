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
 */

/**
* @mainpage
*
* \date 15.1.2013
*
* JuPedSim stands for Jülich Pedestrians Simulator and is currently developed at the Forschungszentrum Jülich in Germany.
*
* @image html logo.png " "
*
* Some useful links:
*
* 	1: <a href="http://www.openpedsim.org">www.openpedsim.org</a> <br>
* 	2: <a href="http://www.vtk.org">www.vtk.org</a> <br>
* 	3: <a href="http://www.trolltech.com">www.trolltech.com</a> <br>
* 	4: <a href="http://www.fz-juelich.de">www.fz-juelich.de</a> <br>
* 	4: <a href="http://www.jupedsim.org">www.fz-juelich.de</a> <br>
*
*/

#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "Analysis.h"

///global unique log variable
OutputHandler* Log;

int main(int argc, char **argv) {

	Log = new STDIOHandler();

	// Parsing the arguments
	ArgumentParser* args = new ArgumentParser();
	args->ParseArgs(argc, argv);
/*
	// create and init the simulation engine
	Analysis analysis = Analysis();
	analysis.InitArgs(args);

	// Simulation
	Log->Write("INFO: \tStart Analysis()\n");
	analysis.RunAnalysis();
	Log->Write("INFO: \tEnd Analysis()\n");
*/
	//do the last cleaning
	delete args;
	delete Log;

	return (EXIT_SUCCESS);
}
