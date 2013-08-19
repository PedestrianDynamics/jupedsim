

#include "Analysis.h"
#include "geometry/Room.h"
#include "general/xmlParser.h"

//#include "VoronoiDiagramGenerator.h"
//#include "VoronoiPolygons.h"

#include "VoronoiDiagram.h"

#include <iostream>
#include <fstream>
using namespace std;

/************************************************
 // Konstruktoren
 ************************************************/

Analysis::Analysis() {

  _building = NULL;
  _iod = new IODispatcher();
  _numFrames = 10;

  _tIn = NULL;
  _tOut = NULL;
  _lengthMeasurementarea = 200;  // the length of the measurement area
  _maxNumofPed =0;  //the maximum index of the pedestrian in the trajectory data
  _deltaF=5;											// half of the time interval that used to calculate instantaneous velocity of ped i.
  // here v_i = (X(t+deltaF) - X(t+deltaF))/(2*deltaF).   X is location.
  _xCor = NULL;
  _yCor = NULL;
  _firstFrame = NULL;   // Record the first frame of each pedestrian
  _lastFrame = NULL;	// Record the last frame of each pedestrian
  _deltaT =160;   // the time interval to calculate the classic flow
  _lineStartX = 0;  			//the coordinate of the line used to calculate the flow and velocity
  _lineStartY =10;
  _lineEndX = 0;
  _lineEndY =100;
  _flowVelocity = false; 						// Method A (Zhang2011a)
  _fundamentalTinTout = false; 			// Method B (Zhang2011a)
  _classicMethod = false; 					// Method C //calculate and save results of classic in separate file
  _voronoiMethod = false;  					// Method D--Voronoi method
  _cutByCircle = false;  //Adjust whether cut each original voronoi cell by a circle
  _getProfile = false;   // Whether make field analysis or not
  _outputGraph = false;   // Whether output the data for plot the fundamental diagram each frame
  _calcIndividualFD = false; //Adjust whether analyze the individual density and velocity of each pedestrian in stationary state (ALWAYS VORONOI-BASED)
  _vComponent = 'B'; // to mark whether x, y or x and y coordinate are used when calculating the velocity
  //AccumPedsPassLine = NULL; // the accumulative pedestrians pass a line with time
  //AccumVPassLine = NULL;  // the accumulative instantaneous velocity of the pedestrians pass a line
  _fps = 16;												// Frame rate of data
  _fClassicRhoV = NULL;
  _fVoronoiRhoV = NULL;
  _individualFD = NULL;
  _fN_t = NULL;

  _scaleX = 10;   // the size of the grid
  _scaleY = 10;
  _lowVertexX = 0;// LOWest vertex of the geometry (x coordinate)
  _lowVertexY = 0; //  LOWest vertex of the geometry (y coordinate)
  _highVertexX = 10; // Highest vertex of the geometry
  _highVertexY = 10;

}

