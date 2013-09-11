#include <getopt.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num() 0
#define omp_get_max_threads()  1
#endif


#include "../tinyxml/tinyxml.h"
#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"

using namespace std;

void ArgumentParser::Usage() {

	fprintf(stderr,
			"Usage: program options\n\n"
			"with the following options (default values in parenthesis):\n\n"
			"  [-q/--inifile <filepath>]        initialisation file( ini.xml found in the actual directory)\n"
			"  [-n/--number <filepath>]         file with the pedestrian distribution in room(./Inputfiles/persons.xml)\n"
			"  [-t/--tmax <double>]             maximal simulation time (500)\n"
			"  [-d/--dt <double>]               time step (0.001)\n"
			"  [--fps <double>]                 framerate (1.0 fps)\n"
			"  [-s/--solver <int>]              type of solver, if needed (euler)\n"
			"                                       1: euler\n"
			"                                       2: verlet\n"
			"                                       3: leapfrog\n"
			"  [-g/--geometry <string>]         path to the geometry file (./Inputfiles/geo.xml)\n"
			"  [-e/--exitstrategy <int>]        strategy how the direction to the exit is computed (3).\n"
			"                                       1: Middlepoint of the exit line\n"
			"                                       2: Shortest distance point of the exit line\n"
			"                                       3: Shortest distance point of the exit line, but exit is 20 cm  shorter\n"
			"  [-l/--linkedcells [<double>]]    use of linked-cells with optional cell size (default cellsize = 2.2m)\n"
			"  [-R/--routing <int> [<string>]]             routing strategy (1):\n"
			"                                       1: local shortest path\n"
			"                                       2: global shortest path\n"
			"                                       3: quickest path\n"
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
			"  [--pathway <filepath>]           file for saving pedestrians path\n"
			"  [-L/--log <int>]                 log output (0):\n"
			"                                       0: no output\n"
			"                                       1: log output to stdout\n"
			"                                       2: log output to file ./outputfiles/log.txt\n"
			"  [-T/--output-file <string>]      file to write the trajectories ( no trajectories are written if omited0):\n"
			"  [-P/--streaming-port <int>]      stream the ouput to the specified address/port\n"
			"  [-O/--streaming-ip <string>]     stream the ouput to the specified address/port\n"
			"  [-N/--generate-mesh <string>]    write the navigation mesh to the given file\n"
			"  [-h/--help]                      this manual output\n"
			"\n");
	exit(EXIT_SUCCESS);
}

ArgumentParser::ArgumentParser() {
	// Default parameter values
	pNumberFilename = "inputfiles/persons.xml";
	pSolver = 1;
	pGeometryFilename = "inputfiles/geo.xml";
	pTmax = 500;
	pfps=1.0;
	pdt = 0.01;
	pExitStrategy = 2;
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
	pNuPed = 0.4;
	pNuWall = 0.2;
	pIntPWidthPed = 0.1;
	pIntPWidthWall = 0.1;
	pMaxFPed = 3;
	pMaxFWall = 3;
	pDistEffMaxPed = 2; //0.8
	pDistEffMaxWall = 2;
	pTauMu = 0.5;
	pTauSigma = 0.001;
	pLog = 0;
	pErrorLogFile="./Logfile.dat";
	pPathwayFilename="";
	pRoutingFilename="";
	pTrafficFilename="";
	pNavMeshFilename="";
	pSeed=0;
	pFormat=FORMAT_XML_PLAIN;
	pPort=-1;
	pHostname="localhost";
	_embedMesh=0;
	pMaxOpenMPThreads = omp_get_thread_num();
}



