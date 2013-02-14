

#include "Analysis.h"
#include "geometry/Room.h"
#include "general/xmlParser.h"

#include "VoronoiDiagramGenerator.h"
#include "VoronoiPolygons.h"

#include <iostream>
#include <fstream>
using namespace std;

/************************************************
// Konstruktoren
 ************************************************/

Analysis::Analysis() {

	_building = NULL;
	_iod = new IODispatcher();
	fps=1;
	_numFrames = 10;

	Tin = NULL;
	Tout = NULL;
    LengthMeasurementarea = 200;  // the length of the measurement area
    MaxNumofPed =0;  //the maximum index of the pedestrian in the trajectory data
	DeltaF=5;											// half of the time interval that used to calculate instantaneous velocity of ped i.
																	// here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
	Xcor = NULL;
	Ycor = NULL;
	firstFrame = NULL;   // Record the first frame of each pedestrian
	lastFrame = NULL;	// Record the last frame of each pedestrian
    DeltaT =160;   // the time interval to calculate the classic flow
    Line_startX = 0;  			//the coordinate of the line used to calculate the flow and velocity
    Line_startY =10;
    Line_endX = 0;
    Line_endY =100;
	IsFlowVelocity = false; 						// Method A (Zhang2011a)
	IsFundamentalTinTout = false; 			// Method B (Zhang2011a)
	IsClassicMethod = true; 					// Method C //calculate and save results of classic in separate file
	IsVoronoiMethod = false;  					// Method D--Voronoi method
    cutbycircle = false;  //Adjust whether cut each original voronoi cell by a circle
    IsGetProfile = false;   // Whether make field analysis or not
    IsOutputGraph = false;   // Whether output the data for plot the fundamental diagram each frame
    IscalcIndividualFD = false; //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
    VComponent = 'B'; // to mark whether x, y or x and y coordinate are used when calculating the velocity
    //AccumPedsPassLine = NULL; // the accumulative pedestrians pass a line with time
    //AccumVPassLine = NULL;  // the accumulative instantaneous velocity of the pedestrians pass a line
	fps = 16;												// Frame rate of data
	fClassic_rho_v = NULL;
	fVoronoi_rho_v = NULL;
	IndividualFD = NULL;

	scale_x = 10;   // the size of the grid
	scale_y = 10;
	low_ed_x = 0;// LOWest vertex of the geometry (x coordinate)
	low_ed_y = 0; //  LOWest vertex of the geometry (y coordinate)
	high_ed_x = 10; // Highest vertex of the geometry
	high_ed_y = 10;

}

Analysis::~Analysis() {
	delete _building;
	delete _iod;
	delete  firstFrame;
	delete  lastFrame;
	delete  Tin;
	delete  Tout;

	for (int i=0; i<MaxNumofPed; i++)
	{
		 delete Xcor[i];
		 delete Ycor[i];
	}
	delete []Xcor;
	delete []Ycor;
}

/************************************************
// Setter-Funktionen
 ************************************************/


Building * Analysis::GetBuilding() const {
	return _building;
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

	pMeasureZone = args->GetMeasureArea();


	IsFlowVelocity = args->GetIsMethodA();
	IsFundamentalTinTout = args->GetIsMethodB();
	IsClassicMethod = args ->GetIsMethodC();
	IsVoronoiMethod = args ->GetIsMethodD();
	LengthMeasurementarea = args->GetLengthMeasurementArea();
	DeltaF = args->GetDelatT_Vins();
	DeltaT = args->GetTimeInterval_A();
	Line_startX = args->GetLine_startx();
	Line_startY = args->GetLine_starty();
	Line_endX = args->GetLine_endx();
	Line_endY = args->GetLine_endy();
	cutbycircle = args->GetIsCutByCircle();
	IsGetProfile = args->GetIsGetProfile();
	IsOutputGraph = args->GetIsOutputGraph();
	IscalcIndividualFD = args->GetIsIndividualFD();
	VComponent = args->GetVComponent();
	scale_x = args->GetScale_x();
	scale_y = args->GetScale_y();

	if(IsClassicMethod)
	{
			#ifdef WIN32
		string results_C=  ".\\Output\\Fundamental_Diagram\\Classical_Voronoi\\rho_v_Classic_"+TrajectoryName+".dat";
		//		  string results_C=  "rho_v_Classic_"+TrajectoryName+".dat";
			#else
				  string results_C=  "Output/Fundamental_Diagram/Classical_Voronoi/rho_v_Classic_"+TrajectoryName+".dat";
			#endif
			  if((fClassic_rho_v=fopen(results_C.c_str(),"w"))==NULL)
				{
				  printf("cannot open the file to write classical density and velocity\n");
				  exit(0);
				}
			  fprintf(fClassic_rho_v,"#Frame \tclassical density(m^(-2))\t	classical velocity(m/s)\n");
	}
	if(IsVoronoiMethod)
	{
			#ifdef WIN32
				  //string results_V=  "Output\\Fundamental_Diagram\\Classical_Voronoi\\rho_v_Voronoi_"+TrajectoryName+".dat";
				  string results_V=  "rho_v_Voronoi_"+TrajectoryName+".dat";
			#else
				  string results_V=  "Output/Fundamental_Diagram/Classical_Voronoi/rho_v_Voronoi_"+TrajectoryName+".dat";
			#endif
			  if((fVoronoi_rho_v=fopen(results_V.c_str(),"w"))==NULL)
				{
				  Log->Write("cannot open the file to write Voronoi density and velocity\n");
				  exit(0);
				}
			  fprintf(fVoronoi_rho_v,"#Frame \t Voronoi density(m^(-2))\t	Voronoi velocity(m/s)\n");
	}
	if(IscalcIndividualFD)
	{
		#ifdef WIN32
			//string Individualfundment="Output\\Fundamental_Diagram\\Individual_FD\\IndividualFD"+TrajectoryName+".dat";
			string Individualfundment="IndividualFD"+TrajectoryName+".dat";
		#else
			  string Individualfundment="Output/Fundamental_Diagram/Individual_FD/IndividualFD"+TrajectoryName+".dat";
		#endif

			if((IndividualFD=fopen(Individualfundment.c_str(),"w"))==NULL)
			{
				Log->Write("cannot open the file individual\n");
				exit(0);
			}
			fprintf(IndividualFD,"#Individual density(m^(-2))\t	Individual velocity(m/s)\n");
	}
	// IMPORTANT: do not change the order in the following..
	sprintf(tmp, "\tGeometrie: [%s]\n", args->GetGeometryFilename().c_str());
	s.append(tmp);
	Log->Write("INFO: \t" + s);
	pGeoPoly = ReadGeometry(args->GetGeometryFilename());
	//pBuilding->WriteToErrorLog();

	TrajectoryName = args->GetTrajectoryName();
	TrajectoryFile = args->GetTrajectoriesFile();
	Log->Write(string("INFO: \t") + TrajectoryFile);
	ReadTrajetories(TrajectoryFile);
}

