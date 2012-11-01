#include <getopt.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../general/xmlParser.h"
#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"


void ArgumentParser::Usage() {

	//TODO: randomize autodetect
	fprintf(stderr,
			"Usage: program options\n\n"
			"with the following options (default values in parenthesis):\n\n"
			"  [-n/--number <filepath>]         file with number of pedestrians in room (./Inputfiles/start.dat)\n"
			"  [-t/--tmax <double>]             maximal simulation time (500)\n"
			"  [-d/--dt <double>]               time step (0.001)\n"
			"  [--fps <double>]               	framerate (1.0 fps)\n"
			"  [-s/--solver <int>]              type of solver, if needed (euler)\n"
			"                                       1: euler\n"
			"                                       2: verlet\n"
			"                                       3: leapfrog\n"
			"  [-g/--geometry <string>]         path to .jul-geometry file (./Inputfiles/Engstelle.jul)\n"
			"  [-e/--exitstrategy <int>]        strategy how the direction to the exit is computed (3).\n"
			"                                       1: Middlepoint of the exit line\n"
			"                                       2: Shortest distance point of the exit line\n"
			"                                       3: Shortest distance point of the exit line, but exit is 20 cm  shorter\n"
			"  [-o/--online]                    online simulation (default offline)\n"
			"  [-l/--linkedcells [<double>]]    use of linked-cells with optional cell size (default cellsize = 2.2m)\n"
			"  [-R/--routing <int> [<string>]]             routing strategy (1):\n"
			"                                       1: local shortest path\n"
			"                                       2: global shortest path\n"
			"                                       3: quickest path\n"
			"                                       4: from file <filename.txt>\n"
			"										5: circle router\n"
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
			"                                       2: log output to file ./outputfiles/log.txt\n"
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
	pNumberFilename = "inputfiles/person.xml";
	pSolver = 1;
	pGeometryFilename = "inputfiles/geo.xml";
	pTmax = 900;
	pfps=10.0;
	pdt = 0.01;
	pExitStrategy = 2;
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
	pErrorLogFile="./Logfile.dat";
	pPathwayfile=""; // saving pedestrian path
	pTrajOutputDir="";
	pRoutingFilename="";
	pTrafficFilename="";
	pSeed=0;

#ifdef _OPENMP
	pMaxOpenmpThreads = omp_get_thread_num();
#else
	pMaxOpenmpThreads = 1;
#endif
}


string ArgumentParser::GetPersonsFilename() const {
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

double ArgumentParser::GetLinkedCellSize() const{
	return pLinkedCellSize;
}
unsigned int ArgumentParser::GetSeed() const {
	return pSeed;
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

string ArgumentParser::GetTrafficFile() const {
	return pTrafficFilename;
}

string ArgumentParser::GetRoutingFile() const{
	return pRoutingFilename;
}

int ArgumentParser::GetMaxOmpThreads() const{
	return pMaxOpenmpThreads;
}


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
			case 'R':
			{
				int r = atoi(optarg);
				if (r < 6)
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
				string inifile="ini.xml";
				if (optarg)
					inifile=optarg;
				Log->write("INFO: \t Loading initialization file <"+inifile+">");
				ParseIniFile(inifile);
			}
			break;

			case 'p':
				pMaxOpenmpThreads = atof(optarg);
#ifdef _OPENMP
				omp_set_num_threads(pMaxOpenmpThreads);
#endif
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

	XMLNode xMainNode=XMLNode::openFileHelper(inifile.c_str(),"JPSgcfm");

	Log->write("INFO: \tParsing the ini file");
	//I just assume all parameter are present

	//seed
	if(!xMainNode.getChildNode("seed").isEmpty()){
		const char* seed=xMainNode.getChildNode("seed").getText();
		pSeed=atoi(seed);
		srand(pSeed);
		Log->write("INFO: \tseed <"+string(seed)+">");
	}

	//geometry
	if(!xMainNode.getChildNode("geometry").isEmpty()){
		pGeometryFilename=xMainNode.getChildNode("geometry").getText();
		Log->write("INFO: \tgeometry <"+string(pGeometryFilename)+">");
	}

	//persons and distributions
	if(!xMainNode.getChildNode("person").isEmpty()){
		pNumberFilename=xMainNode.getChildNode("person").getText();
		Log->write("INFO: \tperson <"+string(pNumberFilename)+">");
	}

	//routing
	if(!xMainNode.getChildNode("routing").isEmpty()){
		pRoutingFilename=xMainNode.getChildNode("routing").getText();
		Log->write("INFO: \trouting <"+string(pRoutingFilename)+">");
	}

	//traffic
	if(!xMainNode.getChildNode("traffic").isEmpty()){
		pTrafficFilename=xMainNode.getChildNode("traffic").getText();
		Log->write("INFO: \ttraffic <"+string(pTrafficFilename)+">");
	}

	//logfile
	if(!xMainNode.getChildNode("logfile").isEmpty()){

		pErrorLogFile=xMainNode.getChildNode("logfile").getText();
		pLog=2;
		Log->write("INFO: \tlogfile <"+string(pErrorLogFile)+">");
	}

	//trajectories
	if(!xMainNode.getChildNode("trajectories").isEmpty()){
		pTrajOutputDir=xMainNode.getChildNode("trajectories").getText();
		pTravisto=1;
		Log->write("INFO: \toutput directory  <"+string(pTrajOutputDir)+">");
	}

	//model parameters, only one node
	XMLNode xPara=xMainNode.getChildNode("parameters");
	if(xPara.isEmpty()){
		Log->write("INFO: \tno gcfm parameter values found");
		return;
	}

	//tmax
	if(!xPara.getChildNode("tmax").isEmpty()){
		const char* tmax=xPara.getChildNode("tmax").getText();
		const char* unit=xPara.getChildNode("tmax").getAttribute("unit");
		pTmax=atof(tmax);
		Log->write("INFO: \tpTmax <"+string(tmax)+" " +unit +" (unit ignored)>");
	}

	//solver
	if(!xPara.getChildNode("solver").isEmpty()){
		string solver=string(xPara.getChildNode("solver").getText());
		if(solver=="euler") pSolver=1;
		else if(solver=="verlet") pSolver=2;
		else if(solver=="leapfrogr") pSolver=3;
		else {
			Log->write("ERROR: \twrong value for solver type!!!\n");
			exit(0);
		}
		Log->write("INFO: \tpSolver <"+string(solver)+">");
	}

	//stepsize
	if(!xPara.getChildNode("stepsize").isEmpty()){
		const char* stepsize=xPara.getChildNode("stepsize").getText();
		pdt=atof(stepsize);
		Log->write("INFO: \tstepsize <"+string(stepsize)+">");
	}

	//linked-cells
	if(!xPara.getChildNode("linkedcells").isEmpty()){
		string linkedcells=string(xPara.getChildNode("linkedcells").getAttribute("enabled"));
		string cell_size=string(xPara.getChildNode("linkedcells").getAttribute("cell_size"));
		if(linkedcells=="true"){
			pLinkedCells=true;
			pLinkedCellSize=atoi(cell_size.c_str());
			Log->write("INFO: \tlinked cells enable with size  <"+cell_size+">");
		}else{

			Log->write("WARNING: \tinvalid parameters for linkedcells");
		}
	}

	//route choice strategy
	if(!xPara.getChildNode("routeChoiceStrategy").isEmpty()){
		string strategy=string(xPara.getChildNode("routeChoiceStrategy").getText());

		if(strategy=="local_shortest") pRouter=1;
		else if(strategy=="global_shortest") pRouter=2;
		else if(strategy=="quickest") pRouter=3;
		else if(strategy=="dynamic") pRouter=4;
		else if(strategy=="from_file") pRouter=5;
		else if(strategy=="dummy") pRouter=6;
		else{
			Log->write("ERROR: \twrong value for routing strategy!!!\n");
			exit(0);
		}
		Log->write("INFO: \trouting  <"+string(strategy)+">");
	}

	//desired speed
	if(!xPara.getChildNode("v0").isEmpty()){
		string mu=string(xPara.getChildNode("v0").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("v0").getAttribute("sigma"));
		pV0Mu=atof(mu.c_str());
		pV0Sigma=atof(sigma.c_str());
		Log->write("INFO: \tdesired velocity mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//bmax
	if(!xPara.getChildNode("bmax").isEmpty()){
		string mu=string(xPara.getChildNode("bmax").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("bmax").getAttribute("sigma"));
		pBmaxMu=atof(mu.c_str());
		pBmaxSigma=atof(sigma.c_str());
		Log->write("INFO: \tBmax mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//bmin
	if(!xPara.getChildNode("bmin").isEmpty()){
		string mu=string(xPara.getChildNode("bmin").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("bmin").getAttribute("sigma"));
		pBminMu=atof(mu.c_str());
		pBminSigma=atof(sigma.c_str());
		Log->write("INFO: \tBmin mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//amin
	if(!xPara.getChildNode("amin").isEmpty()){
		string mu=string(xPara.getChildNode("amin").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("amin").getAttribute("sigma"));
		pAminMu=atof(mu.c_str());
		pAminSigma=atof(sigma.c_str());
		Log->write("INFO: \tAmin mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//tau
	if(!xPara.getChildNode("tau").isEmpty()){
		string mu=string(xPara.getChildNode("tau").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("tau").getAttribute("sigma"));
		pTauMu=atof(mu.c_str());
		pTauSigma=atof(sigma.c_str());
		Log->write("INFO: \tTau mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//atau
	if(!xPara.getChildNode("atau").isEmpty()){
		string mu=string(xPara.getChildNode("atau").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("atau").getAttribute("sigma"));
		pAtauMu=atof(mu.c_str());
		pAtauSigma=atof(sigma.c_str());
		Log->write("INFO: \tAtau mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//pExitStrategy
	if(!xPara.getChildNode("exitStrategy").isEmpty()){
		const char* e=xPara.getChildNode("exitStrategy").getText();
		int ie = atoi(e);
		if (ie == 1 || ie == 2 || ie == 3)
		{
			pExitStrategy = ie;
			Log->write("INFO: \texitStrategy <"+ string(e) +">");
		}
		else
		{
			pExitStrategy = 2;

			Log->write("WARNING: \twrong value for exitStrategy. Use strategy 2\n");
		}

	}
	//force_ped
	if(!xPara.getChildNode("force_ped").isEmpty()){
		string nu=string(xPara.getChildNode("force_ped").getAttribute("nu"));
		string dist_max=string(xPara.getChildNode("force_ped").getAttribute("dist_max"));
		string disteff_max=string(xPara.getChildNode("force_ped").getAttribute("disteff_max"));
		string interpolation_width=string(xPara.getChildNode("force_ped").getAttribute("interpolation_width"));
		pMaxFPed=atof(dist_max.c_str());
		pNuPed=atof(nu.c_str());
		pDistEffMaxPed=atof(disteff_max.c_str());
		pIntPWidthPed=atof(interpolation_width.c_str());
		Log->write("INFO: \tfrep_ped mu=" +nu +", dist_max="+dist_max+", disteff_max="
				+ disteff_max+ ", interpolation_width="+interpolation_width);
	}
	//force_wall
	if(!xPara.getChildNode("force_wall").isEmpty()){
		string nu=string(xPara.getChildNode("force_wall").getAttribute("nu"));
		string dist_max=string(xPara.getChildNode("force_wall").getAttribute("dist_max"));
		string disteff_max=string(xPara.getChildNode("force_wall").getAttribute("disteff_max"));
		string interpolation_width=string(xPara.getChildNode("force_wall").getAttribute("interpolation_width"));
		pMaxFWall=atof(dist_max.c_str());
		pNuWall=atof(nu.c_str());
		pDistEffMaxWall=atof(disteff_max.c_str());
		pIntPWidthWall=atof(interpolation_width.c_str());
		Log->write("INFO: \tfrep_wall mu=" +nu +", dist_max="+dist_max+", disteff_max="
				+ disteff_max+ ", interpolation_width="+interpolation_width);
	}
	Log->write("INFO: \tdone parsing ini");
}
