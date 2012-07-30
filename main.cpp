


#include <mpi.h>
#include "geometry/Building.h"
#include "general/ArgumentParser.h"
#include "Simulation.h"
#include "mpi/MPIDispatcher.h"


/**
 * Main method to start the program.
 */
OutputHandler* Log;

int main(int argc, char **argv) {


	int ierr, myrank, numprocs;
	double starttime,endtime;
	//MPI_Status status;
	ierr = MPI_Init(&argc, &argv);
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	starttime=MPI_Wtime();


	MPIDispatcher* mpi = new MPIDispatcher(myrank, numprocs);
	// Ausgabe nach stdio oder in Datei
	//Log = new FileHandler("./Logfile.dat");
	Log = new STDIOHandler();
	// Argumente aus der Komandozeile lesen
	ArgumentParser* args = new ArgumentParser();
	args->ParseArgs(argc, argv);

	// Argmente an Simulation übergeben und entsprechende Variablen setzten
	Simulation sim = Simulation();
	sim.SetMPIDispatcher(mpi);
	sim.InitArgs(args);


	// Simulation
	Log->write("INFO: \tStart runSimulation()\n");
	int evacTime=sim.RunSimulation();
	Log->write("INFO: \tEnd runSimulation()\n");

	endtime=MPI_Wtime();
	if(myrank==0){
		double execTime=endtime-starttime;
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

	}else{
		double execTime=endtime-starttime;
		char tmp[CLENGTH];
		sprintf(tmp,"\nPedestrians [%d] threads [%d]",sim.GetNPedsGlobal(),args->GetMaxOmpThreads());
		Log->write(tmp);
		sprintf(tmp,"\nExec Time [s]     : %.2f",execTime);
		Log->write(tmp);
		sprintf(tmp,"Evac Time [s]     : %d",evacTime);
		Log->write(tmp);
		sprintf(tmp,"Real Time Factor  : %.2f X\n",evacTime/execTime);
		Log->write(tmp);
		printf(">> I [ %d] am done !\n",mpi->GetMyRank());
	}


	//do the last cleaning
	delete args;
	delete mpi;
	delete Log;

	ierr = MPI_Finalize();
	return ierr;
}
