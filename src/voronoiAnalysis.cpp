//============================================================================
// Name        : voronoiAnalysis.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "VoronoiDiagramGenerator.h"
#include "VoronoiPolygons.h"
#include "Geometry.h"
#include "Analysis.h"
#include "voronoiAnalysis.h"

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <time.h>
#include "xmlParser.h"

using namespace std;

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
using boost::function;
using namespace boost::geometry;

typedef model::d2::point_xy<double, boost::geometry::cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;

/**
 * Tell the user how to use the program
 */
void usage(char *name) {
  fprintf(stderr, "Usage: program options\n\nwith the following optional options (default values in parathesis):\n\n");
  fprintf(stderr, "-f inputfile\n");
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
  
}
int main(int argc, char *argv[])
{

//============================= Measurement Methods and expected results choice ==============================================

	bool IsFlowVelocity = true; 						// Method A (Zhang2011a)
	bool IsFundamentalTinTout = true; 			// Method B (Zhang2011a)
	bool IsClassicMethod = true; 					// Method C //calculate and save results of classic in separate file

	//--------------------------------------   Parameters related to the Voronoi method ------------------------------------

	bool IsVoronoiMethod =true;  					// Method D--Voronoi method
	cutbycircle = false;  									// Adjust whether cut each original Voronoi cell by a circle (for bottleneck exp is necessary)
	IsOutputGraph = true;   							// Whether output the data to plot the Voronoi diagram each frame
	fieldAnalysis = false;   								// Whether make field analysis or not
	calcIndividualfunddata = true; 					// Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
	if(fieldAnalysis || calcIndividualfunddata || IsOutputGraph)
	{
		IsVoronoiMethod = true;
	}

	if(IsFundamentalTinTout)
	{
		IsClassicMethod = true;
		LengthMeasurementarea = 240;  // This is only used for method B
	}

	use_Vx=false;  											// only the x coordinate is used when calculating the velocity
	use_Vy=false; 											// only the y coordinate is used when calculating the velocity
	use_Vxy=true; 											// both x and y coordinate are used when calculating the velocity
	int deltaF = 5 ;											// half of the time interval that used to calculate instantaneous velocity of ped i.
																	// here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.

//============================== Parameters initialization =============================================


	if (IsFlowVelocity)
	{
		DeltaT=70; 											// Time interval that used to count the flow [fr]
		Line_startX = -240.0;  								// The coordinate of the line used to calculate the flow and velocity
		Line_startY = 100.0;
		Line_endX = 0.0;
		Line_endY = 100.0;
	}

	//PLOT VORONOI GRAPHS
   /*In this code, the Voronoi diagram has to be generated with the help of an accessory rectangle!!!!!!
    * here we use the minimal and maximal coordinates of the geometry (xmin, xmax, ymin, ymax) to
    * define the rectangle-------*/
	float xmin = -500;
	float xmax = 400;
	float ymin = -240;
	float ymax = 400;

	/*to get the individual Fundamental diagram based on Voronoi diagram (personal space and velocity),
	 * it better to give the start and end time of the steady state. */

	int beginstationary=700;  //the begin of stationary state
	int endstationary=1800;  //the end of stationary state

	/*to make field analysis, the geometery has to be discretized into small cells. Here NRow is the number
	 * of rows for the discretization, NColumn is the number of column. scale_x and scale_y represent the
	 * size of the cells. low_ed_x and low_ed_y are the coordinates of the lowest vertex of the geometry.
	 * these prameters are necessary for the discretization.*/

	if(fieldAnalysis)
	{
		NRow=80;    // the number of rows that the geometry will be discretized for field analysis
		NColumn=65;	//the number of columns that the geometry will be discretized for field analysis
		scale_x=10;   // the size of the grid
		scale_y=10;
		low_ed_x = -100;//0; //todo LOWest vertex of the geometry
		low_ed_y = -200; // 0 todo
	}
	//-------------------------------------- Start to Analyze and calculate --------------------------------------------------------------
	  Analysis analysor;
	  Geometry geometry;
	  //TODO: select geometry with input parameters
	  polygon_2d geom=geometry.Tshape();
	  float cor_x=-500, cor_y=-240, cor_l=900, cor_w=640;  //---coordination of the left bottom point of the corridor as well as its length and width
	  //geom = geometry.Channel(cor_x, cor_y, cor_l, cor_w);
	  float ax1=-240, ay1=100, ax2=240, ay2=240;  //---coordination of the left bottom point of the measurement area as well as its length and width

	  polygon_2d measurezone=geometry.MeasureArea(ax1,ay1,ax2,ay2);//   (-300,100,300,200);  //define the measurement area

	  string datafile=  "ko-240-050-240";  //trajectory file name
	  string inputfile=datafile+".xml";

	  #ifdef WIN32
	  	  string outputPath=  "Fundamental_Diagram\\Classical_Voronoi\\";
	  #else
	  	  string outputPath=  "Fundamental_Diagram/Classical_Voronoi/";
	  #endif

	  string results_C = outputPath+"rho_v_Classic_"+datafile+".dat";
	  string results_V = outputPath+"rho_v_Voronoi_"+datafile+".dat";
	  if(IsClassicMethod)
	  {

		  if((fClassic_rho_v=fopen(results_C.c_str(),"w"))==NULL)
			{
			  printf("cannot open the file to write classical density and velocity\n");
			  exit(0);
			}
		  fprintf(fClassic_rho_v,"#Frame \tclassical density(m^(-2))\t	classical velocity(m/s)\n");
	  }
	  if(IsVoronoiMethod)
	  {

		  if((fVoronoi_rho_v=fopen(results_V.c_str(),"w"))==NULL)
			{
			  printf("cannot open the file to write Voronoi density and velocity\n");
			  exit(0);
			}
		  fprintf(fVoronoi_rho_v,"#Frame \t Voronoi density(m^(-2))\t	Voronoi velocity(m/s)\n");
	  }
  /*========================== Read in command line parameters =========================***/
  /*=========================== parse command line ===================================*/
	int par = 1;
	while (par < argc)
	{
		if (argv[par][0] == '-')
		{
			//printf("%d=%c%c\n",i,argv[i][0],argv[i][1]);
			switch (argv[par][1])
			{
				case 'f':					// if a file name is passed
					if (par + 1 < argc && argv[par + 1][0] != '-') 			// all options begin with '-'
					{
						char filename[200];
						sprintf(filename, "%s", argv[par + 1]);
						inputfile = filename;
						par++;
					}
					break;
				case 'c':
					cutbycircle = true;
					break;
				case 'a':
					fieldAnalysis = true;
					break;
				case 'g':
					IsOutputGraph = true;
					break;
				case 'l':
					IsClassicMethod = true;
					break;
				case 'F':
					IsFundamentalTinTout = true;
					break;
				case 'V':
					IsFlowVelocity = true;
					break;
				case 'L':
					if (par + 1 < argc && argv[par + 1][0] != '-')
						Line_startX = atof(argv[++par]);
					if (par + 1 < argc && argv[par + 1][0] != '-')
						Line_startY = atof(argv[++par]);
					if (par + 1 < argc && argv[par + 1][0] != '-')
						Line_endX = atof(argv[++par]);
					if (par + 1 < argc && argv[par + 1][0] != '-')
						Line_endY = atof(argv[++par]);
					break;
				case 'G':
					if (par + 1 < argc && argv[par + 1][0] != '-')
						cor_x = atof(argv[++par]);
					if (par + 1 < argc && argv[par + 1][0] != '-')
						cor_y = atof(argv[++par]);
					if (par + 1 < argc && argv[par + 1][0] != '-')
						cor_l = atof(argv[++par]);
					if (par + 1 < argc && argv[par + 1][0] != '-')
						cor_w = atof(argv[++par]);
					break;
				case 'v':
					calcIndividualfunddata = true;
					break;
				case 'y':
					beginstationary = atoi(argv[++par]);
					break;
				case 'Y':
					endstationary = atoi(argv[++par]);
					break;
				case 'R':
					NRow = atoi(argv[++par]);
					break;
				case 'C':
					NColumn = atoi(argv[++par]);
					break;
				case 'm':
					ax1 = atoi(argv[++par]);
					break;
				case 'n':
					ay1 = atoi(argv[++par]);
					break;
				case 'M':
					ax2 = atoi(argv[++par]);
					break;
				case 'N':
					ay2 = atoi(argv[++par]);
					break;
				case 'o':
					if (par + 1 < argc && argv[par + 1][0] != '-') 			// all options begin with '-'
					{
						char filename[200];
						sprintf(filename, "%s", argv[par + 1]);
						results_C = filename;
						par++;
					}
					break;
				case 'O':
					if (par + 1 < argc && argv[par + 1][0] != '-') 			// all options begin with '-'
					{
						char filename[200];
						sprintf(filename, "%s", argv[par + 1]);
						datafile = filename;
						par++;
					}
					break;
				case 'h':
					usage(argv[0]);
					break;
				case 'd':
					use_Vxy = false;
					break;
				case 'e':
					if(use_Vxy == false && use_Vx ==false)
						use_Vy = true;
					break;
				case 'k':
					if(use_Vxy == false && use_Vy ==false)
						use_Vx = true;
					break;
				case 'p':
					fps = atoi(argv[++par]);
					break;
				default:
					usage(argv[0]);
					break;
			}
		}
		else
		{
			usage(argv[0]);
		}
		par++;
  }


  cout << "inputfile " << inputfile << endl;
  cout << "cutbycircle " << cutbycircle << endl;
  cout << "fieldAnalysis " << fieldAnalysis << endl;
  cout << "IsOutputGraph " << IsOutputGraph << endl;
  cout << "IsVoronoiMethod " << IsVoronoiMethod << endl;
  cout << "IsClassicMethod " << IsClassicMethod << endl;

  cout << "IsFundamentalTinTout " << IsFundamentalTinTout << endl;
  cout << "calcIndividualfunddata " << calcIndividualfunddata  << endl;
  cout << "IsFlowVelocity " <<IsFlowVelocity  << endl;
  
  cout << "L: x1= " << Line_startX << ", y1= " <<  Line_startY<< "; x2= " << Line_endX << ", y2= " <<  Line_endY<< endl; 
  cout << "beginstationary " << beginstationary << endl;
  cout << "endstationary " <<endstationary  << endl;
  cout << "Row= " <<NRow  << endl;
  cout << "Column= " <<NColumn  << endl;
  cout << "Meas. Area ax1= " <<ax1  << endl;
  cout << "Meas. Area ay1= " <<ay1  << endl;

  cout << "Meas. Area ax2= " <<ax2  << endl;
  cout << "Meas. Area ay2= " <<ay2  << endl;
  cout << "use_Vxy= " <<use_Vxy  << endl;
  cout << "use_Vy= " <<use_Vy  << endl;
  cout << "use_Vx= " <<use_Vx  << endl;
  cout << "fps= " <<fps  << endl;
  cout << "cor_x = "<< cor_x << " cor_y = "<<cor_y<< " cor_l = "<<cor_l<<" cor_w="<<cor_w<<endl;
  cout << "Datafile " << datafile << endl;
  cout << "Outputfile " << results_C << endl;
  cout << "========================================="<<endl;
  //getc(stdin);



//---------read the trajectory data from file-----------
////////////////////////////--------------Parse the xmlFile---------------///////////////////////////////////////////////////////

  printf(">>> input file <%s>\n",inputfile.c_str());

  XMLNode xMainNode = XMLNode::openFileHelper(inputfile.c_str(),"trajectoriesDataset");
  XMLNode xHeader = xMainNode.getChildNode("header"); // header
  XMLNode xFrame = xMainNode.getChildNode("frame"); // frame
  int numFrames = xMainNode.nChildNode("frame"); // how much frames


  printf("numFrames = %d\n",numFrames);
  if(!xHeader.getChildNode("agents").isEmpty()){
	  const char* N = xHeader.getChildNode("agents").getText();
	  MaxNumofPed = atoi(N);
	  printf("N=%d\n", MaxNumofPed);
  }
  if(!xHeader.getChildNode("agents").isEmpty()){
	  const char* N = xHeader.getChildNode("frameRate").getText();
	  fps = atoi(N);
	  printf("fps=%d\n", fps);
  }
  float Xcor[MaxNumofPed+1][numFrames];
  float Ycor[MaxNumofPed+1][numFrames];
  int 	  firstFrame[MaxNumofPed+1];  // Record the first frame of each pedestrian
  int 	  lastFrame[MaxNumofPed+1];	// Record the last frame of each pedestrian
  int	  Tin[MaxNumofPed+1];				// Record the time of each pedestrian entering measurement area
  int	  Tout[MaxNumofPed+1];				// Record the time of each pedestrian exiting measurement area
  bool  IsinMeasurezone[MaxNumofPed+1];  // Record whether pedestrian i is in measurement area or not
  for(int i = 0; i <= MaxNumofPed; i++)
  {
	  for (int j = 0; j < numFrames; j++)
	  {
		  Xcor[i][j] = 0;
		  Ycor[i][j] = 0;
	  }
	  firstFrame[i] = 10000;
	  lastFrame[i] = 0;
	  Tin[i] = 0;
	  Tout[i] = 0;
	  IsinMeasurezone[i] = false;
  }

  DensityPerFrame=new float[numFrames+1];
  for(int i=0;i<=numFrames;i++)
	{
	  DensityPerFrame[i]=0;
	}

  if(!xFrame.isEmpty())
  {
	  for (int f=0; f<numFrames; f++) //read the data frame by frame
	  {
		  xFrame = xMainNode.getChildNode("frame", f); // frame j
		  //const char* frid = xFrame.getAttribute("ID");
		 // int frId;
		 //frId = atoi(frid);
		 // printf("j = %d\n",f);
		  //printf("frame ID = %d\n",frId);
		  int numPedsInFrame = xFrame.nChildNode("agent"); // how much agents in this frame
		  //printf("This frame has %d peds\n",numPedsInFrame);
		  for (int i=0; i<numPedsInFrame; i++) //read pedestrians in frame j
		  {
			  if(!xFrame.getChildNode("agent", i).isEmpty())
			  {
                  //get agent id, x, y
				  const char* x = xFrame.getChildNode("agent", i).getAttribute("xPos");
				  const char* y = xFrame.getChildNode("agent", i).getAttribute("yPos");
				  const char* id = xFrame.getChildNode("agent", i).getAttribute("ID");
				  int ID = atoi(id);
				  Xcor[ID][f] = atof(x);
				  Ycor[ID][f] = atof(y);
				  if(f < firstFrame[ID])
				  {
					  firstFrame[ID] = f;
				  }
				  if(f > lastFrame[ID])
				  {
					  lastFrame[ID] = f;
				  }
			      if(IsFundamentalTinTout==true)
				{
			    	  if(within(make<point_2d>(atof(x),atof(y)), measurezone)&&!(IsinMeasurezone[ID]))
				    {
				      Tin[ID]=f;
				      IsinMeasurezone[ID] = true;
				    }
			    	  if((!within(make<point_2d>(atof(x),atof(y)), measurezone))&&IsinMeasurezone[ID])
				    {
				      Tout[ID]=f;
				      IsinMeasurezone[ID] = false;
				    }
				}
//				  printf("ID=%d, X=%.2f, Y=%.2f\n", ID, X, Y);
			  }
		  } //for i
	  }// getFrame number j
  }
  printf("finished test of xmlParser.\n");

  if(calcIndividualfunddata)
  {
	  analysor.initial(datafile);
  }


  if(!xFrame.isEmpty())
   {
 	  for (int f=0; f<numFrames; f++) //read the data frame by frame
 	  {
 		  xFrame = xMainNode.getChildNode("frame", f); // frame j
		  const char* frid = xFrame.getAttribute("ID");
/*
 		  //printf("j = %d\n",f);
*/ 		  printf("frame ID = %d\n",atoi(frid));

 		  int numPedsInFrame = xFrame.nChildNode("agent"); // how many agents in this frame
 		  //printf("This frame has %d peds\n",numPedsInFrame);
		  float *XInFrame = new float[numPedsInFrame]; 				// save the X coordinates of pedestrian in the geometry in this frame
		  float *YInFrame = new float[numPedsInFrame];				// save the Y coordinates of pedestrian in the geometry in this frame
		  float *VInFrame = new float[numPedsInFrame]; 	// save the instantaneous velocity of pedestrians in the geometry in this frame

 		  for (int i=0; i<numPedsInFrame; i++) //read pedestrians in frame j
 		  {
 			  if(!xFrame.getChildNode("agent", i).isEmpty())
 			  {
                   //get agent id, x, y
 				  const char* x = xFrame.getChildNode("agent", i).getAttribute("xPos");
 				  const char* y = xFrame.getChildNode("agent", i).getAttribute("yPos");
 				  const char* id = xFrame.getChildNode("agent", i).getAttribute("ID");
 				  int ID = atoi(id);
 				 XInFrame[i] = atof(x);
 				 YInFrame[i] = atof(y);

 		 		  if(use_Vx)
 		 		  {
 		 		    	int Tpast = f - deltaF;
 		 		    	int Tfuture = f + deltaF;
 		 			  	if((Tpast >=firstFrame[ID])&&(Tfuture <= lastFrame[ID]))
 		 		    	{
 		 		    		VInFrame[i] = fps*0.01*(Xcor[ID][Tfuture] - Xcor[ID][Tpast])/(2.0 * deltaF);  //one dimensional velocity
 		 		    	}
 		 			  	else if((Tpast <firstFrame[ID])&&(Tfuture <= lastFrame[ID]))
 		 		    	{
 		 		    		VInFrame[i] = fps*0.01*(Xcor[ID][Tfuture] - Xcor[ID][f])/(deltaF);  //one dimensional velocity
 		 		    	}
 		 			  	else if((Tpast >=firstFrame[ID])&&(Tfuture > lastFrame[ID]))
 		 		    	{
 		 		    		VInFrame[i] = fps*0.01*(Xcor[ID][f] - Xcor[ID][Tpast])/( deltaF);  //one dimensional velocity
 		 		    	}
 		 		  }
 		 		  if(use_Vy)
 		 		  {
		 		    	int Tpast = f - deltaF;
		 		    	int Tfuture = f + deltaF;
		 			  	if((Tpast >=firstFrame[ID])&&(Tfuture <= lastFrame[ID]))
		 		    	{
		 		    		VInFrame[i] = fps*0.01*(Ycor[ID][Tfuture] - Ycor[ID][Tpast])/(2.0 * deltaF);  //one dimensional velocity
		 		    	}
		 			  	else if((Tpast <firstFrame[ID])&&(Tfuture <= lastFrame[ID]))
		 		    	{
		 		    		VInFrame[i] = fps*0.01*(Ycor[ID][Tfuture] - Ycor[ID][f])/(deltaF);  //one dimensional velocity
		 		    	}
		 			  	else if((Tpast >=firstFrame[ID])&&(Tfuture > lastFrame[ID]))
		 		    	{
		 		    		VInFrame[i] = fps*0.01*(Ycor[ID][f] - Ycor[ID][Tpast])/( deltaF);  //one dimensional velocity
		 		    	}
 		 		  }
 		 		  if(use_Vxy)
 		 		  {
		 		    	int Tpast = f - deltaF;
		 		    	int Tfuture = f + deltaF;
		 			  	if((Tpast >=firstFrame[ID])&&(Tfuture <= lastFrame[ID]))
		 		    	{
		 		    		VInFrame[i] = fps*0.01*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][Tpast]),2))/(2.0 * deltaF);  //two dimensional velocity

		 		    	}
		 			  	else if((Tpast <firstFrame[ID])&&(Tfuture <= lastFrame[ID]))
		 		    	{
		 		    		VInFrame[i] = fps*0.01*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][f]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][f]),2))/(deltaF);
		 		    	}
		 			  	else if((Tpast >=firstFrame[ID])&&(Tfuture > lastFrame[ID]))
		 		    	{
		 		    		VInFrame[i] = fps*0.01*sqrt(pow((Xcor[ID][f] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][f] - Ycor[ID][Tpast]),2))/(deltaF);  //two dimensional velocity
		 		    	}
 		 		  }
 				  //printf("ID=%d, X=%.2f, Y=%.2f, Vx=%.2f\n", ID, Xcor[ID][f],Ycor[ID][f], VInFrame[i]);
 				  if(IsFlowVelocity)
 				    {
 						  bool IspassLine=false;
 						  IspassLine=analysor.IsPassLine(Line_startX,Line_startY, Line_endX, Line_endY,Xcor[ID][f-1],Ycor[ID][f-1],Xcor[ID][f],Ycor[ID][f]);
							if(IspassLine==true)
							{
								ClassicFlow++;
								V_deltaT+=VInFrame[i];
							 }
 				    }
 			  }
 		  } //for i

 		 //printf("We begin to calculate some results now.");
 		  //----------------------------------start to calculate some result below------------------------------------------------

 	      if(IsFlowVelocity)
 		{
 		  AccumPedsPassLine.push_back(ClassicFlow);
 		  AccumVPassLine.push_back(V_deltaT);
 		}

 	      float classicVelocity=0;
 	      float classicDensity=0;
 	      int PedinMeasurearea=0;

		  if(IsClassicMethod)
		    {
			  	  for (int i=0; i<numPedsInFrame; i++)
			  	  {
			  		  if(within(make<point_2d>(XInFrame[i], YInFrame[i]), measurezone))
			  		  {
			  			  classicVelocity+=VInFrame[i];
			  			  PedinMeasurearea++;
			  		  }
			  	  }
				  classicDensity=PedinMeasurearea*10000.00/area(measurezone);
				  DensityPerFrame[f]=classicDensity;

				  if(PedinMeasurearea!=0)
				    {
					  classicVelocity /= (1.0*PedinMeasurearea);
				    }
				  else
				    {
					  classicVelocity = 0;
				    }
				  fprintf(fClassic_rho_v,"%d\t%.3f\t%.3f\n",atoi(frid), classicDensity,classicVelocity);
		    }

		  //------------------Voronoi Method---------------------------------
		     if(IsVoronoiMethod)
		     {

		 		  vector<polygon_2d> polygons;
		 		  vector<polygon_2d>::iterator polygon_iterator;
		 		  polygons = getVoronoiPolygons(XInFrame, YInFrame,numPedsInFrame, 2.0*xmin-xmax, 2.0*xmax-xmin, 2.0*ymin-ymax, 2.0*ymax-ymin);
		 		  if(cutbycircle)
		 		  {
		 			  polygons = cutPolygonsWithCircle(polygons, XInFrame, YInFrame, 50);
		 		  }

		 		 /*-----------------------------!!!! pay attention!!!!-------------------------
		 		  * the function cutPolygonsWithGgeometry() will change the pointxs and pointys
		 		  * if the geometry doesn't contain all the points (In other words,
		 		  * the geometry is smaller that the actual used space)
				*/
		 		  polygons = cutPolygonsWithGgeometry(polygons, geom, XInFrame, YInFrame);


		 		  float VoronoiVelocity;
		 		  //int Pedsingeometry=uniqueVelocity.size();
		 		  if(numPedsInFrame>0)
		 		{
/*		 		  float uniqueV[Pedsingeometry];
		 		  for(int m=0;m<Pedsingeometry;m++)
		 			{
		 			  uniqueV[m]=uniqueVelocity[m];
		 			}*/
		 		  VoronoiVelocity=analysor.getVoronoiVelocity(polygons,VInFrame,measurezone);

		 		  if(calcIndividualfunddata)
		 			{
//		 			  if(i>beginstationary&&i<endstationary)
		 			{
		 			  analysor.getIndividualfundmentaldiagram(polygons,VInFrame,measurezone);
		 			}
		 			}
		 		}
		 		  else
		 		{
		 		  VoronoiVelocity=0;
		 		}

		 		float VoronoiDensity=analysor.getVoronoiDensity(polygons, measurezone);
		 		fprintf(fVoronoi_rho_v,"%d\t%.3f\t%.3f\n",atoi(frid),VoronoiDensity, VoronoiVelocity);

				  //------------------field analysis----------------------------------------------------------
				  if(fieldAnalysis)
				{
				  //char buffer[33];
				  std::string buffer(frid);
				  //itoa(i,buffer,10);
				  #ifdef WIN32
				  	  string fieldvelocity=outputPath+"\\field\\velocity\\fieldvelocity"+datafile+"_"+buffer+".dat";
				  	  string fieldensity=outputPath+"\\field\\density\\fielddensity"+datafile+"_"+buffer+".dat";
				  #else
				  	  string fieldvelocity=outputPath+"field/velocity/fieldvelocity"+datafile+"_"+buffer+".dat";
				  	  string fieldensity=outputPath+"field/density/fielddensity"+datafile+"_"+buffer+".dat";
				  #endif
				  FILE *fieldresult;
				  if((fieldresult=fopen(fieldvelocity.c_str(),"w"))==NULL)
					{
					  printf("cannot open the file <%s> to write the field data\n",fieldvelocity.c_str());
					  exit(0);
					}
				  FILE *fielddensity;
				  if((fielddensity=fopen(fieldensity.c_str(),"w"))==NULL)
					{
					  printf("cannot open the file to write the field density\n");
					  exit(0);
					}
				  for(int row_i=0;row_i<NRow;row_i++) //
					{
					  for(int colum_j=0;colum_j<NColumn;colum_j++)
					{
					  polygon_2d measurezoneXY=geometry.MeasureArea(low_ed_x+colum_j*scale_x,low_ed_y+row_i*scale_y,scale_x,scale_y);
					  float densityXY=analysor.getVoronoiDensity(polygons,measurezoneXY);
					  fprintf(fielddensity,"%.3f\t",densityXY);
					  float velocityXY;
					  if(numPedsInFrame>0)
						{
						  velocityXY=analysor.getVoronoiVelocity(polygons,VInFrame,measurezoneXY);
						}
					  else
						{
						  velocityXY=0;
						}
					  fprintf(fieldresult,"%.3f\t",velocityXY);
					}
					  fprintf(fielddensity,"\n");
					  fprintf(fieldresult,"\n");
					}
				  fclose(fieldresult);
				  fclose(fielddensity);
				}

		 		 //------------the following codes is written to output the Voronoi polygons of a frame-----------
				  if(IsOutputGraph)
				{
				  std::string buffer(frid);
				  #ifdef WIN32
				  string point=outputPath+"\\VoronoiCell\\points"+datafile+"_"+buffer+".dat";
				  string polygon=outputPath+"\\VoronoiCell\\polygon"+datafile+"_"+buffer+".dat";
				  string v_individual=outputPath+"\\VoronoiCell\\speed"+datafile+"_"+buffer+".dat";
				  #else
				  string point=outputPath+"VoronoiCell/points"+datafile+"_"+buffer+".dat";
				  string polygon=outputPath+"VoronoiCell/polygon"+datafile+"_"+buffer+".dat";
				  string v_individual=outputPath+"VoronoiCell/speed"+datafile+"_"+buffer+".dat";
				  #endif
				  ofstream points (point.c_str());
				  ofstream polys (polygon.c_str());
				  ofstream velo (v_individual.c_str());

				  for(polygon_iterator=polygons.begin(); polygon_iterator!=polygons.end();polygon_iterator++)
					{
					  polys << dsv(*polygon_iterator) << endl;
					}
			    	for(int pts=0;pts<numPedsInFrame;pts++)
					{
					  points << XInFrame[pts] << "\t" << YInFrame[pts] << endl;
					  velo << fabs(VInFrame[pts]) << endl;
					}
				  points.close();
				  polys.close();
				  velo.close();
				}

		     }


 		  delete []XInFrame;
 		  delete []YInFrame;
 		  delete []VInFrame;
 	  }// getFrame number j

 	  fclose(fClassic_rho_v);
 	  fclose(fVoronoi_rho_v);
	     //--------------------Fundamental diagram based on Tin and Tout----------------------------------------------------------------------
	     if(IsFundamentalTinTout)
	       {
			  #ifdef WIN32
				  string FD_TinTout=  "Fundamental_Diagram\\TinTout\\FDTinTout_"+datafile+".dat";
			  #else
				  string FD_TinTout=  "Fundamental_Diagram/TinTout/FDTinTout_"+datafile+".dat";
			  #endif
			  cout << "Fundamental diagram based on Tin and Tout"<<endl;
			  analysor.getFundamentalTinTout(Tin,Tout,DensityPerFrame, fps, LengthMeasurementarea,MaxNumofPed, FD_TinTout); //MC. 15.8.12. replaced "datafile" by results
			  delete []DensityPerFrame;
	       }
	     //-----------------------------------------------------------------------------------------------------------------------------------
	     if(IsFlowVelocity)
	       {
			  #ifdef WIN32
				  string FD_FlowVelocity=  "Fundamental_Diagram\\FlowVelocity\\FDFlowVelocity_"+datafile+".dat";
			  #else
				  string FD_FlowVelocity=  "Fundamental_Diagram/FlowVelocity/FDFlowVelocity_"+datafile+".dat";
			  #endif
			  analysor.FlowRate_Velocity(DeltaT,fps, AccumPedsPassLine,AccumVPassLine,FD_FlowVelocity);
	       }

   }

  cout <<"complete successfully"<<endl;
  return 0;
}

