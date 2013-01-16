

#include "Analysis.h"

/************************************************
// Konstruktoren
 ************************************************/

Analysis::Analysis() {
	pNPeds = 0; // number of pedestrians
	pTmax = 0;
	pDt = 0;
	pBuilding = NULL;
	iod = new IODispatcher();
	fps=1;
}

Analysis::~Analysis() {
	delete pBuilding;
	delete iod;
}

/************************************************
// Setter-Funktionen
 ************************************************/


int Analysis::SetNPeds(int i) {
	return pNPeds = i;
}


/************************************************
// Getter-Funktionen
 ************************************************/


int Analysis::GetNPeds() const {
	return pNPeds;
}

Building * Analysis::GetBuilding() const {
	return pBuilding;
}

/************************************************
// Sonstige-Funktionen
 ************************************************/

/* bekommt alle Konsolenoptionen vom ArgumentParser
 * und setzt die entsprechenden Parameter in der Simulation
 * */
void Analysis::InitArgs(ArgumentParser* args) {
	char tmp[CLENGTH];
	string s = "Parameter:\n";

	switch (args->GetLog()) {
		case 0:
			// no log file
			//Log = new OutputHandler();
			break;
		case 1:
			if(Log) delete Log;
			Log = new STDIOHandler();
			break;
		case 2:
		{
			char name[CLENGTH]="";
			sprintf(name,"%s.P0.dat",args->GetErrorLogFile().c_str());
			if(Log) delete Log;
			Log = new FileHandler(name);
		}
		break;
		default:
			printf("Wrong option for Logfile!\n\n");
			exit(0);
	}

	Log->Write("INFO: \tOptionen an Simulation geben\n");


	if(args->GetTrajectoriesFile().empty()==false)
	{
		switch (args->GetFileFormat())
		{
		case FORMAT_XML_PLAIN:
		{
			OutputHandler* tofile = new FileHandler(args->GetTrajectoriesFile().c_str());
			iod->AddIO(tofile);
			break;
		}
		case FORMAT_XML_BIN:
		{
			Log->Write("INFO: \tFormat xml-bin not yet supported\n");
			exit(0);
			break;
		}
		case FORMAT_VTK:
		{
			Log->Write("INFO: \tFormat vtk not yet supported\n");
			exit(0);
			break;
		}
		}

	}



	// IMPORTANT: do not change the order in the following..
	pBuilding = new Building();

	sprintf(tmp, "\tGeometrie: [%s]\n", args->GetGeometryFilename().c_str());
	s.append(tmp);
	Log->Write("INFO: \t" + s);
	pBuilding->LoadBuilding(args->GetGeometryFilename());
	pBuilding->AddSurroundingRoom();
	pBuilding->InitGeometry(); // create the polygones


	//pBuilding->WriteToErrorLog();
}



int Analysis::RunAnalysis() {
	int frameNr = 1; // Frame Number
	int writeInterval = (int) ((1. / fps) / pDt + 0.5);
	writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
	double t;


	// writing the header

	iod->WriteHeader(pNPeds, fps, pBuilding);
	iod->WriteGeometry(pBuilding);
	iod->WriteFrame(0, pBuilding);


	// writing the footer
	iod->WriteFooter();

	//return the evacuation time
	return (int) t;
}