Analysis::~Analysis() {
  delete _building;
  delete _iod;
  delete  _firstFrame;
  delete  _lastFrame;
  delete  _tIn;
  delete  _tOut;

  for (int i=0; i<_maxNumofPed; i++)
    {
      delete _xCor[i];
      delete _yCor[i];
    }
  delete []_xCor;
  delete []_yCor;
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

  _measureZone = args->GetMeasureArea();


  _flowVelocity = args->GetIsMethodA();
  _fundamentalTinTout = args->GetIsMethodB();
  _classicMethod = args ->GetIsMethodC();
  if(_fundamentalTinTout)
    {
      _classicMethod = true;
    }
  _voronoiMethod = args ->GetIsMethodD();
  _lengthMeasurementarea = args->GetLengthMeasurementArea();
  _deltaF = args->GetDelatT_Vins();
  _deltaT = args->GetTimeIntervalA();
  _lineStartX = args->GetLineStartX();
  _lineStartY = args->GetLineStartY();
  _lineEndX = args->GetLineEndX();
  _lineEndY = args->GetLineEndY();
  _cutByCircle = args->GetIsCutByCircle();
  _getProfile = args->GetIsGetProfile();
  _outputGraph = args->GetIsOutputGraph();
  _calcIndividualFD = args->GetIsIndividualFD();
  _vComponent = args->GetVComponent();
  _scaleX = args->GetScaleX();
  _scaleY = args->GetScaleY();

  // IMPORTANT: do not change the order in the following..
  sprintf(tmp, "\tGeometrie: [%s]\n", args->GetGeometryFilename().c_str());
  s.append(tmp);
  Log->Write("INFO: \t" + s);
  _geoPoly = ReadGeometry(args->GetGeometryFilename());
  //pBuilding->WriteToErrorLog();

  _trajectoryName = args->GetTrajectoryName();
  _trajectoryFile = args->GetTrajectoriesFile();
  std::cout<<_trajectoryFile.c_str();
  Log->Write(string("INFO: \t") + _trajectoryFile);
  ReadTrajetories(_trajectoryFile);

  if(_classicMethod)
    {
#ifdef WIN32
      string results_C=  "Output\\Fundamental_Diagram\\Classical_Voronoi\\rho_v_Classic_"+_trajectoryName+".dat";
#else
      string results_C=  "Output/Fundamental_Diagram/Classical_Voronoi/rho_v_Classic_"+_trajectoryName+".dat";
#endif
                         
      if((_fClassicRhoV=fopen(results_C.c_str(),"w"))==NULL)
	{
	  Log->Write("cannot open file %s to write classical density and velocity\n", results_C.c_str());
	  exit(0);
	}
      fprintf(_fClassicRhoV,"#Frame \tclassical density(m^(-2))\t	classical velocity(m/s)\n");
    }
  if(_voronoiMethod)
    {
#ifdef WIN32
      string results_V=  "Output\\Fundamental_Diagram\\Classical_Voronoi\\rho_v_Voronoi_"+_trajectoryName+".dat";
      //string results_V=  "rho_v_Voronoi_"+_trajectoryName+".dat";
#else
      string results_V=  "Output/Fundamental_Diagram/Classical_Voronoi/rho_v_Voronoi_"+_trajectoryName+".dat";
#endif
      if((_fVoronoiRhoV=fopen(results_V.c_str(),"w"))==NULL)
	{
	  Log->Write("cannot open the file to write Voronoi density and velocity\n");
	  exit(0);
	}
      fprintf(_fVoronoiRhoV,"#Frame \t Voronoi density(m^(-2))\t	Voronoi velocity(m/s)\n");
    }
  if(_calcIndividualFD)
    {
#ifdef WIN32
      string Individualfundment="Output\\Fundamental_Diagram\\Individual_FD\\IndividualFD"+_trajectoryName+".dat";
#else
      string Individualfundment="Output/Fundamental_Diagram/Individual_FD/IndividualFD"+_trajectoryName+".dat";
#endif
      if((_individualFD=fopen(Individualfundment.c_str(),"w"))==NULL)
	{
	  Log->Write("cannot open the file individual\n");
	  exit(0);
	}
      fprintf(_individualFD,"#Individual density(m^(-2))\t	Individual velocity(m/s)\n");
    }
  if(_flowVelocity)
    {

#ifdef WIN32
      string N_t= "Output\\Fundamental_Diagram\\FlowVelocity\\Flow_NT_"+_trajectoryName+"_Out.dat";
#else
      string N_t= "Output/Fundamental_Diagram/FlowVelocity/Flow_NT_"+_trajectoryName+"_Out.dat";
#endif
      if((_fN_t=fopen(N_t.c_str(),"w"))==NULL)
	{
	  Log->Write("cannot open the file %s  t\n", N_t.c_str() );
	  exit(0);
	}
      else 
	cout << "can open \n";
	
      fprintf(_fN_t,"#Frame\t	Cumulative pedestrians\n");
    }
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
      for (unsigned int j = 0; j< temp_GeoPoly.size(); j++){
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
    }
  }

  _highVertexX = geo_maxX;
  _highVertexY = geo_maxY;
  _lowVertexX = geo_minX;
  _lowVertexY = geo_minY;
  cout <<"_highVertexX: \t"<<_highVertexX<< endl;
  cout <<"_highVertexY: \t"<<_highVertexY<< endl;
  cout <<"_lowVertexX: \t"<<_lowVertexX<< endl;
  cout <<"_lowVertexY: \t"<<_lowVertexY<< endl;

  return geoPoly;
}