void ArgumentParser::ParseArgs(int argc, char **argv) {
	int c;
	int option_index = 0;

	//special case of the default configuration
	if(argc==1){
		Log->Write("INFO: \tTrying to load the default configuration from the file <ini.xml>");
		ParseIniFile("ini.xml");
		return;
	}

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
			{"output-file", 1, 0, 'T'},
			{"streaming-port", 1, 0, 'P'},
			{"streaming-ip", 1, 0, 'O'},
			{"help", 0, 0, 'h'},
			{"inifile", optional_argument, 0, 'q'},
			{"generate-mesh", required_argument, 0, 'N'},
			{0, 0, 0, 0}
	};

	while ((c = getopt_long_only(argc, argv,
			"n:t:d:s:g:e:r:R:l:p:v:V:a:A:z:Z:b:B:y:Y:x:X:i:I:m:M:f:F:c:C:L:T:O:h:q:D:Q:N:",
			long_options, &option_index)) != -1) {

		switch (c) {
			case 'T':
			{
				if (optarg)
					pTrajectoriesFile=optarg;
				break;
			}
			case 'P':
			{
				if (optarg)
					pPort=atoi(optarg);
				break;
			}
			case 'O':
			{
				if (optarg)
					pHostname=optarg;
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
				pPathwayFilename = optarg;
				break;
			}
			case 't':
			{
				double t = atof(optarg);
				if (t > 0)
					pTmax = t;
				else {
					Log->Write("ERROR: \tin ArgumentParser::ParseArgs() "
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
					Log->Write("ERROR: \tin ArgumentParser::ParseArgs() "
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
					Log->Write("ERROR: \tin ArgumentParser::ParseArgs() "
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
				if (e == 1 || e == 2 || e == 3 || e == 4 )
					pExitStrategy = e;
				else {
					Log->Write("ERROR: \tin ArgumentParser::ParseArgs() "
							"wrong value for exit strategy!!!\n");
					exit(0);
				}
				break;
			}
			case 'R': // TODO: are these options still correct ?
			{
				int r = atoi(optarg);
				switch(r){
				case 1:
					pRoutingStrategies.push_back(make_pair (1,ROUTING_LOCAL_SHORTEST));
					break;
				case 2:
					pRoutingStrategies.push_back(make_pair (2, ROUTING_GLOBAL_SHORTEST));
					break;
				case 3:
					pRoutingStrategies.push_back(make_pair (3,ROUTING_QUICKEST));
					break;
				case 4:
					pRoutingStrategies.push_back(make_pair (4,ROUTING_DYNAMIC));
					break;
				case 5:
					pRoutingStrategies.push_back(make_pair (5,ROUTING_NAV_MESH));
					break;
				case 6:
					pRoutingStrategies.push_back(make_pair (6,ROUTING_DUMMY));
					break;
				default:
					Log->Write("ERROR: \tin ArgumentParser::ParseArgs() "
							"wrong value for routing strategy!!!\n");
					exit(0);
					break;
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
				Log->Write("INFO: \t Loading initialization file <"+inifile+">");
				ParseIniFile(inifile);
			}
			break;

			case 'p':
				pMaxOpenMPThreads = atof(optarg);
#ifdef _OPENMP
				omp_set_num_threads(pMaxOpenMPThreads);
#endif
				break;

			case 'h':
				Usage();
				break;

			case 'N':
				pNavMeshFilename=optarg;
				break;

			default:
			{
				Log->Write("ERROR: \tin ArgumentParser::ParseArgs() "
						"wrong program options!!!\n");
				Usage();
				exit(EXIT_FAILURE);
			}
		}
	}
}


void ArgumentParser::ParseIniFile(string inifile){

	Log->Write("INFO: \tParsing the ini file");

	TiXmlDocument doc(inifile);
	if (!doc.LoadFile()){
		Log->Write("ERROR: \t%s", doc.ErrorDesc());
		Log->Write("ERROR: \t could not parse the ini file");
		exit(EXIT_FAILURE);
	}

	// everything is fine. proceed with parsing

	TiXmlElement* xMainNode = doc.RootElement();
	if( ! xMainNode ) {
		Log->Write("ERROR:\tRoot element does not exist");
		exit(EXIT_FAILURE);
	}

	if( xMainNode->ValueStr () != "JPSgcfm" ) {
		Log->Write("ERROR:\tRoot element value is not 'JPSgcfm'.");
		exit(EXIT_FAILURE);
	}

	//seed
	if(xMainNode->FirstChild("seed")){
		pSeed=atoi(xMainNode->FirstChild("seed")->FirstChild()->Value());
		srand(pSeed);
		Log->Write("INFO: \tseed < %d >",pSeed);
	}

	//geometry
	if(xMainNode->FirstChild("geometry")){
		pGeometryFilename=xMainNode->FirstChild("geometry")->FirstChild()->Value();
		Log->Write("INFO: \tgeometry <"+pGeometryFilename+">");
	}

	//persons and distributions
	if(xMainNode->FirstChild("person")){
		pNumberFilename=xMainNode->FirstChild("person")->FirstChild()->Value();
		Log->Write("INFO: \tperson <"+(pNumberFilename)+">");
	}

	//routing
	if(xMainNode->FirstChild("routing")){
		pRoutingFilename=xMainNode->FirstChild("routing")->FirstChild()->Value();
		Log->Write("INFO: \trouting <"+(pRoutingFilename)+">");
	}

	//traffic
	if(xMainNode->FirstChild("traffic")){
		pTrafficFilename=xMainNode->FirstChild("traffic")->FirstChild()->Value();
		Log->Write("INFO: \ttraffic <"+ (pTrafficFilename)+">");
	}

	//logfile
	if(xMainNode->FirstChild("logfile")){
		pErrorLogFile=xMainNode->FirstChild("logfile")->FirstChild()->Value();
		pLog=2;
		Log->Write("INFO: \tlogfile <"+(pErrorLogFile)+">");
	}

	//trajectories
	TiXmlNode* xTrajectories=xMainNode->FirstChild("trajectories");
	if(xTrajectories){

		xMainNode->FirstChildElement("trajectories")->Attribute("fps",&pfps);

		string format= xMainNode->FirstChildElement("trajectories")->Attribute("format")?
				xMainNode->FirstChildElement("trajectories")->Attribute("format"):"xml-plain";
		_embedMesh = string(xMainNode->FirstChildElement("trajectories")->Attribute("embed_mesh"))=="true"?1:0;

		if(format=="xml-plain") pFormat=FORMAT_XML_PLAIN;
		if(format=="xml-plain" && _embedMesh==1) pFormat=FORMAT_XML_PLAIN_WITH_MESH;
		if(format=="xml-bin") pFormat=FORMAT_XML_BIN;
		if(format=="plain") pFormat=FORMAT_PLAIN;
		if(format=="vtk") pFormat=FORMAT_VTK;


		//a file descriptor was given
		if(xTrajectories->FirstChild("file")){
			const char* tmp = xTrajectories->FirstChildElement("file")->Attribute("location");
			if(tmp) pTrajectoriesFile = tmp;
			Log->Write("INFO: \toutput file  <"+string(pTrajectoriesFile)+">");
			Log->Write("INFO: \tin format <%s> at <%f> frames per seconds",format.c_str(),pfps);
		}

		if(xTrajectories->FirstChild("socket")){
			const char* tmp = xTrajectories->FirstChildElement("socket")->Attribute("hostname");
			if(tmp) pHostname = tmp;
			xTrajectories->FirstChildElement("socket")->Attribute("port",&pPort);
			Log->Write("INFO: \tStreaming results to output [%s:%d] ",pHostname.c_str(),pPort);
		}
	}

	//model parameters, only one node
	TiXmlNode* xPara=xMainNode->FirstChild("parameters");
	if(!xPara){
		Log->Write("INFO: \tno gcfm parameter values found");
		return;
	}

	//tmax
	if(xPara->FirstChild("tmax")){
		const char* tmax=xPara->FirstChildElement("tmax")->FirstChild()->Value();
		const char* unit=xPara->FirstChildElement("tmax")->Attribute("unit");
		pTmax=atof(tmax);
		Log->Write("INFO: \tpTmax <"+string(tmax)+" " +unit +" (unit ignored)>");
	}

	//solver
	if(xPara->FirstChild("solver")){
		string solver=xPara->FirstChild("solver")->FirstChild()->Value();
		if(solver=="euler") pSolver=1;
		else if(solver=="verlet") pSolver=2;
		else if(solver=="leapfrog") pSolver=3;
		else {
			Log->Write("ERROR: \twrong value for solver type!!!\n");
			exit(EXIT_FAILURE);
		}
		Log->Write("INFO: \tpSolver <"+string(solver)+">");
	}

	//stepsize
	if(xPara->FirstChild("stepsize")){
		const char* stepsize=xPara->FirstChild("stepsize")->FirstChild()->Value();
		if(stepsize)
			pdt=atof(stepsize);
		Log->Write("INFO: \tstepsize <%f>",pdt);
	}

	//exit crossing strategy
	if(xPara->FirstChild("exitCrossingStrategy")){
		const char* tmp=xPara->FirstChild("exitCrossingStrategy")->FirstChild()->Value();
		if(tmp) 	pExitStrategy= atoi(tmp);
		Log->Write("INFO: \texitCrossingStrategy < %d >", pExitStrategy);
	}

	//linked-cells
	if(xPara->FirstChild("linkedcells")){
		string linkedcells=xPara->FirstChildElement("linkedcells")->Attribute("enabled");
		string cell_size=xPara->FirstChildElement("linkedcells")->Attribute("cell_size");

		if(linkedcells=="true"){
			pLinkedCells=true;
			pLinkedCellSize=atof(cell_size.c_str());
			Log->Write("INFO: \tlinked cells enabled with size  <"+cell_size+">");
		}else{
			Log->Write("WARNING: \tinvalid parameters for linkedcells");
		}
	}

	//desired speed
	if(xPara->FirstChild("v0")){
		string mu=xPara->FirstChildElement("v0")->Attribute("mu");
		string sigma=xPara->FirstChildElement("v0")->Attribute("sigma");
		pV0Mu=atof(mu.c_str());
		pV0Sigma=atof(sigma.c_str());
		Log->Write("INFO: \tdesired velocity mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//bmax
	if(xPara->FirstChild("bmax")){
		string mu=xPara->FirstChildElement("bmax")->Attribute("mu");
		string sigma=xPara->FirstChildElement("bmax")->Attribute("sigma");
		pBmaxMu=atof(mu.c_str());
		pBmaxSigma=atof(sigma.c_str());
		Log->Write("INFO: \tBmax mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//bmin
	if(xPara->FirstChild("bmin")){
		string mu=xPara->FirstChildElement("bmin")->Attribute("mu");
		string sigma=xPara->FirstChildElement("bmin")->Attribute("sigma");
		pBminMu=atof(mu.c_str());
		pBminSigma=atof(sigma.c_str());
		Log->Write("INFO: \tBmin mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//amin
	if(xPara->FirstChild("amin")){
		string mu=xPara->FirstChildElement("amin")->Attribute("mu");
		string sigma=xPara->FirstChildElement("amin")->Attribute("sigma");
		pAminMu=atof(mu.c_str());
		pAminSigma=atof(sigma.c_str());
		Log->Write("INFO: \tAmin mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//tau
	if(xPara->FirstChild("tau")){
		string mu=xPara->FirstChildElement("tau")->Attribute("mu");
		string sigma=xPara->FirstChildElement("tau")->Attribute("sigma");
		pTauMu=atof(mu.c_str());
		pTauSigma=atof(sigma.c_str());
		Log->Write("INFO: \tTau mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//atau
	if(xPara->FirstChild("atau")){
		string mu=xPara->FirstChildElement("atau")->Attribute("mu");
		string sigma=xPara->FirstChildElement("atau")->Attribute("sigma");
		pAtauMu=atof(mu.c_str());
		pAtauSigma=atof(sigma.c_str());
		Log->Write("INFO: \tAtau mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//force_ped
	if(xPara->FirstChild("force_ped")){
		string nu=xPara->FirstChildElement("force_ped")->Attribute("nu");
		string dist_max=xPara->FirstChildElement("force_ped")->Attribute("dist_max");
		string disteff_max=xPara->FirstChildElement("force_ped")->Attribute("disteff_max");
		string interpolation_width=xPara->FirstChildElement("force_ped")->Attribute("interpolation_width");

		pMaxFPed=atof(dist_max.c_str());
		pNuPed=atof(nu.c_str());
		pDistEffMaxPed=atof(disteff_max.c_str());
		pIntPWidthPed=atof(interpolation_width.c_str());
		Log->Write("INFO: \tfrep_ped mu=" +nu +", dist_max="+dist_max+", disteff_max="
				+ disteff_max+ ", interpolation_width="+interpolation_width);
	}

	//force_wall
	if(xPara->FirstChild("force_wall")){
		string nu=xPara->FirstChildElement("force_wall")->Attribute("nu");
		string dist_max=xPara->FirstChildElement("force_wall")->Attribute("dist_max");
		string disteff_max=xPara->FirstChildElement("force_wall")->Attribute("disteff_max");
		string interpolation_width=xPara->FirstChildElement("force_wall")->Attribute("interpolation_width");
		pMaxFWall=atof(dist_max.c_str());
		pNuWall=atof(nu.c_str());
		pDistEffMaxWall=atof(disteff_max.c_str());
		pIntPWidthWall=atof(interpolation_width.c_str());
		Log->Write("INFO: \tfrep_wall mu=" +nu +", dist_max="+dist_max+", disteff_max="
				+ disteff_max+ ", interpolation_width="+interpolation_width);
	}

	// pre parse the person file to extract some information we need
	//route choice strategy
	TiXmlDocument docPersons(pNumberFilename);
	if (!docPersons.LoadFile()){
		Log->Write("ERROR: \t%s", docPersons.ErrorDesc());
		Log->Write("ERROR: \t could not parse the person file");
		exit(EXIT_FAILURE);
	}


	TiXmlElement* xPersonsNode = docPersons.RootElement();
	if( ! xPersonsNode ) {
		Log->Write("ERROR:\tRoot element does not exist");
		exit(EXIT_FAILURE);
	}

	if( xPersonsNode->ValueStr () != "persons" ) {
		Log->Write("ERROR:\tRoot element value is not 'persons'.");
		exit(EXIT_FAILURE);
	}

	TiXmlNode* xRouters=xPersonsNode->FirstChild("routers");

	if(!xRouters) {
		Log->Write("ERROR:\tNo routers found.");
				exit(EXIT_FAILURE);
	}

	for(TiXmlElement* e = xRouters->FirstChildElement("router"); e;
			e = e->NextSiblingElement("router")) {

		string strategy=e->Attribute("method");
		int id=atoi(e->Attribute("id"));

		if(strategy=="local_shortest")
			pRoutingStrategies.push_back(make_pair(id,ROUTING_LOCAL_SHORTEST));
		else if(strategy=="global_shortest")
			pRoutingStrategies.push_back(make_pair(id,ROUTING_GLOBAL_SHORTEST));
		else if(strategy=="quickest")
			pRoutingStrategies.push_back(make_pair(id,ROUTING_QUICKEST));
		else if(strategy=="dynamic")
			pRoutingStrategies.push_back(make_pair(id,ROUTING_DYNAMIC));
		else if(strategy=="nav_mesh")
			pRoutingStrategies.push_back(make_pair(id,ROUTING_NAV_MESH));
		else if(strategy=="dummy")
			pRoutingStrategies.push_back(make_pair(id,ROUTING_DUMMY));
		else{
			Log->Write("ERROR: \twrong value for routing strategy!!!\n");
			cout<<strategy<<endl;
			exit(EXIT_FAILURE);
		}
	}

	Log->Write("INFO: \tdone parsing ini");
}


const string& ArgumentParser::GetPersonsFilename() const {
	return pNumberFilename;
}

const FileFormat& ArgumentParser::GetFileFormat() const {
	return pFormat;
}
const string& ArgumentParser::GetHostname() const {
	return pHostname;
}
void ArgumentParser::SetHostname(const string& hostname) {
	pHostname = hostname;
}
int ArgumentParser::GetPort() const {
	return pPort;
}
void ArgumentParser::SetPort(int port) {
	pPort = port;
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

const string& ArgumentParser::GetGeometryFilename() const {
	return pGeometryFilename;
}

const string& ArgumentParser::GetNavigationMesh() const {
	return pNavMeshFilename;
}

int ArgumentParser::GetExitStrategy() const {
	return pExitStrategy;
}


bool ArgumentParser::GetLinkedCells() const {
	return pLinkedCells;
}

vector< pair<int, RoutingStrategy> > ArgumentParser::GetRoutingStrategy() const {
	return pRoutingStrategies;
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

double ArgumentParser::GetLinkedCellSize() const{
	return pLinkedCellSize;
}
unsigned int ArgumentParser::GetSeed() const {
	return pSeed;
}


const string& ArgumentParser::GetPathwayFile() const {
	return pPathwayFilename;
}

int ArgumentParser::GetEmbededMesh() const {
	return _embedMesh;
}


const string& ArgumentParser::GetErrorLogFile() const {
	return pErrorLogFile;
}

const string& ArgumentParser::GetTrafficFile() const {
	return pTrafficFilename;
}

const string& ArgumentParser::GetRoutingFile() const{
	return pRoutingFilename;
}

int ArgumentParser::GetMaxOpenMPThreads() const{
	return pMaxOpenMPThreads;
}
const string& ArgumentParser::GetTrajectoriesFile() const {
	return pTrajectoriesFile;
}

void ArgumentParser::SetTrajectoriesFile(const string& trajectoriesFile) {
	pTrajectoriesFile = trajectoriesFile;
}