polygon_2d Analysis::ReadGeometry(const string& geometryFile){

	_building = new Building();
	_building->LoadBuilding(geometryFile);
	double geo_minX  = FLT_MAX;
	double geo_minY  = FLT_MAX;
	double geo_maxX  = -FLT_MAX;
	double geo_maxY  = -FLT_MAX;
	polygon_2d geoPoly;
	// create the polygons
	_building->InitGeometry();
	vector<Obstacle*> GeoObst;
	for(int i=0;i<_building->GetNumberOfRooms();i++){
		Room* room=_building->GetRoom(i);

		for( int j=0;j<room->GetNumberOfSubRooms();j++){
			SubRoom* subroom = room->GetSubRoom(i);
			const vector<Point>& temp_GeoPoly = subroom->GetPolygon();
			for (unsigned int j = 0; j< temp_GeoPoly.size();j++){
				append(geoPoly, make<point_2d>(temp_GeoPoly[j]._x, temp_GeoPoly[j]._y));
				geo_minX = (temp_GeoPoly[j]._x<=geo_minX) ? temp_GeoPoly[j]._x : geo_minX;
				geo_minY = (temp_GeoPoly[j]._y<=geo_minY) ? temp_GeoPoly[j]._y : geo_minY;
				geo_maxX = (temp_GeoPoly[j]._x>=geo_maxX) ? temp_GeoPoly[j]._x : geo_maxX;
				geo_maxY = (temp_GeoPoly[j]._y>=geo_maxY) ? temp_GeoPoly[j]._y : geo_maxY;

			}
			correct(geoPoly);
			//std::cout << "poly without hole: " << boost::geometry::dsv(geoPoly) << std::endl;
			GeoObst = subroom->GetAllObstacles();
		}
		for (unsigned int k = 0; k < GeoObst.size(); k++) {
			//std::cout<< GeoObst.size()<<std::endl;
			//std::cout<< pBuilding->GetAnzRooms() <<'\t'<<room->GetAnzSubRooms()<<std::endl;
			const vector<Point>& temp_obst = GeoObst[k]->GetPolygon();

			geoPoly.inners().resize(k+1);
			geoPoly.inners().back();
			model::ring<point_2d>& inner = geoPoly.inners().back();
			for (unsigned int j = 0; j< temp_obst.size();j++){
				append(inner, make<point_2d>(temp_obst[j]._x, temp_obst[j]._y));
			}

			correct(geoPoly);
			//std::cout << "poly with hole: " << boost::geometry::dsv(geoPoly) << std::endl;
		}
	}

	high_ed_x = geo_maxX;
	high_ed_y = geo_maxY;
	low_ed_x = geo_minX;
	low_ed_y = geo_minY;

	return geoPoly;
}