void Analysis::ReadTrajetories(const string& trajectoriesFile){
  //---------read the trajectory data from file-----------
  ////////////////////////////--------------Parse the xmlFile---------------///////////////////////////////////////////////////////

  Log->Write(">>> input file <%s>\n",trajectoriesFile.c_str());

  XMLNode xMainNode = XMLNode::openFileHelper(trajectoriesFile.c_str(),"trajectoriesDataset");
  XMLNode xHeader = xMainNode.getChildNode("header"); // header
  XMLNode xFrame = xMainNode.getChildNode("frame"); // frame
  _numFrames = xMainNode.nChildNode("frame"); // how much frames

  Log->Write("numFrames = %d\n",_numFrames);
  if(!xHeader.getChildNode("agents").isEmpty()){
    const char* N = xHeader.getChildNode("agents").getText();
    _maxNumofPed = atoi(N);
    Log->Write("N=%d\n", _maxNumofPed);
  }
  if(!xHeader.getChildNode("frameRate").isEmpty()){
    const char* NF = xHeader.getChildNode("frameRate").getText();
    _fps = atoi(NF);
    Log->Write("fps=%d\n", _fps);
  }
  _xCor = new double* [_maxNumofPed];
  _yCor = new double* [_maxNumofPed];
  for (int i=0; i<_maxNumofPed; i++)
    {
      _xCor[i] = new double [_numFrames];
      _yCor[i] = new double [_numFrames];
    }
  _firstFrame = new int[_maxNumofPed];  // Record the first frame of each pedestrian
  _lastFrame = new int[_maxNumofPed];  // Record the last frame of each pedestrian
  _tIn = new int[_maxNumofPed];				// Record the time of each pedestrian entering measurement area
  _tOut = new int[_maxNumofPed];				// Record the time of each pedestrian exiting measurement area
  bool IsinMeasurezone[_maxNumofPed];  // Record whether pedestrian i is in measurement area or not

  for(int i = 0; i <_maxNumofPed; i++)
    {
      for (int j = 0; j < _numFrames; j++)
	{
	  _xCor[i][j] = 0;
	  _yCor[i][j] = 0;
	}
      _firstFrame[i] = INT_MAX;
      _lastFrame[i] = INT_MIN;
      _tIn[i] = 0;
      _tOut[i] = 0;
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
		  _xCor[ID][f] = atof(x);
		  _yCor[ID][f] = atof(y);
		  if(f < _firstFrame[ID])
		    {
		      _firstFrame[ID] = f;
		    }
		  if(f > _lastFrame[ID])
		    {
		      _lastFrame[ID] = f;
		    }
		  if(_fundamentalTinTout==true)
		    {
		      if(within(make<point_2d>(atof(x),atof(y)), _measureZone)&&!(IsinMeasurezone[ID]))
			{
			  _tIn[ID]=f;
			  IsinMeasurezone[ID] = true;
			}
		      if((!within(make<point_2d>(atof(x),atof(y)), _measureZone))&&IsinMeasurezone[ID])
			{
			  _tOut[ID]=f;
			  IsinMeasurezone[ID] = false;
			}
		    }
		}
	    } //for i
	}// getFrame number j
    }
  Log->Write("finished test of xmlParser.\n");

}

