#include "ArgumentParser.h"
#include "../IO/OutputHandler.h"

// private-Funktionen

void ArgumentParser::Usage() {
	fprintf(stderr,
			"Usage: program options\n\n"
			"with the following optional options (default values in parathesis):\n\n"
			"  [-n/--number <filepath>]         file with number of pedestrians in room (./Inputfiles/start.dat)\n"
			"  [-t/--tmax <double>]             maximal simulation time (500)\n"
			"  [-d/--dt <double>]               time step (0.001)\n"
			"  [--fps <double>]               	framerate (1.0 fps)\n"
			"  [-s/--solver <int>]              type of solver (euler)\n"
			"                                       1: Euler\n"
			"                                       2: Velocity Verlet\n"
			"                                       3: Leapfrog\n"
			"  [-g/--geometry <string>]         path to .jul-geometry file (./Inputfiles/Engstelle.jul)\n"
			"  [-e/--exitstrategy <int>]        strategy how the direction to the exit is computed (3).\n"
			"                                       1: Middlepoint of the exit line\n"
			"                                       2: Shortest distance point of the exit line\n"
			"                                       3: Shortest distance point of the exit line, but exit is 20 cm  shorter\n"
			"  [-r/--randomize <int>]           distribute randomly peds in the start area (1):\n"
			"                                       1: 2-D distribution per Subroom (3 columns in start.dat!!!)\n"
			"                                       2: 2-D distribution per Room (2 columns in start.dat!!!)\n"
			"  [-o/--online]                    online simulation (default offline)\n"
			"  [-l/--linkedcells [<double>]]    use of linked-cells with optional cell size (default cellsize = 2.2m)\n"
			"  [-R/--routing <int>]             routing strategy (1):\n"
			"                                       1: Not longer available\n"
			"                                       2: gloabal shortest path\n"
			"                                       3: quickest path\n"
			"										4: circle router\n"
			"  [-v/--v0mu <double>]             mu for normal distribution of v0, desired velocity (1.24)\n"
			"  [-V/--v0sigma <double>]          sigma for normal distribution of v0, desired velocity (0.26)\n"
			"  [-a/--ataumu <double>]           mu for normal distribution of a_tau, factor for velocity 1st axis (0.53)\n"
			"  [-A/--atausigma <double>]        sigma for normal distribution of a_tau, factor for velocity 1st axis (0.001)\n"
			"  [-z/--aminmu <double>]           mu for normal distribution of a_min, minimal size 1st axis (0.18)\n"
			"  [-Z/--aminsigma <double>]        sigma for normal distribution of a_min, minimal size 1st axis (0.001)\n"
			"  [-b/--bmaxmu <double>]           mu for normal distribution of b_max, maximal size 2nd axis (0.25)\n"
			"  [-B/--bmaxsigma <double>]        sigma for normal distribution of b_max, maximal size 2nd axis (0.001)\n"
			"  [-y/--bminmu <double>]           mu for normal distribution of b_min, minimal size 2nd axis (0.2)\n"
			"  [-Y/--bminsigma <double>]        sigma for normal distribution of b_min, minimal size 2nd axis (0.001)\n"
			"  [-c/--taumu <double>]            mu for normal distribution of tau, reaction time (0.5)\n"
			"  [-C/--tausigma <double>]         sigma for normal distribution of tau, reaction time (0.001)\n"
			"  [-x/--nuped <double>]            strength of forces between pedestrians, factor (0.3)\n"
			"  [-X/--nuwall <double>]           strength of wall forces, factor (0.2)\n"
			"  [-i/--intpwidthped <double>]     interpolation width of pedestrian forces (0.1)\n"
			"  [-I/--intpwidthwall <double>]    interpolation width of wall forces (0.1)\n"
			"  [-m/--maxfped <double>]          interpolation parameter of pedestrian forces (3)\n"
			"  [-M/--maxfwall <double>]         interpolation parameter of wall forces (3)\n"
			"  [-f/--disteffmaxped  <double>]   maximal effective distance to pedestrians (2)\n"
			"  [-F/--disteffmaxwall <double>]   maximal effective distance to walls (2)\n"
			"  [--pathway <filepath>]   		file for saving pedestrians path\n"
			"  [-L/--log <int>]                 log output (0):\n"
			"                                       0: no output\n"
			"                                       1: log output to stdout\n"
			"                                       2: log output to file ./Logfile.dat\n"
			"  [-T/--travisto <int>]            TraVisTo output (0):\n"
			"                                       0: no output\n"
			"                                       1: TraVisTo output to file ./Output.xml\n"
			"                                       2: TraVisTo output to file ./Output.xml AND online\n"
			"  [-h/--help]                      this manual output\n"
			"\n");
	exit(0);
}

