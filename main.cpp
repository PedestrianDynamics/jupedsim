


#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "Simulation.h"


/**
 * Main method to start the program.
 */
OutputHandler* Log;

int main(int argc, char **argv) {


	time_t starttime,endtime;

	//Log = new FileHandler("./Logfile.dat");
	Log = new STDIOHandler();

	// Parsing the arguments
	ArgumentParser* args = new ArgumentParser();
	args->ParseArgs(argc, argv);

	// init the simulation engine
	Simulation sim = Simulation();
	sim.InitArgs(args);

	// Simulation
	time(&starttime);
	Log->write("INFO: \tStart runSimulation()\n");
	int evacTime=sim.RunSimulation();
	Log->write("INFO: \tEnd runSimulation()\n");
	time(&endtime);

	double execTime=difftime(endtime,starttime);
	printf("\nExec Time [s]     : %.2f\n",execTime);
	printf("Evac Time [s]     : %d\n",evacTime);
	printf("Real Time Factor  : %.2f X\n\n",evacTime/execTime);

	char tmp[CLENGTH];
	sprintf(tmp,"\nPedestrians [%d] threads [%d]",sim.GetNPedsGlobal(),args->GetMaxOmpThreads());
	Log->write(tmp);
	sprintf(tmp,"\nExec Time [s]     : %.2f",execTime);
	Log->write(tmp);
	sprintf(tmp,"Evac Time [s]     : %d",evacTime);
	Log->write(tmp);
	sprintf(tmp,"Real Time Factor  : %.2f X\n",evacTime/execTime);
	Log->write(tmp);


	//do the last cleaning
	delete args;
	delete Log;

	return (EXIT_SUCCESS);
}
