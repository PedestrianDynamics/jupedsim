#include <getopt.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include<math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "../general/xmlParser.h"
#include "../IO/OutputHandler.h"
#include "ArgumentParser.h"

using namespace std;

void ArgumentParser::Usage() {

	cout<<" JPSreport.exe --inifile=input.xml"<<endl;
/*
	fprintf(stderr,
			"Usage: program options\n\n"
			"With the following options (default values in parenthesis):\n\n"
			"	[-t/--trajectory <string>]						name of input trajectory file\n"
			"  	[-I/--input path <filepath>]    				path of the input file(./Inputfiles/)\n"
			"  	[-O/--output path <filepath>]   				path of the input file(./Outputfiles/)\n"
			"	[-g/--geometry <string>]        				path to the geometry file (./Inputfiles/geo.xml)\n"
			"	[-m/--measurment area <int>]    			type of measurement area(1)\n"
			"                                   							1: Bounding box\n"
			"                                       						2: Line\n"
			"	[-b/--bounding box  <double>]				p1.x p1.y p2.x p2.y p3.x p3.y p4.x p4.y (in clockwise)\n"
			"	[-d/--moving direction <double>]			p1.x p1.y p2.x p2.y \n"
			"	[-l/--line <double>]								p1.x p1.y p2.x p2.y \n"



	fprintf(stderr, "-c --> set cutbycircle=true (false)\n");
	fprintf(stderr, "-a --> set fieldAnalysis=true (false)\n");
	fprintf(stderr, "-g --> set IsOutputGraph=true (false)\n");
	fprintf(stderr, "-v --> set calcIndividualfunddata=true (false)\n");
	fprintf(stderr, "-s scale (3000)\n");
	fprintf(stderr, "-l --> set IsClassicMethod=true (false)\n");
	fprintf(stderr, "-F --> set IsFundamentalTinTout=true (false). density is classical. So IsClassicMethod should be true\n");
	fprintf(stderr, "-V --> set IsFlowVelocity=true (false)\n");
	fprintf(stderr, "-L x1 y1 x2 y2 (0.0, 300.0, 250.0, 300.0)\n");
	fprintf(stderr, "-y  beginstationary (700)\n");
	fprintf(stderr, "-Y  endstationary (1800)\n");
	fprintf(stderr, "-R Row (65)\n");
	fprintf(stderr, "-C Column (80)\n");
	fprintf(stderr, "-m  Meas. Area ax1 (-300)\n");
	fprintf(stderr, "-n  Meas. Area ay1 (100)\n");
	fprintf(stderr, "-M  Meas. Area ax2 (300)\n");
	fprintf(stderr, "-N  Meas. Area ay2 (200)\n");
	fprintf(stderr, "-o  Outputfile (result.dat)\n");
	fprintf(stderr, "-O  goes in the name of the polygons, speed and point files (dummy)\n");
	fprintf(stderr, "-d --> set use_Vxy false (true)\n");
	fprintf(stderr, "-e --> set use_Vy true (false)\n");
	fprintf(stderr, "-k --> set use_Vx true (false)\n");
	fprintf(stderr, "-p fps (10)\n");
	fprintf(stderr, "-G cor_x cor_y length width (corridor)\n");


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
			"  [-h/--help]                      this manual output\n"
			"\n");


*/
	exit(EXIT_SUCCESS);
}

ArgumentParser::ArgumentParser() {
	// Default parameter values
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
	pSeed=0;
	pFormat=FORMAT_XML_PLAIN;
}



void ArgumentParser::ParseArgs(int argc, char **argv) {
	int c;
	int option_index = 0;

	static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			{"inifile", optional_argument, 0, 'q'},
			{0, 0, 0, 0}
	};

	while ((c = getopt_long_only(argc, argv,
			"q:h",
			long_options, &option_index)) != -1) {

		switch (c) {

			case 'q':
			{
				string inifile="input.xml";
				if (optarg)
					inifile=optarg;
				Log->Write("INFO: \t Loading initialization file <"+inifile+">");
				ParseIniFile(inifile);
			}
			break;

			case 'h':
				Usage();
				break;
			default:
			{
				Log->Write("ERROR: \tin ArgumentParser::ParseArgs() "
						"wrong program options!!!\n");
				Usage();
			}
		}
	}
}