ArgumentParser::ArgumentParser() {
	// Default parameter values
	pNumberFilename = "Inputfiles/start.dat";
	pSolver = 1;
	pGeometryFilename = "Inputfiles/Engstelle.jul";
	pTmax = 900;
	pfps=1.0;
	pdt = 0.01;
	pExitStrategy = 2;
	pRandomize = 2;
	pRouter = 1;
	pLinkedCells = false;
	pLinkedCellSize=2.2;
	pV0Mu = 1.24;
	pV0Sigma = 0.26;
	pBmaxMu = 0.25;
	pBmaxSigma = 0.001;
	pBminMu = 0.2;
	pBminSigma = 0.001;
	pAtauMu = 0.53;
	pAtauSigma = 0.001;
	pAminMu = 0.18;
	pAminSigma = 0.001;
	pNuPed = 0.2;
	pNuWall = 0.2;
	pIntPWidthPed = 0.1;
	pIntPWidthWall = 0.1;
	pMaxFPed = 3;
	pMaxFWall = 3;
	pDistEffMaxPed = 0.8;
	pDistEffMaxWall = 2;
	pTauMu = 0.5;
	pTauSigma = 0.001;
	pLog = 0;
	pTravisto = 0;
	pTrajektorien = 0;
	pErrorLogFile="./Logfile.dat";
	pPathwayfile=""; // saving pedestrian path
	pTrajOutputDir="";
	pDoorsStateFile=""; //"doors_states.txt";
	pRoomsStateFile=""; //"rooms_states.txt";
	pEvacuationType="emergency";
	pSeed=0;
	pScenarioID=0;
	pMaxOpenmpThreads=omp_get_max_threads();
}

// Getter-Funktionen

string ArgumentParser::GetNumberFilename() const {
	return pNumberFilename;
}

int ArgumentParser::GetSolver() const {
	return pSolver;
}

double ArgumentParser::GetTmax() const {
	return pTmax;
}

double ArgumentParser::Getdt() const {
	return pdt;
}

double ArgumentParser::Getfps() const {
	return pfps;
}

string ArgumentParser::GetGeometryFilename() const {
	return pGeometryFilename;
};

int ArgumentParser::GetExitStrategy() const {
	return pExitStrategy;
}

int ArgumentParser::GetRandomize() const {
	return pRandomize;
}

bool ArgumentParser::GetLinkedCells() const {
	return pLinkedCells;
}

int ArgumentParser::GetRoutingStrategy() const {
	return pRouter;
}

double ArgumentParser::GetV0Mu() const {
	return pV0Mu;
}

double ArgumentParser::GetV0Sigma() const {
	return pV0Sigma;
}

double ArgumentParser::GetBmaxMu() const {
	return pBmaxMu;
}

double ArgumentParser::GetBmaxSigma() const {
	return pBmaxSigma;
}

double ArgumentParser::GetBminMu() const {
	return pBminMu;
}

double ArgumentParser::GetBminSigma() const {
	return pBminSigma;
}

double ArgumentParser::GetAtauMu() const {
	return pAtauMu;
}

double ArgumentParser::GetAtauSigma() const {
	return pAtauSigma;
}

double ArgumentParser::GetAminMu() const {
	return pAminMu;
}

double ArgumentParser::GetAminSigma() const {
	return pAminSigma;
}