int Analysis::RunAnalysis()
{
  XMLNode xMainNode = XMLNode::openFileHelper(_trajectoryFile.c_str(),"trajectoriesDataset");
  XMLNode xFrame = xMainNode.getChildNode("frame"); // frame
  int ClassicFlow=0; // the number of pedestrians pass a line in a certain time
  double V_deltaT=0;   // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N
  double *DensityPerFrame;
  DensityPerFrame = new double[_numFrames];
  for(int i=0;i<_numFrames;i++)
    {
      DensityPerFrame[i]=0;
    }
  bool *PassLine = new bool[_maxNumofPed];
  for(int i=0; i<_maxNumofPed; i++)
    {
      PassLine[i] = false;
    }

  string N_t="Flow_NT_"+_trajectoryName+"_Out.dat";
  ofstream flowNTs(N_t.c_str());

  if(!xFrame.isEmpty())
    {
      for (int f=0; f<_numFrames; f++) //read the data frame by frame
	{
	  xFrame = xMainNode.getChildNode("frame", f); // frame j
	  const char* frid = xFrame.getAttribute("ID");

	  Log->Write("frame ID = %d\n",atoi(frid));

	  int numPedsInFrame = xFrame.nChildNode("agent"); 		// how many agents in this frame
	  //printf("This frame has %d peds\n",numPedsInFrame);
	  double *XInFrame = new double[numPedsInFrame]; 				// save the X coordinates of pedestrian in the geometry in this frame
	  double *YInFrame = new double[numPedsInFrame];				// save the Y coordinates of pedestrian in the geometry in this frame
	  double *VInFrame = new double[numPedsInFrame]; 				// save the instantaneous velocity of pedestrians in the geometry in this frame

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
		  int Tpast = f - _deltaF;
		  int Tfuture = f + _deltaF;
		  VInFrame[i] = GetVinFrame(f, Tpast, Tfuture, ID, _firstFrame, _lastFrame, _xCor, _yCor, _vComponent);
		  bool IspassLine=false;
		  if(f >_firstFrame[ID]&&!PassLine[ID])
		    {
		      IspassLine = IsPassLine(_lineStartX,_lineStartY, _lineEndX, _lineEndY,_xCor[ID][f-1],_yCor[ID][f-1],_xCor[ID][f],_yCor[ID][f]);
		    }
		  if(IspassLine==true)
		    {
		      PassLine[ID] = true;
		      ClassicFlow++;
		      V_deltaT+=VInFrame[i];
		      //cout<<_xCor[ID][f-1]<< "\t"<<_yCor[ID][f-1]<< "\t"<<_xCor[ID][f]<< "\t"<<_yCor[ID][f]<< endl;
		    }
		}
	    } //for i

	  if(_flowVelocity)
	    {
	      _accumPedsPassLine.push_back(ClassicFlow);
	      _accumVPassLine.push_back(V_deltaT);
	      fprintf(_fN_t,"%d\t%d\n",atoi(frid), ClassicFlow);
	    }

	  if(_classicMethod)
	    {
	      double ClassicDensity = GetClassicalDensity(XInFrame, YInFrame, numPedsInFrame, _measureZone);
	      double ClassicVelocity = GetClassicalVelocity(XInFrame, YInFrame, VInFrame, numPedsInFrame, _measureZone);
	      DensityPerFrame[f]=ClassicDensity;
	      fprintf(_fClassicRhoV,"%d\t%.3f\t%.3f\n",atoi(frid), ClassicDensity,ClassicVelocity);
	    }

	  //------------------Voronoi Method---------------------------------
	  if(_voronoiMethod)
	    {

	      vector<polygon_2d> polygons;
	      VoronoiDiagram vd;
	      if(numPedsInFrame>2)
		{
		  polygons = vd.getVoronoiPolygons(XInFrame, YInFrame, VInFrame, numPedsInFrame);
		  if(_cutByCircle)
		    {
		      //polygons = cutPolygonsWithCircle(polygons, XInFrame, YInFrame, 50);
		    }
		  polygons = vd.cutPolygonsWithGeometry(polygons, _geoPoly, XInFrame, YInFrame);
		  double VoronoiVelocity;
		  if(numPedsInFrame>0)
		    {
		      VoronoiVelocity=GetVoronoiVelocity(polygons,VInFrame,_measureZone);
		    }
		  else
		    {
		      VoronoiVelocity=0;
		    }
		  double VoronoiDensity=GetVoronoiDensity(polygons, _measureZone);
		  fprintf(_fVoronoiRhoV,"%d\t%.3f\t%.3f\n",atoi(frid),VoronoiDensity, VoronoiVelocity);

		  if(_calcIndividualFD)
		    {
		      if(numPedsInFrame>0)
			{
			  // if(i>beginstationary&&i<endstationary)
			  {
			    GetIndividualFD(polygons,VInFrame,_measureZone);
			  }
			}
		    }
		  //------------------field analysis----------------------------------------------------------
		  if(_getProfile)
		    {
		      if(numPedsInFrame>0)
			{
			  GetProfiles(string(frid), polygons, VInFrame);
			}
		    }
		  //------------the following codes is written to output the Voronoi polygons of a frame-----------
		  if(_outputGraph)
		    {
		      cout<<"output polygons"<<endl;
		      OutputVoroGraph(string(frid), polygons, numPedsInFrame,XInFrame, YInFrame,VInFrame);
		    }
		}
	      else
		{
		  cout<<" the number of the pedestrians is less than 2 !!"<< endl;
		}
	    }
	
	  delete []XInFrame;
	
	  delete []YInFrame;
	
	  delete []VInFrame;
	
	}// getFrame number j
    }
	
  if(_classicMethod)
    fclose(_fClassicRhoV);
	
  if(_voronoiMethod)
    fclose(_fVoronoiRhoV);
	
  if(_flowVelocity)
    fclose(_fN_t);//
	
  delete []PassLine;
	
  //--------------------Fundamental diagram based on Tin and Tout----------------------------------------------------------------------
  if(_fundamentalTinTout)
    {
#ifdef WIN32
      string FD_TinTout=  "Output\\Fundamental_Diagram\\TinTout\\FDTinTout_"+_trajectoryName+".dat";
#else
      string FD_TinTout=  "Output/Fundamental_Diagram/TinTout/FDTinTout_"+_trajectoryName+".dat";
#endif
      Log->Write("Fundamental diagram based on Tin and Tout will be calculated!");
      GetFundamentalTinTout(_tIn,_tOut,DensityPerFrame, _fps, _lengthMeasurementarea,_maxNumofPed, FD_TinTout); //MC. 15.8.12. replaced "datafile" by results
    }
  //-----------------------------------------------------------------------------------------------------------------------------------
  if(_flowVelocity)
    {
#ifdef WIN32
      string FD_FlowVelocity=  "Output\\Fundamental_Diagram\\FlowVelocity\\FDFlowVelocity_"+_trajectoryName+".dat";
#else
      string FD_FlowVelocity=  "Output/Fundamental_Diagram/FlowVelocity/FDFlowVelocity_"+_trajectoryName+".dat";
#endif
      FlowRate_Velocity(_deltaT,_fps, _accumPedsPassLine,_accumVPassLine,FD_FlowVelocity);
    }
  delete []DensityPerFrame;
  return 0;
}