void ArgumentParser::ParseIniFile(string inifile){

	XMLNode xMainNode=XMLNode::openFileHelper(inifile.c_str(),"JPSreport");

	Log->Write("INFO: \tParsing the ini file");
	//I just assume all parameters are present

	//geometry
	XMLNode xGeometry = xMainNode.getChildNode("Geometry");
	if(!xGeometry.isEmpty()){
		string geometry = xmltoa(xGeometry.getAttribute("file"));
		Log->Write("INFO: \tgeometry  <"+geometry+">");
	}

	//trajectories
	XMLNode xTrajectories=xMainNode.getChildNode("Trajectories");
	if(!xTrajectories.isEmpty()){

/*		pfps = xmltof(xTrajectories.getAttribute("fps"),pfps);
		string format=xmltoa(xTrajectories.getAttribute("format"),"xml-plain");
		if(format=="xml-plain") pFormat=FORMAT_XML_PLAIN;
		if(format=="xml-bin") pFormat=FORMAT_XML_BIN;
		if(format=="plain") pFormat=FORMAT_PLAIN;
		if(format=="vtk") pFormat=FORMAT_VTK;*/

		//a file descriptor was given
		string trajectory_path;
		if(!xTrajectories.getChildNode("directory").isEmpty()){
			trajectory_path =
					xmltoa(
								xTrajectories.getChildNode(
										"directory").getAttribute("location"));
			pTrajectoriesFile =
					trajectory_path + xmltoa(
							xTrajectories.getChildNode(
									"file").getAttribute("name"),
									pTrajectoriesFile.c_str())+".xml";

		Log->Write("INFO: \ttrajectory file  <"+string(pTrajectoriesFile)+">");
		}


	}

	//measurement area
	XMLNode xMeasurementArea=xMainNode.getChildNode("MeasurementAreas");
	if(!xMeasurementArea.isEmpty()){
		if(!xMeasurementArea.getChildNode("Area_B").isEmpty())
		{
			string pMeasureAreaId =
							xmltoa(
									xMeasurementArea.getChildNode(
											"Area_B").getAttribute("id"));
			string pMeasureAreaType =
							xmltoa(
									xMeasurementArea.getChildNode(
											"Area_B").getAttribute("type"));
			XMLNode xBox = xMeasurementArea.getChildNode("Area_B");
			string box_p1x = xmltoa(xBox.getChildNode("p1").getAttribute("x"));
			string box_p1y = xmltoa(xBox.getChildNode("p1").getAttribute("y"));
			string box_p2x = xmltoa(xBox.getChildNode("p2").getAttribute("x"));
			string box_p2y = xmltoa(xBox.getChildNode("p2").getAttribute("y"));
			string box_p3x = xmltoa(xBox.getChildNode("p3").getAttribute("x"));
			string box_p3y = xmltoa(xBox.getChildNode("p3").getAttribute("y"));
			string box_p4x = xmltoa(xBox.getChildNode("p4").getAttribute("x"));
			string box_p4y = xmltoa(xBox.getChildNode("p5").getAttribute("y"));
			string MovingDire_start = xmltoa(xBox.getChildNode("MovingDirection").getAttribute("start"));
			string MovingDire_end = xmltoa(xBox.getChildNode("MovingDirection").getAttribute("end"));
			double start_x = atof(xmltoa(xBox.getChildNode(MovingDire_start.c_str()).getAttribute("x")));
			double start_y = atof(xmltoa(xBox.getChildNode(MovingDire_start.c_str()).getAttribute("y")));
			double end_x = atof(xmltoa(xBox.getChildNode(MovingDire_end.c_str()).getAttribute("x")));
			double end_y = atof(xmltoa(xBox.getChildNode(MovingDire_end.c_str()).getAttribute("y")));
			double LengthMeasurementArea = sqrt(pow((start_x-end_x),2)+pow((start_y-end_y),2));

			Log->Write("INFO: \tmeasure area id  <"+pMeasureAreaId+">");
			Log->Write("INFO: \tmeasure area type  <"+pMeasureAreaType+">");
			Log->Write("INFO: \tp1 of Box  <"+box_p1x+','+box_p1y+">");
			cout<<"INFO: \tlength of measurement area is:  <" << LengthMeasurementArea<<">"<<endl;
		}
		if(!xMeasurementArea.getChildNode("Area_L").isEmpty())
		{
			string pMeasureAreaId = xmltoa(xMeasurementArea.getChildNode("Area_L").getAttribute("id"));
			string pMeasureAreaType = xmltoa(xMeasurementArea.getChildNode("Area_L").getAttribute("type"));
			XMLNode xLine = xMeasurementArea.getChildNode("Area_L");
			string line_startx = xmltoa(xLine.getChildNode("Start").getAttribute("x"));
			string line_starty = xmltoa(xLine.getChildNode("Start").getAttribute("y"));
			string line_endx = xmltoa(xLine.getChildNode("End").getAttribute("x"));
			string line_endy = xmltoa(xLine.getChildNode("End").getAttribute("y"));
			Log->Write("INFO: \tmeasure area id  <"+pMeasureAreaId+">" + "\t<"+pMeasureAreaType+">");
			Log->Write("INFO: \treference line starts from  <"+line_startx+","+line_starty+">" +" to  <"+line_endx+","+line_endy+">" );
		}
	}

	//instantaneous velocity
	XMLNode xVelocity=xMainNode.getChildNode("Velocity");
	if(!xVelocity.isEmpty()){
		string UseXComponent = string(xVelocity.getChildNode("UseXComponent").getText());
		string UseYComponent = string(xVelocity.getChildNode("UseYComponent").getText());
		string HalfFrameNumberToUse = string(xVelocity.getChildNode("HalfFrameNumberToUse").getText());
		if(UseXComponent == "true"&&UseYComponent == "false"){
			Log->Write("INFO: \tonly x-component coordinates will be used in velocity calculation within  2* <"+HalfFrameNumberToUse+" frames>" );
		}
		else if(UseXComponent == "false"&&UseYComponent == "true"){
			Log->Write("INFO: \tonly y-component coordinates will be used in velocity calculation within  2* <"+HalfFrameNumberToUse+" frames>" );
		}
		else if(UseXComponent == "true"&&UseYComponent == "true"){
			Log->Write("INFO: \tx and y-component of coordinates will be used in velocity calculation within  2* <"+HalfFrameNumberToUse+" frames>" );
		}
		else{
			Log->Write("INFO: \ttype of velocity is not selected, please check it !!! " );
			return;
		}
	}

	// method A
	XMLNode xMethod_A=xMainNode.getChildNode("Method_A");
	if(string(xMethod_A.getAttribute("enabled"))=="true"){
		string TimeInterval_A = string(xMethod_A.getChildNode("TimeInterval").getText());
		string MeasureAreaId = string(xMethod_A.getChildNode("MeasurementArea").getAttribute("id"));
		Log->Write("INFO: \tMethod A is selected" );
		Log->Write("INFO: \ttime interval used in Method A is <"+TimeInterval_A+" frames>" );
	}

	// method B
	XMLNode xMethod_B=xMainNode.getChildNode("Method_B");
	if(string(xMethod_B.getAttribute("enabled"))=="true"){
		string MeasureAreaId = string(xMethod_B.getChildNode("MeasurementArea").getAttribute("id"));
		Log->Write("INFO: \tMethod B is selected" );
	}

	// method C
	XMLNode xMethod_C=xMainNode.getChildNode("Method_C");
	if(string(xMethod_C.getAttribute("enabled"))=="true"){
		string MeasureAreaId = string(xMethod_C.getChildNode("MeasurementArea").getAttribute("id"));
		Log->Write("INFO: \tMethod C is selected" );
	}

/*
	<Method_D enabled="true" cutbycircle="true" IsOutputGraph="false"
		IndividualFDdata="true">
		<MeasurementArea id="1" />
		<SteadyState start="" end="" /> <!-- //the begin of stationary state //the end of stationary state -->
		<fieldAnalysis enabled="true" NRow="80" NColumn="65"
			scale_x="10" scale_y="10" low_ed_x="-100" low_ed_x="-100" />*/

	// method D
	XMLNode xMethod_D=xMainNode.getChildNode("Method_D");
	if(string(xMethod_D.getAttribute("enabled"))=="true"){
		string xIsCutByCircle = string(xMethod_D.getAttribute("cutbycircle"));
		string xIsOutputGraph = string(xMethod_D.getAttribute("IsOutputGraph"));
		string xIsIndividualFD = string(xMethod_D.getAttribute("IndividualFDdata"));
		string MeasureAreaId = string(xMethod_D.getChildNode("MeasurementArea").getAttribute("id"));

		if(!xMethod_D.getChildNode("SteadyState").isEmpty()){
			const char* steady_start = xMethod_D.getChildNode("SteadyState").getAttribute("start");
			const char* steady_end = xMethod_D.getChildNode("SteadyState").getAttribute("end");
			double pSteady_start = atof(steady_start);
			double pSteady_end = atof(steady_end);
			Log->Write("INFO: \tthe steady state is from  <" + string(steady_start) + "> to <"+string(steady_end) +"> frame"  );
		}
		if(string(xMethod_D.getChildNode("fieldAnalysis").getAttribute("enabled")) == "true"){
			const char* Nrow = xMethod_D.getChildNode("fieldAnalysis").getAttribute("NRow");
			const char* Ncolumn = xMethod_D.getChildNode("fieldAnalysis").getAttribute("NColumn");
			const char* scale_x = xMethod_D.getChildNode("fieldAnalysis").getAttribute("scale_x");
			const char* scale_y = xMethod_D.getChildNode("fieldAnalysis").getAttribute("scale_y");
			const char* low_ed_x = xMethod_D.getChildNode("fieldAnalysis").getAttribute("low_ed_x");
			const char* low_ed_y = xMethod_D.getChildNode("fieldAnalysis").getAttribute("low_ed_y");
			Log->Write("INFO: \tprofiles will be calculated" );
			Log->Write("INFO: \tthe scale of the discretized cell in x, y direction are: <" + string(scale_x) + "> and <"+string(scale_y) +">"  );
		}
		Log->Write("INFO: \tMethod D is selected" );
	}
/*
	if(!xMainNode.getChildNode("Geometry").isEmpty()){
		pGeometryFilename=xMainNode.getChildNode("geometry").getText();
		Log->Write("INFO: \tgeometry <"+string(pGeometryFilename)+">");
	}


	//logfile
	if(!xMainNode.getChildNode("logfile").isEmpty()){
		pErrorLogFile=xMainNode.getChildNode("logfile").getText();
		pLog=2;
		Log->Write("INFO: \tlogfile <"+string(pErrorLogFile)+">");
	}


	//model parameters, only one node
	XMLNode xPara=xMainNode.getChildNode("parameters");
	if(xPara.isEmpty()){
		Log->Write("INFO: \tno gcfm parameter values found");
		return;
	}

	//tmax
	if(!xPara.getChildNode("tmax").isEmpty()){
		const char* tmax=xPara.getChildNode("tmax").getText();
		const char* unit=xPara.getChildNode("tmax").getAttribute("unit");
		pTmax=atof(tmax);
		Log->Write("INFO: \tpTmax <"+string(tmax)+" " +unit +" (unit ignored)>");
	}


	//stepsize
	if(!xPara.getChildNode("stepsize").isEmpty()){
		const char* stepsize=xPara.getChildNode("stepsize").getText();
		pdt=atof(stepsize);
		Log->Write("INFO: \tstepsize <"+string(stepsize)+">");
	}

	//exit crossing strategy
	if(!xPara.getChildNode("exitCrossingStrategy").isEmpty()){
		string exitStrategy=xPara.getChildNode("exitCrossingStrategy").getText();
		pExitStrategy=xmltof(exitStrategy.c_str(),pExitStrategy);
		Log->Write("INFO: \exitCrossingStrategy <"+string(exitStrategy)+">");
	}

	//linked-cells
	if(!xPara.getChildNode("linkedcells").isEmpty()){
		string linkedcells=string(xPara.getChildNode("linkedcells").getAttribute("enabled"));
		string cell_size=string(xPara.getChildNode("linkedcells").getAttribute("cell_size"));
		if(linkedcells=="true"){
			pLinkedCells=true;
			pLinkedCellSize=xmltof(cell_size.c_str(),pLinkedCellSize);
			Log->Write("INFO: \tlinked cells enable with size  <"+cell_size+">");
		}else{
			Log->Write("WARNING: \tinvalid parameters for linkedcells");
		}
	}

	//desired speed
	if(!xPara.getChildNode("v0").isEmpty()){
		string mu=string(xPara.getChildNode("v0").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("v0").getAttribute("sigma"));
		pV0Mu=atof(mu.c_str());
		pV0Sigma=atof(sigma.c_str());
		Log->Write("INFO: \tdesired velocity mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//bmax
	if(!xPara.getChildNode("bmax").isEmpty()){
		string mu=string(xPara.getChildNode("bmax").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("bmax").getAttribute("sigma"));
		pBmaxMu=atof(mu.c_str());
		pBmaxSigma=atof(sigma.c_str());
		Log->Write("INFO: \tBmax mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//bmin
	if(!xPara.getChildNode("bmin").isEmpty()){
		string mu=string(xPara.getChildNode("bmin").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("bmin").getAttribute("sigma"));
		pBminMu=atof(mu.c_str());
		pBminSigma=atof(sigma.c_str());
		Log->Write("INFO: \tBmin mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//amin
	if(!xPara.getChildNode("amin").isEmpty()){
		string mu=string(xPara.getChildNode("amin").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("amin").getAttribute("sigma"));
		pAminMu=atof(mu.c_str());
		pAminSigma=atof(sigma.c_str());
		Log->Write("INFO: \tAmin mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//tau
	if(!xPara.getChildNode("tau").isEmpty()){
		string mu=string(xPara.getChildNode("tau").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("tau").getAttribute("sigma"));
		pTauMu=atof(mu.c_str());
		pTauSigma=atof(sigma.c_str());
		Log->Write("INFO: \tTau mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}
	//atau
	if(!xPara.getChildNode("atau").isEmpty()){
		string mu=string(xPara.getChildNode("atau").getAttribute("mu"));
		string sigma=string(xPara.getChildNode("atau").getAttribute("sigma"));
		pAtauMu=atof(mu.c_str());
		pAtauSigma=atof(sigma.c_str());
		Log->Write("INFO: \tAtau mu=" +mu +" ,"+ " sigma="+sigma+" ");
	}

	//pExitStrategy
	if(!xPara.getChildNode("exitStrategy").isEmpty()){
		const char* e=xPara.getChildNode("exitStrategy").getText();
		int ie = atoi(e);
		if (ie == 1 || ie == 2 || ie == 3 || ie == 4)
		{
			pExitStrategy = ie;
			Log->Write("INFO: \texitStrategy <"+ string(e) +">");
		}
		else
		{
			pExitStrategy = 2;
			Log->Write("WARNING: \twrong value for exitStrategy. Use strategy 2\n");
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
		Log->Write("INFO: \tfrep_ped mu=" +nu +", dist_max="+dist_max+", disteff_max="
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
		Log->Write("INFO: \tfrep_wall mu=" +nu +", dist_max="+dist_max+", disteff_max="
				+ disteff_max+ ", interpolation_width="+interpolation_width);
	}

*/
	Log->Write("INFO: \tdone parsing ini");
}


const FileFormat& ArgumentParser::GetFileFormat() const {
	return pFormat;
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


const string& ArgumentParser::GetErrorLogFile() const {
	return pErrorLogFile;
}

const string& ArgumentParser::GetTrajectoriesFile() const {
	return pTrajectoriesFile;
}

void ArgumentParser::SetTrajectoriesFile(const string& trajectoriesFile) {
	pTrajectoriesFile = trajectoriesFile;
}