double ArgumentParser::GetNuPed() const {
	return pNuPed;
}

double ArgumentParser::GetNuWall() const {
	return pNuWall;
}

double ArgumentParser::GetIntPWidthPed() const {
	return pIntPWidthPed;
}

double ArgumentParser::GetIntPWidthWall() const {
	return pIntPWidthWall;
}

double ArgumentParser::GetMaxFPed() const {
	return pMaxFPed;
}

double ArgumentParser::GetMaxFWall() const {
	return pMaxFWall;
}

double ArgumentParser::GetDistEffMaxPed() const {
	return pDistEffMaxPed;
}

double ArgumentParser::GetDistEffMaxWall() const {
	return pDistEffMaxWall;
}

double ArgumentParser::GetTauMu() const {
	return pTauMu;
}

double ArgumentParser::GetTauSigma() const {
	return pTauSigma;
}

int ArgumentParser::GetLog() const {
	return pLog;
}

int ArgumentParser::GetTravisto() const {
	return pTravisto;
}

int ArgumentParser::GetTrajektorien() const {
	return pTrajektorien;
}

double ArgumentParser::GetLinkedCellSize() const{
	return pLinkedCellSize;
}
unsigned int ArgumentParser::GetSeed() const {
	return pSeed;
}

unsigned int ArgumentParser::GetScenarioID() const {
	return pScenarioID;
}

string ArgumentParser::GetPathwayFile() const {
	return pPathwayfile;
}


string ArgumentParser::GetTrajOutputDir() const {
	return pTrajOutputDir;
}

string ArgumentParser::GetErrorLogFile() const {
	return pErrorLogFile;
}

string ArgumentParser::GetRoomsStateFile() const {
	return pRoomsStateFile;
}

string ArgumentParser::GetDoorsStateFile() const{
	return pDoorsStateFile;
}

string ArgumentParser::GetEvacuationType() const{
	return pEvacuationType;
}

int ArgumentParser::GetMaxOmpThreads() const{
	return pMaxOpenmpThreads;
}
// Sonstige:

void ArgumentParser::ParseArgs(int argc, char **argv) {
	int c;
	int option_index = 0;
	static struct option long_options[] = {
			{"number", 1, 0, 'n'},
			{"tmax", 1, 0, 't'},
			{"dt", 1, 0, 'd'},
			{"fps", 1, 0, 'D'},
			{"solver", 1, 0, 's'},
			{"geometry", 1, 0, 'g'},
			{"exitstrategy", 1, 0, 'e'},
			{"randomize", 1, 0, 'r'},
			{"routing", 1, 0, 'R'},
			{"linkedcells", optional_argument, 0, 'l'},
			{"maxompthreads", 1, 0, 'p'},
			{"v0mu", 1, 0, 'v'},
			{"v0sigma", 1, 0, 'V'},
			{"ataumu", 1, 0, 'a'},
			{"atausigma", 1, 0, 'A'},
			{"aminmu", 1, 0, 'z'},
			{"aminsigma", 1, 0, 'Z'},
			{"bmaxmu", 1, 0, 'b'},
			{"bmaxsigma", 1, 0, 'B'},
			{"bminmu", 1, 0, 'y'},
			{"bminsigma", 1, 0, 'Y'},
			{"nuped", 1, 0, 'x'},
			{"nuwall", 1, 0, 'X'},
			{"intpwidthped", 1, 0, 'i'},
			{"intpwidthwall", 1, 0, 'I'},
			{"maxfped", 1, 0, 'm'},
			{"maxfwall", 1, 0, 'M'},
			{"disteffmaxped", 1, 0, 'f'},
			{"disteffmaxwall", 1, 0, 'F'},
			{"taumu", 1, 0, 'c'},
			{"tausigma", 1, 0, 'C'},
			{"log", 1, 0, 'L'},
			{"pathway", 1, 0, 'Q'},
			{"travisto", 1, 0, 'T'},
			{"trajectory", 1, 0, 'O'},
			{"help", 0, 0, 'h'},
			{"inifile", optional_argument, 0, 'q'},
			{0, 0, 0, 0}
	};

	while ((c = getopt_long_only(argc, argv,
			"n:t:d:s:g:e:r:R:l:p:v:V:a:A:z:Z:b:B:y:Y:x:X:i:I:m:M:f:F:c:C:L:T:O:h:q:D:Q",
			long_options, &option_index)) != -1) {
		switch (c) {
			case 'O':
			{
				pTrajektorien = atoi(optarg);
				break;
			}
			case 'T':
			{
				pTravisto = atoi(optarg);
				break;
			}
			case 'L':
			{
				pLog = atoi(optarg);
				break;
			}
			case 'c':
			{
				pTauMu = atof(optarg);
				break;
			}
			case 'C':
			{
				pTauSigma = atof(optarg);
				break;
			}
			case 'f':
			{
				pDistEffMaxPed = atof(optarg);
				break;
			}
			case 'F':
			{
				pDistEffMaxWall = atof(optarg);
				break;
			}
			case 'm':
			{
				pMaxFPed = atof(optarg);
				break;
			}
			case 'M':
			{
				pMaxFWall = atof(optarg);
				break;
			}
			case 'i':
			{
				pIntPWidthPed = atof(optarg);
				break;
			}
			case 'I':
			{
				pIntPWidthWall = atof(optarg);
				break;
			}
			case 'x':
			{
				pNuPed = atof(optarg);
				break;
			}
			case 'X':
			{
				pNuWall = atof(optarg);
				break;
			}
			case 'z':
			{
				pAminMu = atof(optarg);
				break;
			}
			case 'Z':
			{
				pAminSigma = atof(optarg);
				break;
			}
			case 'a':
			{
				pAtauMu = atof(optarg);
				break;
			}
			case 'A':
			{
				pAtauSigma = atof(optarg);
				break;
			}
			case 'y':
			{
				pBminMu = atof(optarg);
				break;
			}
			case 'Y':
			{
				pBminSigma = atof(optarg);
				break;
			}
			case 'b':
			{
				pBmaxMu = atof(optarg);
				break;
			}
			case 'B':
			{
				pBmaxSigma = atof(optarg);
				break;
			}
			case 'v':
			{
				pV0Mu = atof(optarg);
				break;
			}
			case 'V':
			{
				pV0Sigma = atof(optarg);
				break;
			}
			case 'n':
			{
				pNumberFilename = optarg;
				break;
			}
			case 'D':
			{
				pfps=atof(optarg);
				break;
			}
			case 'Q':
			{
				pPathwayfile = optarg;
				break;
			}
			case 't':
			{
				double t = atof(optarg);
				if (t > 0)
					pTmax = t;
				else {
					Log->write("ERROR: \tin ArgumentParser::ParseArgs() "
							"tmax has to be positiv!!!\n");
					exit(0);
				}
				break;
			}
			case 'd':
			{
				double d = atof(optarg);
				if (d > 0)
					pdt = d;
				else {
					Log->write("ERROR: \tin ArgumentParser::ParseArgs() "
							"dt has to be positiv!!!\n");
					exit(0);
				}
				break;
			}
			case 's':
			{
				int s = atoi(optarg);
				if (s == 1 || s==2 || s==3) // spaeter erweitern
					pSolver = s;
				else {
					Log->write("ERROR: \tin ArgumentParser::ParseArgs() "
							"wrong value for solver type!!!\n");
					exit(0);
				}
				break;
			}
			case 'g':
				pGeometryFilename = optarg;
				break;
			case 'e':
			{
				int e = atoi(optarg);
				if (e == 1 || e == 2 || e == 3)
					pExitStrategy = e;
				else {
					Log->write("ERROR: \tin ArgumentParser::ParseArgs() "
							"wrong value for exit strategy!!!\n");
					exit(0);
				}
				break;
			}
			case 'r':
			{
				int r = atoi(optarg);
				if (r == 1 || r==2)
					pRandomize = r;
				else {
					Log->write("ERROR: \tin ArgumentParser::ParseArgs() "
							"wrong value for randomize strategy!!!\n");
					exit(0);
				}
				break;
			}
			case 'R':
			{
				int r = atoi(optarg);
				if (r < 5)
					pRouter = r;
				else {
					Log->write("ERROR: \tin ArgumentParser::ParseArgs() "
							"wrong value for routing strategy!!!\n");
					exit(0);
				}
				break;
			}
			case 'l':
			{
				pLinkedCells = true;
				if (optarg)
					pLinkedCellSize=atof(optarg);
				break;
			}
			break;

			case 'q':
			{
				string inifile="ini.txt";
				if (optarg)
					inifile=optarg;
				cout<<"inifile: "<<inifile<<endl;
				ParseIniFile(inifile);
			}
			break;

			case 'p':
				pMaxOpenmpThreads = atof(optarg);
				break;

			case 'h':
				Usage();
				break;
			default:
			{
				Log->write("ERROR: \tin ArgumentParser::ParseArgs() "
						"wrong program options!!!\n");
				Usage();
				exit(0);
			}
		}
	}
}