void Analysis::ReadTrajetories(string trajectoriesFile){
	//---------read the trajectory data from file-----------
	////////////////////////////--------------Parse the xmlFile---------------///////////////////////////////////////////////////////

	  printf(">>> input file <%s>\n",trajectoriesFile.c_str());

	  XMLNode xMainNode = XMLNode::openFileHelper(trajectoriesFile.c_str(),"trajectoriesDataset");
	  XMLNode xHeader = xMainNode.getChildNode("header"); // header
	  XMLNode xFrame = xMainNode.getChildNode("frame"); // frame
	  _numFrames = xMainNode.nChildNode("frame"); // how much frames

	  printf("numFrames = %d\n",_numFrames);
	  if(!xHeader.getChildNode("agents").isEmpty()){
		  const char* N = xHeader.getChildNode("agents").getText();
		  MaxNumofPed = atoi(N);
		  printf("N=%d\n", MaxNumofPed);

		  const char* NF = xHeader.getChildNode("frameRate").getText();
		  fps = atoi(NF);
		  printf("fps=%d\n", fps);
	  }
	  Xcor = new float* [MaxNumofPed];
	  Ycor = new float* [MaxNumofPed];
	  for (int i=0; i<MaxNumofPed; i++)
	  {
		  Xcor[i] = new float [_numFrames];
		  Ycor[i] = new float [_numFrames];
	  }
	  firstFrame = new int[MaxNumofPed];  // Record the first frame of each pedestrian
	  lastFrame = new int[MaxNumofPed];  // Record the last frame of each pedestrian
	  Tin = new int[MaxNumofPed];				// Record the time of each pedestrian entering measurement area
	  Tout = new int[MaxNumofPed];				// Record the time of each pedestrian exiting measurement area
	  bool IsinMeasurezone[MaxNumofPed];  // Record whether pedestrian i is in measurement area or not

	  for(int i = 0; i <MaxNumofPed; i++)
	  {
		  for (int j = 0; j < _numFrames; j++)
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



	  if(!xFrame.isEmpty())
	  {
		  for (int f=0; f<_numFrames; f++) //read the data frame by frame
		  {
			  xFrame = xMainNode.getChildNode("frame", f); // frame j
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
					  int ID = atoi(id)-1;
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
						  if(within(make<point_2d>(atof(x),atof(y)), pMeasureZone)&&!(IsinMeasurezone[ID]))
						{
						  Tin[ID]=f;
						  IsinMeasurezone[ID] = true;
						}
						  if((!within(make<point_2d>(atof(x),atof(y)), pMeasureZone))&&IsinMeasurezone[ID])
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

}

int Analysis::RunAnalysis()
{
	  XMLNode xMainNode = XMLNode::openFileHelper(TrajectoryFile.c_str(),"trajectoriesDataset");
	  XMLNode xHeader = xMainNode.getChildNode("header"); // header
	  XMLNode xFrame = xMainNode.getChildNode("frame"); // frame
	  _numFrames = xMainNode.nChildNode("frame"); // how much frames
	  int ClassicFlow=0; // the number of pedestrians pass a line in a certain time
	  float V_deltaT=0;   // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N
	  float DensityPerFrame[_numFrames+1];
	  for(int i=0;i<=_numFrames;i++)
		{
		  DensityPerFrame[i]=0;
		}
	  if(!xFrame.isEmpty())
	   {
	 	  for (int f=0; f<_numFrames; f++) //read the data frame by frame
	 	  {
	 		  xFrame = xMainNode.getChildNode("frame", f); // frame j
			  const char* frid = xFrame.getAttribute("ID");
	/*
	 		  //printf("j = %d\n",f);
	*/ 		  printf("frame ID = %d\n",atoi(frid));

	 		  int numPedsInFrame = xFrame.nChildNode("agent"); 		// how many agents in this frame
	 		  //printf("This frame has %d peds\n",numPedsInFrame);
			  float *XInFrame = new float[numPedsInFrame]; 				// save the X coordinates of pedestrian in the geometry in this frame
			  float *YInFrame = new float[numPedsInFrame];				// save the Y coordinates of pedestrian in the geometry in this frame
			  float *VInFrame = new float[numPedsInFrame]; 				// save the instantaneous velocity of pedestrians in the geometry in this frame

	 		  for (int i=0; i<numPedsInFrame; i++) //read pedestrians in frame j
	 		  {
	 			  if(!xFrame.getChildNode("agent", i).isEmpty())
	 			  {
	                   //get agent id, x, y
	 				  const char* x = xFrame.getChildNode("agent", i).getAttribute("xPos");
	 				  const char* y = xFrame.getChildNode("agent", i).getAttribute("yPos");
	 				  const char* id = xFrame.getChildNode("agent", i).getAttribute("ID");
	 				  int ID = atoi(id)-1;
	 				 XInFrame[i] = atof(x);
	 				 YInFrame[i] = atof(y);
	 				 int Tpast = f - DeltaF;
	 				 int Tfuture = f + DeltaF;
	 				 VInFrame[i] = GetVinFrame(f, Tpast, Tfuture, firstFrame[ID], lastFrame[ID], Xcor[ID][Tpast], Xcor[ID][f], Xcor[ID][Tfuture], Ycor[ID][Tpast], Ycor[ID][f], Ycor[ID][Tfuture], VComponent);

	 				  //printf("ID=%d, X=%.2f, Y=%.2f, Vx=%.2f\n", ID, Xcor[ID][f],Ycor[ID][f], VInFrame[i]);
//	 				  if(IsFlowVelocity)
//	 				    {
	 						  bool IspassLine=false;
	 						  IspassLine = IsPassLine(Line_startX,Line_startY, Line_endX, Line_endY,Xcor[ID][f-1],Ycor[ID][f-1],Xcor[ID][f],Ycor[ID][f]);
								if(IspassLine==true)
								{
									ClassicFlow++;
									V_deltaT+=VInFrame[i];
								 }
//	 				    }
	 			  }
	 		  } //for i
//	 		   if(IsFlowVelocity)
//				{
				  AccumPedsPassLine.push_back(ClassicFlow);
				  AccumVPassLine.push_back(V_deltaT);
//				}

			   if(IsClassicMethod)
				{
				   float ClassicDensity = getClassicalDensity(XInFrame, YInFrame, numPedsInFrame, pMeasureZone);
				   float ClassicVelocity = getClassicalVelocity(XInFrame, YInFrame, VInFrame, numPedsInFrame, pMeasureZone);
				   DensityPerFrame[f]=ClassicDensity;
				   fprintf(fClassic_rho_v,"%d\t%.3f\t%.3f\n",atoi(frid), ClassicDensity,ClassicVelocity);
				}

	 					  //------------------Voronoi Method---------------------------------
			   if(IsVoronoiMethod)
			   {

					  vector<polygon_2d> polygons;
					  polygons = getVoronoiPolygons(XInFrame, YInFrame,numPedsInFrame, 2.0*low_ed_x-high_ed_x, 2.0*high_ed_x-low_ed_x, 2.0*low_ed_y-high_ed_y, 2.0*high_ed_y-low_ed_y);
					  if(cutbycircle)
					  {
						  polygons = cutPolygonsWithCircle(polygons, XInFrame, YInFrame, 50);
					  }

					 /*-----------------------------!!!! pay attention!!!!-------------------------
					  * the function cutPolygonsWithGgeometry() will change the pointxs and pointys
					  * if the geometry doesn't contain all the points (In other words,
					  * the geometry is smaller that the actual used space)
					*/
					  polygons = cutPolygonsWithGgeometry(polygons, pGeoPoly, XInFrame, YInFrame);
					  float VoronoiVelocity;
					  if(numPedsInFrame>0)
					  {
						  VoronoiVelocity=getVoronoiVelocity(polygons,VInFrame,pMeasureZone);
					  }
					  else
					  {
						  VoronoiVelocity=0;
					  }
					  float VoronoiDensity=getVoronoiDensity(polygons, pMeasureZone);
					  fprintf(fVoronoi_rho_v,"%d\t%.3f\t%.3f\n",atoi(frid),VoronoiDensity, VoronoiVelocity);

					  if(IscalcIndividualFD)
						{
						  	if(numPedsInFrame>0)
						  	{
						  		// if(i>beginstationary&&i<endstationary)
								{
									getIndividualFD(polygons,VInFrame,pMeasureZone);
								}
						  	}
						}
					  //------------------field analysis----------------------------------------------------------
					  if(IsGetProfile)
					{
						  if(numPedsInFrame>0)
						  {
							  GetProfiles(string(frid), polygons, VInFrame);
						  }
					}
					 //------------the following codes is written to output the Voronoi polygons of a frame-----------
					  if(IsOutputGraph)
					{
					  OutputVoroGraph(string(frid), polygons, numPedsInFrame,XInFrame, YInFrame,VInFrame);
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
				  string FD_TinTout=  "Output\\Fundamental_Diagram\\TinTout\\FDTinTout_"+TrajectoryName+".dat";
			  #else
				  string FD_TinTout=  "Output/Fundamental_Diagram/TinTout/FDTinTout_"+TrajectoryName+".dat";
			  #endif
			  cout << "Fundamental diagram based on Tin and Tout"<<endl;
			  getFundamentalTinTout(Tin,Tout,DensityPerFrame, fps, LengthMeasurementarea,MaxNumofPed, FD_TinTout); //MC. 15.8.12. replaced "datafile" by results
		   }
		 //-----------------------------------------------------------------------------------------------------------------------------------
		  if(IsFlowVelocity)
		   {
			  #ifdef WIN32
				  string FD_FlowVelocity=  "Output\\Fundamental_Diagram\\FlowVelocity\\FDFlowVelocity_"+TrajectoryName+".dat";
			  #else
				  string FD_FlowVelocity=  "Output/Fundamental_Diagram/FlowVelocity/FDFlowVelocity_"+TrajectoryName+".dat";
			  #endif
			  FlowRate_Velocity(DeltaT,fps, AccumPedsPassLine,AccumVPassLine,FD_FlowVelocity);
		   }

 }




	/*	  points.push_back(Point(0, 0));
	 	  points.push_back(Point(1, 6));
	 	  points.push_back(Point(8, 10));
	 	  points.push_back(Point(15, 9));
	 	  points.push_back(Point(20, 18));
	 	  points.push_back(Point(35, 25));
	 	  // Construction of the Voronoi Diagram.
	 	  voronoi_diagram<double> vd;
	 	  construct_voronoi(points.begin(), points.end(), &vd);
	 	  // Traversing Voronoi Graph.
	 	  {
	 	    printf("Traversing Voronoi graph.\n");
	 	    printf("Number of visited primary edges using edge iterator: %d\n",
	 	        iterate_primary_edges1(vd));
	 	    printf("Number of visited primary edges using cell iterator: %d\n",
	 	        iterate_primary_edges2(vd));
	 	    printf("Number of visited primary edges using vertex iterator: %d\n",
	 	        iterate_primary_edges3(vd));
	 	    printf("\n");
	 	  }
	 	  // Linking Voronoi cells with input geometries.
	 	  {
	 	    unsigned int cell_index = 0;
	 	    for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin(); it != vd.cells().end(); ++it)
	 	    {
	 	    	if (it->contains_point())
	 			  {
	 				std::size_t index = it->source_index();
	 				Point p = points[index];
	 				printf("Cell #%ud contains a point: (%d, %d).\n", cell_index, x(p), y(p));
	 			  }
	 	      ++cell_index;
	 	    }
	 	  }*/


	/*
	 	int frameNr = 1; // Frame Number
	 	int writeInterval = (int) ((1. / fps) / pDt + 0.5);
	 	writeInterval = (writeInterval <= 0) ? 1 : writeInterval; // mustn't be <= 0
	 */

	/*
	 	// writing the header

	 	iod->WriteHeader(pNPeds, fps, pBuilding);
	 	iod->WriteGeometry(pBuilding);
	 	iod->WriteFrame(0, pBuilding);


	 	// writing the footer
	 	iod->WriteFooter();

	 	//return the evacuation time
	 */
	return 0;
}

/*
 *  according to the location of a pedestrian in adjacent frame (pt1_X,pt1_Y) and (pr2_X,pt2_Y), we
 *  adjust whether he pass the line from Line_start to Line_end
 */
bool Analysis::IsPassLine(float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y)
{
	float x1=Line_startX;
	float y1=Line_startY;
	float x2=Line_endX;
	float y2=Line_endY;
	float x3=pt1_X;
	float y3=pt1_Y;
	float x4=pt2_X;
	float y4=pt2_Y;

	float d=(y2-y1)*(x4-x3)-(y4-y3)*(x2-x1);
	if(d==0)
	{
		return false;
	}
	else
	{
		float x0=((x2-x1)*(x4-x3)*(y3-y1)+(y2-y1)*(x4-x3)*x1-(y4-y3)*(x2-x1)*x3)/d;
		float y0=((y2-y1)*(y4-y3)*(x3-x1)+(x2-x1)*(y4-y3)*y1-(x4-x3)*(y2-y1)*y3)/(-1.0*d);
		float temp1=(x0-x1)*(x0-x2);
		float temp2=(x0-x3)*(x0-x4);
		float temp3=(y0-y1)*(y0-y2);
		float temp4=(y0-y3)*(y0-y4);
		if(temp1<0.00000001)
		{
			temp1=0.0;
		}
		if(temp2<0.00000001)
		{
			temp2=0.0;
		}
		if(temp3<0.00000001)
		{
			temp3=0.0;
		}
		if(temp4<0.00000001)
		{
			temp4=0.0;
		}
		if(temp1<=0.0&&temp2<=0.0&&temp3<=0.0&&temp4<=0.0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------
/*
 * output: the fundamental diagram based on the time a pedestrian enter and exit the measurement area
 * MC 15.08.2012
 * input: outputfile is given not anymore "datafile"
 */
void Analysis::getFundamentalTinTout(int *Tin, int *Tout, float *DensityPerFrame, int fps, int LengthMeasurementarea,int Nped, string ofile)
{
	FILE *fFD_TinTout;
	string fdTinTout=ofile;
	if((fFD_TinTout=fopen(fdTinTout.c_str(),"w"))==NULL)
	{
		printf("cannot open the file to write the TinTout data\n");
		exit(0);
	}
	fprintf(fFD_TinTout,"#person Index\t	Individual density(m^(-2))\t	Individual velocity(m/s)\n");
	for(int i=1;i<=Nped;i++)
	{
		float velocity_temp=fps*0.01*LengthMeasurementarea/(Tout[i]-Tin[i]); //0.01 cm -->m
		float density_temp=0;
		for(int j=Tin[i];j<Tout[i];j++)
		{
			density_temp+=DensityPerFrame[j];
		}
		density_temp/=(Tout[i]-Tin[i]);
		fprintf(fFD_TinTout,"%d\t%f\t%f\n",i,density_temp,velocity_temp);
		//getc(stdin);
	}
	fclose(fFD_TinTout);
}

//----------------------------------------------------------------------------------------------------------------------------
/*
 * Calculate the Flow rate during a certain time interval DeltaT and the mean velocity passing a line.
 * Note: here the time interval in calculating the flow rate is modified. it is the actual time between the first person and last person
 * passing the line in DeltaT.
 * MC 15.08.2012
 * input: outputfile is given not anymore "datafile"
 */
void Analysis::FlowRate_Velocity(int DeltaT, int fps, vector<int> AccumPeds, vector<float> AccumVelocity, string ofile)
{

	FILE *fFD_FlowVelocity;
	string fdFlowVelocity = ofile;  //"Fundamental Diagram\\FlowVelocity\\FundamentalFlowVelocity"+datafile+".dat";
	if((fFD_FlowVelocity=fopen(fdFlowVelocity.c_str(),"w"))==NULL)
	{
		printf("cannot open the file to write the Flow-Velocity data\n");
		exit(0);
	}
	fprintf(fFD_FlowVelocity,"#Flow rate(1/s)	Mean velocity(m/s)\n");
	int TotalTime=AccumPeds.size();  // the total Frame of in the data file
	int TotalPeds=AccumPeds[TotalTime-1];  //the total pedestrians in the data file
	int firstPassT=-1;  // the first time that there are pedestrians pass the line
	int *pedspassT=new int[TotalPeds+1]; // the time for certain pedestrian passing the line
	for(int i=0; i<=TotalPeds; i++)
	{
		pedspassT[i]=-1;
	}

	for(int ix=0; ix<TotalTime; ix++)
	{
		if(AccumPeds[ix]>0 && firstPassT<0)
		{
			firstPassT=ix;
		}
		if(pedspassT[AccumPeds[ix]]<0)
		{
			pedspassT[AccumPeds[ix]]=ix;
		}
	}
	for(int i=firstPassT+DeltaT; i<TotalTime; i+=DeltaT)
	{
		float flow_rate=fps*(AccumPeds[i]-AccumPeds[i-DeltaT])*1.00/(pedspassT[AccumPeds[i]]-pedspassT[AccumPeds[i-DeltaT]]);
		float MeanV=(AccumVelocity[i]-AccumVelocity[i-DeltaT])/(AccumPeds[i]-AccumPeds[i-DeltaT]);
		fprintf(fFD_FlowVelocity,"%.3f\t%.3f\n",flow_rate,MeanV);

	}
	fclose(fFD_FlowVelocity);
	delete []pedspassT;
}

//-----------------------------------------------------------------------------------------------------------------
/*
 *  calculate individual density and velocity using voronoi method. the individual density is defined as the inverse
 *  of the area of the pedestrian's voronoi cell. The individual velocity is his instantaneous velocity at this time.
 *  note that, Only the pedestrians in the measurement area are considered.
 */
void Analysis::getIndividualFD(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea)
{

	vector<polygon_2d>::iterator polygon_iterator;
	float uniquedensity=0;
	float uniquevelocity=0;
	int temp=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d> polygon_list;
		polygon_list v;
		intersection(measureArea, *polygon_iterator, v);
		if(!v.empty())
		{
			uniquedensity=10000/area(*polygon_iterator);
			uniquevelocity=Velocity[temp];
			fprintf(IndividualFD,"%.3f\t%.3f\n",uniquedensity,uniquevelocity);
		}
		temp++;
	}
}

float Analysis::distance(float x1, float y1, float x2, float y2)
{
	float distance=sqrt(pow((x1-x2),2)+pow((y1-y2),2));
	return distance;
}
///---------------------------------------------------------------------------------------------------------------------
/*this function is to obtain the frequency distribution of the pedestrian movement through a line from (Line_startX,Line_startY)
 * to (Line_endX, Line_endY) according to the coordination of a person in two adjacent frames (pt1_X,pt1_Y) and (pt2_X,pt2_Y)
 * input: a empty array that will be used to save the results. "fraction" is the number of same parts that the line will be divided into.
 * output: the number of persons in each part of the line "frequency".
 */
void Analysis::DistributionOnLine(int *frequency,int fraction, float Line_startX,float Line_startY, float Line_endX, float Line_endY,float pt1_X, float pt1_Y,float pt2_X, float pt2_Y)
{
	/* in this function, we firstly calculate the intersection point between the trajectory (the line from point (pt1_X,pt1_Y)
	 *  to (pt2_X,pt2_Y)) of a pedestrian and the Line. then, we determine which part of the line he or she belongs to
	 */
	float sumdistance=distance(Line_startX,Line_startY,Line_endX, Line_endY);
	float A=Line_endY-Line_startY;
	float B=Line_startX-Line_endX;
	float C=Line_endX*Line_startY-Line_startX*Line_endY;
	float d1=A*pt1_X+B*pt1_Y+C;
	float d2=A*pt2_X+B*pt2_Y+C;
	if(d1*d2<0)
	{
		float x1=Line_startX;
		float y1=Line_startY;
		float x2=Line_endX;
		float y2=Line_endY;
		float x3=pt1_X;
		float y3=pt1_Y;
		float x4=pt2_X;
		float y4=pt2_Y;
		float x =((x1 - x2) * (x3 * y4 - x4 * y3) - (x3 - x4) * (x1 * y2 - x2 * y1))
			    		 /((x3 - x4) * (y1 - y2) - (x1 - x2) * (y3 - y4));

		float y =((y1 - y2) * (x3 * y4 - x4 * y3) - (x1 * y2 - x2 * y1) * (y3 - y4))
			    		 /((y1 - y2) * (x3 - x4) - (x1 - x2) * (y3 - y4));
		int index=(int)floor(distance(x,y,x1,y1)*fraction/sumdistance);
		frequency[index]++;
	}
	else if(d1*d2==0)
	{
		if(d1==0)
		{
			int index=(int)floor(distance(Line_startX,Line_startY,pt1_X,pt1_Y)*fraction/sumdistance);
			frequency[index]++;
		}
	}

}

/*
 * calculate the voronoi density according to the area of the voronoi cell and the measurement area.
 * input: the list of the voronoi polygons and the measurement area
 * output: the voronoi density in the measurement area
 * note the unit of the polygons
 */
float Analysis::getVoronoiDensity(vector<polygon_2d> polygon, polygon_2d measureArea)
{
	vector<polygon_2d>::iterator polygon_iterator;
	float density=0;
	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d > polygon_list;
		polygon_list v;
		//intersection_inserter<polygon_2d>(measureArea, *polygon_iterator, std::back_inserter(v));
		intersection(measureArea, *polygon_iterator, v);
		if(!v.empty())
		{
			density+=area(v[0])/area(*polygon_iterator);
			if((area(v[0])/area(*polygon_iterator))>1.00001)
			{
				std::cout<<"this is a wrong result "<<area(v[0])<<'\t'<<area(*polygon_iterator)<<"\n";
			}
		}
	}
	density=10000*density/area(measureArea);
	return density;
}

//---------------------------------------------------------------------------------------------

/*
 * calculate the classical density according to the coordinate of pedestrians (xs,ys) and the measurement area.
 * input: xs,ys, the number of pedestrians in the geometry and the measurement area
 * output: the classical density in the measurement area
 * note that the number of pedestrians should be the pedestrians in the the geometry used to cut the voronoi diagram.this
 * is very important. because sometimes the selected
 * geometry is smaller than the actual one. in this case, some pedestrian can not included in the geometry.
 */
float Analysis::getClassicalDensity(float *xs, float *ys, int pednum, polygon_2d measureArea)
{
	int pedsinMeasureArea=0;
	for(int i=0;i<pednum;i++)
	{
		if(within(make<point_2d>(xs[i], ys[i]), measureArea))
		{
			pedsinMeasureArea++;
		}
	}

	float density=pedsinMeasureArea*10000.00/area(measureArea);
	return density;
}

float Analysis::getClassicalVelocity(float *xs, float *ys, float *VInFrame, int pednum, polygon_2d measureArea){

	int pedsinMeasureArea=0;
	float velocity = 0;
	for(int i=0;i<pednum;i++)
	{
		if(within(make<point_2d>(xs[i], ys[i]), measureArea))
		{
			velocity+=VInFrame[i];
			pedsinMeasureArea++;
		}
	}
	if(pedsinMeasureArea!=0)
	{
		velocity /= (1.0*pedsinMeasureArea);
	}
	else
	{
		velocity = 0;
	}
	return velocity;

}
//---------------------------------------------------------------------------------------------

/*
 * calculate the voronoi velocity according to voronoi cell of each pedestrian and their instantaneous velocity "Velocity".
 * input: voronoi cell and velocity of each pedestrian and the measurement area
 * output: the voronoi velocity in the measurement area
 */
float Analysis::getVoronoiVelocity(vector<polygon_2d> polygon, float* Velocity, polygon_2d measureArea)
{
	vector<polygon_2d>::iterator polygon_iterator;
	float meanV=0;
	int temp=0;

	for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
	{
		typedef std::vector<polygon_2d > polygon_list;

		polygon_list v;
		//intersection_inserter<polygon_2d>(measureArea, *polygon_iterator, std::back_inserter(v));
		intersection(measureArea, *polygon_iterator, v);
		if(!v.empty())
		{
			meanV+=Velocity[temp]*area(v[0])/area(measureArea);
			if((area(v[0])/area(*polygon_iterator))>1.00001)
			{
				std::cout<<"this is a wrong result"<<area(v[0])<<'\t'<<area(*polygon_iterator);
			}
		}
		temp++;
	}

	return meanV;
}

/*
 * this function is to calculate the instantneous velocity of ped ID in Frame Tnow based on his coordinates and his state.
 */

float Analysis::GetVinFrame(int Tnow,int Tpast, int Tfuture, int Tfirst, int Tlast, float Xcor_past, float Xcor_now, float Xcor_future,float Ycor_past, float Ycor_now, float Ycor_future, char VComponent){

	float v=0;

	if(VComponent == 'X')
	{
		if((Tpast >=Tfirst)&&(Tfuture <= Tlast))
		{
			v = fps*0.01*(Xcor_future - Xcor_past)/(2.0 * DeltaF);  //one dimensional velocity
		}
		else if((Tpast <Tfirst)&&(Tfuture <= Tlast))
		{
			v = fps*0.01*(Xcor_future - Xcor_now)/(DeltaF);  //one dimensional velocity
		}
		else if((Tpast >=Tfirst)&&(Tfuture > Tlast))
		{
			v = fps*0.01*(Xcor_now - Xcor_past)/( DeltaF);  //one dimensional velocity
		}
	}
	if(VComponent == 'Y')
	{
		if((Tpast >=Tfirst)&&(Tfuture <= Tlast))
		{
			v = fps*0.01*(Ycor_future - Ycor_past)/(2.0 * DeltaF);  //one dimensional velocity
		}
		else if((Tpast <Tfirst)&&(Tfuture <= Tlast))
		{
			v = fps*0.01*(Ycor_future- Ycor_now)/(DeltaF);  //one dimensional velocity
		}
		else if((Tpast >=Tfirst)&&(Tfuture > Tlast))
		{
			v = fps*0.01*(Ycor_now - Ycor_past)/( DeltaF);  //one dimensional velocity
		}
	}
	if(VComponent == 'B')
	{
		if((Tpast >=Tfirst)&&(Tfuture <= Tlast))
		{
			v = fps*0.01*sqrt(pow((Xcor_future - Xcor_past),2)+pow((Ycor_future - Ycor_past),2))/(2.0 * DeltaF);  //two dimensional velocity

		}
		else if((Tpast <Tfirst)&&(Tfuture <= Tlast))
		{
			v = fps*0.01*sqrt(pow((Xcor_future - Xcor_now),2)+pow((Ycor_future - Ycor_now),2))/(DeltaF);
		}
		else if((Tpast >=Tfirst)&&(Tfuture > Tlast))
		{
			v = fps*0.01*sqrt(pow((Xcor_now - Xcor_past),2)+pow((Ycor_now - Ycor_past),2))/(DeltaF);  //two dimensional velocity
		}
	}

	return v;
}

void Analysis::GetProfiles(string frameId, vector<polygon_2d> polygons, float * velocity)
{
	  //std::string buffer(frameId);
	  #ifdef WIN32
		  string Prfvelocity="Fundamental_Diagram\\Classical_Voronoi\\Profile\\velocity\\Prf_v_"+TrajectoryName+"_"+frameId+".dat";
		  string Prfdensity= "Fundamental_Diagram\\Classical_Voronoi\\Profile\\density\\Prf_d_"+TrajectoryName+"_"+frameId+".dat";
	  #else
		  string Prfvelocity="Fundamental_Diagram/Classical_Voronoi/field/velocity/Prf_v_"+TrajectoryName+"_"+frameId+".dat";
		  string Prfdensity="Fundamental_Diagram/Classical_Voronoi/field/density/Prf_d_"+TrajectoryName+"_"+frameId+".dat";
	  #endif
	  FILE *Prf_velocity;
	  if((Prf_velocity=fopen(Prfvelocity.c_str(),"w"))==NULL)
		{
		  printf("cannot open the file <%s> to write the field data\n",Prfvelocity.c_str());
		  exit(0);
		}
	  FILE *Prf_density;
	  if((Prf_density=fopen(Prfdensity.c_str(),"w"))==NULL)
		{
		  printf("cannot open the file to write the field density\n");
		  exit(0);
		}

	  int NRow = (int)ceil((high_ed_y-low_ed_y)/scale_y); // the number of rows that the geometry will be discretized for field analysis
	  int NColumn = (int)ceil((high_ed_x-low_ed_x)/scale_x); //the number of columns that the geometry will be discretized for field analysis
	  for(int row_i=0;row_i<NRow;row_i++) //
		{
		  for(int colum_j=0;colum_j<NColumn;colum_j++)
		{
		    polygon_2d measurezoneXY;
		    {
		        const double coor[][2] = {
		            {low_ed_x+colum_j*scale_x,low_ed_y+row_i*scale_y}, {low_ed_x+colum_j*scale_x+scale_x,low_ed_y+row_i*scale_y}, {low_ed_x+colum_j*scale_x+scale_x, low_ed_y+row_i*scale_y+scale_y},
		            {low_ed_x+colum_j*scale_x, low_ed_y+row_i*scale_y+scale_y},
		            {low_ed_x+colum_j*scale_x,low_ed_y+row_i*scale_y} // closing point is opening point
		            };
		        assign_points(measurezoneXY, coor);
		    }
		    correct(measurezoneXY);     // Polygons should be closed, and directed clockwise. If you're not sure if that is the case, call this function
		    float densityXY=getVoronoiDensity(polygons,measurezoneXY);
		    fprintf(Prf_density,"%.3f\t",densityXY);
		    float velocityXY = getVoronoiVelocity(polygons,velocity,measurezoneXY);
		  fprintf(Prf_velocity,"%.3f\t",velocityXY);
		}
		  fprintf(Prf_density,"\n");
		  fprintf(Prf_velocity,"\n");
		}
	  fclose(Prf_velocity);
	  fclose(Prf_density);
}

void Analysis::OutputVoroGraph(string frameId, vector<polygon_2d> polygons, int numPedsInFrame, float* XInFrame, float* YInFrame,float* VInFrame)
{
	  //std::string buffer(frameId);
	  #ifdef WIN32
	  string point="Fundamental_Diagram\\Classical_Voronoi\\VoronoiCell\\points"+TrajectoryName+"_"+frameId+".dat";
	  string polygon="Fundamental_Diagram\\Classical_Voronoi\\VoronoiCell\\polygon"+TrajectoryName+"_"+frameId+".dat";
	  string v_individual="Fundamental_Diagram\\Classical_Voronoi\\VoronoiCell\\speed"+TrajectoryName+"_"+frameId+".dat";
	  #else
	  string point="Fundamental_Diagram/Classical_Voronoi/VoronoiCell/points"+TrajectoryName+"_"+frameId+".dat";
	  string polygon="Fundamental_Diagram/Classical_Voronoi/VoronoiCell/polygon"+TrajectoryName+"_"+frameId+".dat";
	  string v_individual="Fundamental_Diagram/Classical_Voronoi/VoronoiCell/speed"+TrajectoryName+"_"+frameId+".dat";
	  #endif

	  ofstream points (point.c_str());
	  ofstream polys (polygon.c_str());
	  ofstream velo (v_individual.c_str());

	  for(vector<polygon_2d>::iterator polygon_iterator=polygons.begin(); polygon_iterator!=polygons.end();polygon_iterator++)
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