/*
 *  according to the location of a pedestrian in adjacent frame (pt1_X,pt1_Y) and (pr2_X,pt2_Y), we
 *  adjust whether he pass the line from Line_start to Line_end
 */
bool Analysis::IsPassLine(double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y)
{
  double x1=Line_startX;
  double y1=Line_startY;
  double x2=Line_endX;
  double y2=Line_endY;
  double x3=pt1_X;
  double y3=pt1_Y;
  double x4=pt2_X;
  double y4=pt2_Y;

  double d=(y2-y1)*(x4-x3)-(y4-y3)*(x2-x1);
  if(d==0)
    {
      return false;
    }
  else
    {
      double x0=((x2-x1)*(x4-x3)*(y3-y1)+(y2-y1)*(x4-x3)*x1-(y4-y3)*(x2-x1)*x3)/d;
      double y0=((y2-y1)*(y4-y3)*(x3-x1)+(x2-x1)*(y4-y3)*y1-(x4-x3)*(y2-y1)*y3)/(-1.0*d);
      double temp1=(x0-x1)*(x0-x2);
      double temp2=(x0-x3)*(x0-x4);
      double temp3=(y0-y1)*(y0-y2);
      double temp4=(y0-y3)*(y0-y4);
      if(temp1<10.0E-16)
	{
	  temp1=0.0;
	}
      if(temp2<10.0E-16)
	{
	  temp2=0.0;
	}
      if(temp3<10.0E-16)
	{
	  temp3=0.0;
	}
      if(temp4<10.0E-16)
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
void Analysis::GetFundamentalTinTout(int *Tin, int *Tout, double *DensityPerFrame, int fps, double LengthMeasurementarea,int Nped, string ofile)
{
  FILE *fFD_TinTout;
  string fdTinTout=ofile;
  if((fFD_TinTout=fopen(fdTinTout.c_str(),"w"))==NULL)
    {
      Log->Write("cannot open the file to write the TinTout data\n");
      exit(0);
    }
  fprintf(fFD_TinTout,"#person Index\t	density_i(m^(-2))\t	velocity_i(m/s)\n");
  for(int i=0;i<Nped;i++)
    {
      double velocity_temp=fps*CMtoM*LengthMeasurementarea/(Tout[i]-Tin[i]);
      double density_temp=0;
      for(int j=Tin[i];j<Tout[i];j++)
	{
	  density_temp+=DensityPerFrame[j];
	}
      density_temp/=(Tout[i]-Tin[i]);
      fprintf(fFD_TinTout,"%d\t%f\t%f\n",i+1,density_temp,velocity_temp);
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
void Analysis::FlowRate_Velocity(int DeltaT, int fps, vector<int> AccumPeds, vector<double> AccumVelocity, string ofile)
{

  FILE *fFD_FlowVelocity;
  string fdFlowVelocity = ofile;  //"Fundamental Diagram\\FlowVelocity\\FundamentalFlowVelocity"+datafile+".dat";
  if((fFD_FlowVelocity=fopen(fdFlowVelocity.c_str(),"w"))==NULL)
    {
      Log->Write("cannot open the file to write the Flow-Velocity data\n");
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
      double flow_rate=fps*(AccumPeds[i]-AccumPeds[i-DeltaT])*1.00/(pedspassT[AccumPeds[i]]-pedspassT[AccumPeds[i-DeltaT]]);
      double MeanV=(AccumVelocity[i]-AccumVelocity[i-DeltaT])/(AccumPeds[i]-AccumPeds[i-DeltaT]);
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
void Analysis::GetIndividualFD(vector<polygon_2d> polygon, double* Velocity, polygon_2d measureArea)
{

  vector<polygon_2d>::iterator polygon_iterator;
  double uniquedensity=0;
  double uniquevelocity=0;
  int temp=0;
  for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
    {
      typedef std::vector<polygon_2d> polygon_list;
      polygon_list v;
      intersection(measureArea, *polygon_iterator, v);
      if(!v.empty())
	{
	  uniquedensity=1/(area(*polygon_iterator)*CMtoM*CMtoM);
	  uniquevelocity=Velocity[temp];
	  fprintf(_individualFD,"%.3f\t%.3f\n",uniquedensity,uniquevelocity);
	}
      temp++;
    }
}

double Analysis::Distance(double x1, double y1, double x2, double y2)
{
  double distance=sqrt(pow((x1-x2),2)+pow((y1-y2),2));
  return distance;
}
///---------------------------------------------------------------------------------------------------------------------
/*this function is to obtain the frequency distribution of the pedestrian movement through a line from (Line_startX,Line_startY)
 * to (Line_endX, Line_endY) according to the coordination of a person in two adjacent frames (pt1_X,pt1_Y) and (pt2_X,pt2_Y)
 * input: a empty array that will be used to save the results. "fraction" is the number of same parts that the line will be divided into.
 * output: the number of persons in each part of the line "frequency".
 */
void Analysis::DistributionOnLine(int *frequency,int fraction, double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y)
{
  /* in this function, we firstly calculate the intersection point between the trajectory (the line from point (pt1_X,pt1_Y)
   *  to (pt2_X,pt2_Y)) of a pedestrian and the Line. then, we determine which part of the line he or she belongs to
   */
  double sumdistance=Distance(Line_startX,Line_startY,Line_endX, Line_endY);
  double A=Line_endY-Line_startY;
  double B=Line_startX-Line_endX;
  double C=Line_endX*Line_startY-Line_startX*Line_endY;
  double d1=A*pt1_X+B*pt1_Y+C;
  double d2=A*pt2_X+B*pt2_Y+C;
  if(d1*d2<0)
    {
      double x1=Line_startX;
      double y1=Line_startY;
      double x2=Line_endX;
      double y2=Line_endY;
      double x3=pt1_X;
      double y3=pt1_Y;
      double x4=pt2_X;
      double y4=pt2_Y;
      double x =((x1 - x2) * (x3 * y4 - x4 * y3) - (x3 - x4) * (x1 * y2 - x2 * y1))
	/((x3 - x4) * (y1 - y2) - (x1 - x2) * (y3 - y4));

      double y =((y1 - y2) * (x3 * y4 - x4 * y3) - (x1 * y2 - x2 * y1) * (y3 - y4))
	/((y1 - y2) * (x3 - x4) - (x1 - x2) * (y3 - y4));
      int index=(int)floor(Distance(x,y,x1,y1)*fraction/sumdistance);
      frequency[index]++;
    }
  else if(d1*d2==0)
    {
      if(d1==0)
	{
	  int index=(int)floor(Distance(Line_startX,Line_startY,pt1_X,pt1_Y)*fraction/sumdistance);
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
double Analysis::GetVoronoiDensity(vector<polygon_2d> polygon, polygon_2d measureArea)
{
  vector<polygon_2d>::iterator polygon_iterator;
  double density=0;
  for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
    {
      typedef std::vector<polygon_2d > polygon_list;
      polygon_list v;
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
  density=density/(area(measureArea)*CMtoM*CMtoM);
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
double Analysis::GetClassicalDensity(double *xs, double *ys, int pednum, polygon_2d measureArea)
{
  int pedsinMeasureArea=0;
  for(int i=0;i<pednum;i++)
    {
      if(within(make<point_2d>(xs[i], ys[i]), measureArea))
	{
	  pedsinMeasureArea++;
	}
    }

  double density=pedsinMeasureArea/(area(measureArea)*CMtoM*CMtoM);
  return density;
}

double Analysis::GetClassicalVelocity(double *xs, double *ys, double *VInFrame, int pednum, polygon_2d measureArea){

  int pedsinMeasureArea=0;
  double velocity = 0;
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
double Analysis::GetVoronoiVelocity(vector<polygon_2d> polygon, double* Velocity, polygon_2d measureArea)
{
  vector<polygon_2d>::iterator polygon_iterator;
  double meanV=0;
  int temp=0;

  for(polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
    {
      typedef std::vector<polygon_2d > polygon_list;

      polygon_list v;
      //intersection_inserter<polygon_2d>(measureArea, *polygon_iterator, std::back_inserter(v));
      intersection(measureArea, *polygon_iterator, v);
      if(!v.empty())
	{
	  meanV+=(Velocity[temp]*area(v[0])/area(measureArea));
	  //std::cout<<"the velocity and areas:"<<Velocity[temp]<<'\t'<<area(v[0])<<'\t'<<meanV<<'\t'<<temp<<'\n';
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

//double Analysis::GetVinFrame(int Tnow,int Tpast, int Tfuture, int Tfirst, int Tlast, double Xcor_past, double Xcor_now, double Xcor_future,double Ycor_past, double Ycor_now, double Ycor_future, char VComponent){
double Analysis::GetVinFrame(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor, char VComponent){

  double v=0;
  //VInFrame[i] = GetVinFrame(f, Tpast, Tfuture, _firstFrame[ID], _lastFrame[ID], _xCor[ID][Tpast], _xCor[ID][f], _xCor[ID][Tfuture], _yCor[ID][Tpast], _yCor[ID][f], _yCor[ID][Tfuture], _vComponent);

  if(VComponent == 'X')
    {
      if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID]))
	{
	  v = _fps*CMtoM*(Xcor[ID][Tfuture] - Xcor[ID][Tpast])/(2.0 * _deltaF);  //one dimensional velocity
	}
      else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID]))
	{
	  v = _fps*CMtoM*(Xcor[ID][Tfuture] - Xcor[ID][Tnow])/(_deltaF);  //one dimensional velocity
	}
      else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID]))
	{
	  v = _fps*CMtoM*(Xcor[ID][Tnow] - Xcor[ID][Tpast])/( _deltaF);  //one dimensional velocity
	}
    }
  if(VComponent == 'Y')
    {
      if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID]))
	{
	  v = _fps*CMtoM*(Ycor[ID][Tfuture] - Ycor[ID][Tpast])/(2.0 * _deltaF);  //one dimensional velocity
	}
      else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID]))
	{
	  v = _fps*CMtoM*(Ycor[ID][Tfuture] - Ycor[ID][Tnow])/(_deltaF);  //one dimensional velocity
	}
      else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID]))
	{
	  v = _fps*CMtoM*(Ycor[ID][Tnow] - Ycor[ID][Tpast])/( _deltaF);  //one dimensional velocity
	}
    }
  if(VComponent == 'B')
    {
      if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID]))
	{
	  v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][Tpast]),2))/(2.0 * _deltaF);  //two dimensional velocity
	}
      else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID]))
	{
	  v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][Tnow]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][Tnow]),2))/(_deltaF);
	}
      else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID]))
	{
	  v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tnow] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][Tnow] - Ycor[ID][Tpast]),2))/(_deltaF);  //two dimensional velocity
	}
    }

  v = fabs(v);
  return v;
}