void ArgumentParser::ParseIniFile(string inifile){
	ifstream infile;
	string line;
	string output;
	int lineCount=0;

	infile.open(inifile.c_str(), fstream::in);
	if (infile.is_open()==false) {
		Log->write("WARNING: \tCannot load ini.txt");
		Log->write("WARNING: \tWorking with default values");
		return;
	}

	//get the error log output file
	if(NULL==getline(infile, line)) {
		Log->write("ERROR: \t Could not get the error logs output directory");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pErrorLogFile;
		lineCount++;
	}

	//get the  geometry file
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get the input directory");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pGeometryFilename;
		lineCount++;
	}

	//get the person counting system file
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get the initial distribution file");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pNumberFilename;
		lineCount++;
	}

	//get the states of the rooms file
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get the rooms states file");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pRoomsStateFile;
		lineCount++;
	}

	//get the states of the doors
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get the doors states file");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pDoorsStateFile;
		lineCount++;
	}

	//get the outputdir
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get the output dir (for the trajectories)");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pTrajOutputDir;
		lineCount++;
	}

	//get the type of simulation ( emergency vs normal)
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get evacuation type");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pEvacuationType;
		lineCount++;
		if((pEvacuationType!="emergency") && (pEvacuationType!="normal")){
			Log->write("ERROR: \t 'emergency' or 'normal' required");
			Log->write("\tgot: " +pEvacuationType);
			exit(EXIT_FAILURE);
		}
	}


	//get the seed
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get the seed");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pSeed;
		lineCount++;
		srand(pSeed);
	}

	//get the scenarion id
	if(NULL==getline(infile, line)) {
		Log->write("WARNING: \t Could not get the scenario id");
		exit(EXIT_FAILURE);
	}else{
		istringstream iss(line, istringstream::in);
		iss>>pScenarioID;
		lineCount++;
	}

	if(lineCount!=9){
		Log->write("ERROR: \t The ini.txt is missing some inputs lines");
		exit(EXIT_FAILURE);
	}

	std::stringstream ss;
	Log->write("INFO:\t Parsing the ini file");
	ss<<pScenarioID;
	Log->write("     \t\t scenario id: "+ss.str());
	ss<<pSeed;
	Log->write("     \t\t Seed: "+ss.str());
	Log->write("     \t\t evacuation type: "+pEvacuationType);
	Log->write("     \t\t errorLogOutput: "+pErrorLogFile);
	Log->write("     \t\t geometry: "+pGeometryFilename);
	Log->write("     \t\t outputDir: "+pTrajOutputDir);
	Log->write("     \t\t PZA file: "+pNumberFilename);
	Log->write("     \t\t GMS file (doors): "+pDoorsStateFile);
	Log->write("     \t\t GMS file (rooms): "+pRoomsStateFile);
	Log->write("INFO:\t ini file parsed successfully!");
}