void Analysis::GetProfiles(string frameId, vector<polygon_2d> polygons, double * velocity)
{
  //std::string buffer(frameId);
#ifdef WIN32
  string Prfvelocity="Fundamental_Diagram\\Classical_Voronoi\\Profile\\velocity\\Prf_v_"+_trajectoryName+"_"+frameId+".dat";
  string Prfdensity= "Fundamental_Diagram\\Classical_Voronoi\\Profile\\density\\Prf_d_"+_trajectoryName+"_"+frameId+".dat";
#else
  string Prfvelocity="Fundamental_Diagram/Classical_Voronoi/field/velocity/Prf_v_"+_trajectoryName+"_"+frameId+".dat";
  string Prfdensity="Fundamental_Diagram/Classical_Voronoi/field/density/Prf_d_"+_trajectoryName+"_"+frameId+".dat";
#endif
  FILE *Prf_velocity;
  if((Prf_velocity=fopen(Prfvelocity.c_str(),"w"))==NULL)
    {
      Log->Write("cannot open the file <%s> to write the field data\n",Prfvelocity.c_str());
      exit(0);
    }
  FILE *Prf_density;
  if((Prf_density=fopen(Prfdensity.c_str(),"w"))==NULL)
    {
      Log->Write("cannot open the file to write the field density\n");
      exit(0);
    }

  int NRow = (int)ceil((_highVertexY-_lowVertexY)/_scaleY); // the number of rows that the geometry will be discretized for field analysis
  int NColumn = (int)ceil((_highVertexX-_lowVertexX)/_scaleX); //the number of columns that the geometry will be discretized for field analysis
  for(int row_i=0;row_i<NRow;row_i++) //
    {
      for(int colum_j=0;colum_j<NColumn;colum_j++)
	{
	  polygon_2d measurezoneXY;
	  {
	    const double coor[][2] = {
	      {_lowVertexX+colum_j*_scaleX,_lowVertexY+row_i*_scaleY}, {_lowVertexX+colum_j*_scaleX+_scaleX,_lowVertexY+row_i*_scaleY}, {_lowVertexX+colum_j*_scaleX+_scaleX, _lowVertexY+row_i*_scaleY+_scaleY},
	      {_lowVertexX+colum_j*_scaleX, _lowVertexY+row_i*_scaleY+_scaleY},
	      {_lowVertexX+colum_j*_scaleX,_lowVertexY+row_i*_scaleY} // closing point is opening point
	    };
	    assign_points(measurezoneXY, coor);
	  }
	  correct(measurezoneXY);     // Polygons should be closed, and directed clockwise. If you're not sure if that is the case, call this function
	  double densityXY=GetVoronoiDensity(polygons,measurezoneXY);
	  fprintf(Prf_density,"%.3f\t",densityXY);
	  double velocityXY = GetVoronoiVelocity(polygons,velocity,measurezoneXY);
	  fprintf(Prf_velocity,"%.3f\t",velocityXY);
	}
      fprintf(Prf_density,"\n");
      fprintf(Prf_velocity,"\n");
    }
  fclose(Prf_velocity);
  fclose(Prf_density);
}

void Analysis::OutputVoroGraph(string frameId, vector<polygon_2d> polygons, int numPedsInFrame, double* XInFrame, double* YInFrame,double* VInFrame)
{
#ifdef WIN32
  string point="Output\\Fundamental_Diagram\\Classical_Voronoi\\VoronoiCell\\points"+_trajectoryName+"_"+frameId+".dat";
  string polygon="Output\\Fundamental_Diagram\\Classical_Voronoi\\VoronoiCell\\polygon"+_trajectoryName+"_"+frameId+".dat";
  //string point="points"+_trajectoryName+"_"+frameId+".dat";
  //string polygon="polygon"+_trajectoryName+"_"+frameId+".dat";
  string v_individual="Output\\Fundamental_Diagram\\Classical_Voronoi\\VoronoiCell\\speed"+_trajectoryName+"_"+frameId+".dat";
#else
  string point="Fundamental_Diagram/Classical_Voronoi/VoronoiCell/points"+_trajectoryName+"_"+frameId+".dat";
  string polygon="Fundamental_Diagram/Classical_Voronoi/VoronoiCell/polygon"+_trajectoryName+"_"+frameId+".dat";
  string v_individual="Fundamental_Diagram/Classical_Voronoi/VoronoiCell/speed"+_trajectoryName+"_"+frameId+".dat";
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
