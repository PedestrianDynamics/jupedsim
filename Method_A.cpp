/*
 * Method_A.cpp
 *
 *  Created on: Jan 5, 2015
 *      Author: piccolo
 */

#include <Method_A.h>
#include <iostream>

#ifdef __linux__
#include <sys/stat.h>
#include <dirent.h>
#elif   __APPLE__
#include <sys/stat.h>
#include <dirent.h>
#else
#include <direct.h>
#endif

Method_A::Method_A()
{

}

Method_A::~Method_A()
{
    delete  [] _firstFrame;
    delete  [] _lastFrame;

    for (int i=0; i<_maxNumofPed; i++) {
         delete [] _xCor[i];
         delete [] _yCor[i];
    }
    delete [] _xCor;
    delete [] _yCor;
}

bool Method_A::Process (const PedData& peddata)
{
     peddata.GetMinID();

     return true;
}

bool Method_A::Process(const string& projectRootDir, const string& trajectoryfile, const FileFormat& _trajFormat,const string& outputfile, const MeasurementArea_L& area, int deltaF, char vComponent, int deltaT)
{

	_deltaF = deltaF;
	_vComponent = vComponent;
	_areaForMethod_A = area;
	_projectRootDir = projectRootDir;
	LoadTrajectory(trajectoryfile, _trajFormat);

	string N_t= _projectRootDir+"./Output/Fundamental_Diagram/FlowVelocity/Flow_NT_"+trajectoryfile+"_Out.dat";
	FILE *_fN_t;
	if((_fN_t=CreateFile(N_t))==NULL)
	{
		Log->Write("cannot open the file %s  t\n", N_t.c_str() );
		exit(EXIT_FAILURE);
	}
	fprintf(_fN_t,"#Frame\t	Cumulative pedestrians\n");
	for(int frameNr = 0; frameNr < _numFrames; frameNr++ )
	{
		int frid =  frameNr + min_Frame;
        std::vector<int> ids=peds_t[frameNr];
        int numPedsInFrame = ids.size();
        GetPedsParametersInFrame(frameNr, peds_t);
		_accumPedsPassLine.push_back(ClassicFlow);
		_accumVPassLine.push_back(V_deltaT);
		fprintf(_fN_t,"%d\t%d\n",frid, ClassicFlow);
	}

	string FD_FlowVelocity=  _projectRootDir+"./Output/Fundamental_Diagram/FlowVelocity/FDFlowVelocity_"+trajectoryfile+".dat";
	FlowRate_Velocity(deltaT,_fps, _accumPedsPassLine,_accumVPassLine,FD_FlowVelocity);

	delete [] PassLine;

    return true;

}

void Method_A::CreateGlobalVariables(int numPeds, int numFrames)
{
     _xCor = new double* [numPeds];
     _yCor = new double* [numPeds];
     for (int i=0; i<numPeds; i++) {
          _xCor[i] = new double [numFrames];
          _yCor[i] = new double [numFrames];
     }
     _firstFrame = new int[numPeds];  // Record the first frame of each pedestrian
     _lastFrame = new int[numPeds];  // Record the last frame of each pedestrian

     for(int i = 0; i <numPeds; i++) {
          for (int j = 0; j < numFrames; j++) {
               _xCor[i][j] = 0;
               _yCor[i][j] = 0;
          }
          _firstFrame[i] = INT_MAX;
          _lastFrame[i] = INT_MIN;
     }

     PassLine = new bool[numPeds];
     for(int i=0; i<numPeds; i++) {
          PassLine[i] = false;
     }
}

void Method_A::InitializeVariables(const string& filename)
{
     vector<double> xs;
     vector<double> ys;
     ifstream  fdata;
     fdata.open(filename.c_str());
     if (fdata.is_open() == false)
     {
          Log->Write("ERROR: \t could not parse the trajectories file <%s>",filename.c_str());
          exit(EXIT_FAILURE);
     }
     else
     {
          string line;
          int lineNr=1;
          while ( getline(fdata,line) )
          {
               //looking for the framerate which is suppposed to be at the second position
               if(line[0] == '#')
               {
                    std::vector<std::string> strs;
                    boost::split(strs, line , boost::is_any_of(":"),boost::token_compress_on);

                    if(strs[0]=="#framerate" && strs.size()==2)
                    {
                         _fps= atof(strs[1].c_str());
                         Log->Write("INFO:\tFrame rate fps=%d", _fps);
                    }

               }
               else if ( line[0] != '#' && !(line.empty()) )
               {

                    std::vector<std::string> strs;
                    boost::split(strs, line , boost::is_any_of("\t "),boost::token_compress_on);

                    if(strs.size() <4)
                    {
                         Log->Write("ERROR:\t There is an error in the file at line %d", lineNr);
                         exit(EXIT_FAILURE);
                    }

                    _IdsTXT.push_back(atoi(strs[0].c_str()));
                    _FramesTXT.push_back(atoi(strs[1].c_str()));

                    xs.push_back(atof(strs[2].c_str()));
                    ys.push_back(atof(strs[3].c_str()));
               }
               lineNr++;
          }
     }
     fdata.close();

     min_ID = *min_element(_IdsTXT.begin(),_IdsTXT.end());
     min_Frame = *min_element(_FramesTXT.begin(),_FramesTXT.end());

     //Total number of frames
     _numFrames = *max_element(_FramesTXT.begin(),_FramesTXT.end()) - min_Frame+1;

     //Total number of agents
     _maxNumofPed = *max_element(_IdsTXT.begin(),_IdsTXT.end()) - min_ID+1;
     CreateGlobalVariables(_maxNumofPed, _numFrames);

     std::vector<int> firstFrameIndex;  //The first frame index of each pedestrian
     std::vector<int> lastFrameIndex;	 //The last frame index of each pedestrian
     int prevValue = _IdsTXT[0] - 1;
     for (size_t i = 0; i < _IdsTXT.size(); i++)
     {
          if (prevValue != _IdsTXT[i])
          {
               firstFrameIndex.push_back(i);
               prevValue = _IdsTXT[i];
          }
     }
     for (size_t  i = 1; i < firstFrameIndex.size(); i++)
     {
          lastFrameIndex.push_back(firstFrameIndex[i] - 1);
     }
     lastFrameIndex.push_back(_IdsTXT.size() - 1);
     for (unsigned int i = 0; i < firstFrameIndex.size(); i++)
     {
          _firstFrame[i] = _FramesTXT[firstFrameIndex[i]] - min_Frame;
          _lastFrame[i] = _FramesTXT[lastFrameIndex[i]] - min_Frame;
     }
     for(unsigned int i = 0; i < _IdsTXT.size(); i++)
     {
          int ID = _IdsTXT[i] - min_ID;
          int frm = _FramesTXT[i] - min_Frame;
          double x = xs[i]*M2CM;
          double y = ys[i]*M2CM;
          _xCor[ID][frm] = x;
          _yCor[ID][frm] = y;
     }

     //save the data for each frame
     for (unsigned int i = 0; i < _FramesTXT.size(); i++ )
     {
          int id = _IdsTXT[i]-min_ID;
          int t =_FramesTXT[i]-min_Frame;
          peds_t[t].push_back(id);
     }

}

// initialize the global variables variables
void Method_A::InitializeVariables(TiXmlElement* xRootNode)
{
     if( ! xRootNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          exit(EXIT_FAILURE);
     }
     if( xRootNode->ValueStr () != "trajectoriesDataset" ) {
          Log->Write("ERROR:\tRoot element value is not 'geometry'.");
          exit(EXIT_FAILURE);
     }

     //counting the number of frames
     int frames = 0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame")) {
          frames++;
     }
     _numFrames = frames;
     Log->Write("INFO:\tnum Frames = %d",_numFrames);

     //Number of agents

     TiXmlNode*  xHeader = xRootNode->FirstChild("header"); // header
     if(xHeader->FirstChild("agents")) {
          _maxNumofPed=atoi(xHeader->FirstChild("agents")->FirstChild()->Value());
          Log->Write("INFO:\tmax num of peds N=%d", _maxNumofPed);
     }

     //framerate
     if(xHeader->FirstChild("frameRate")) {
          _fps=atoi(xHeader->FirstChild("frameRate")->FirstChild()->Value());
          Log->Write("INFO:\tFrame rate fps=%d", _fps);
     }

     CreateGlobalVariables(_maxNumofPed, _numFrames);
     //processing the frames node
     TiXmlNode*  xFramesNode = xRootNode->FirstChild("frame");
     if (!xFramesNode) {
          Log->Write("ERROR: \tThe geometry should have at least one frame");
          exit(EXIT_FAILURE);
     }

     // obtaining the minimum id and minimum frame
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame"))
     {
          int frm = atoi(xFrame->Attribute("ID"));
          if(frm < min_Frame)
          {
               min_Frame = frm;
          }
          for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
                    xAgent = xAgent->NextSiblingElement("agent"))
          {
               int id= atoi(xAgent->Attribute("ID"));
               if(id < min_ID)
               {
                    min_ID = id;
               }
          }
     }
     int frameNr=0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame")) {

          //todo: can be parallelized with OpenMP
          for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
                    xAgent = xAgent->NextSiblingElement("agent")) {

               //get agent id, x, y
               double x= atof(xAgent->Attribute("xPos"));
               double y= atof(xAgent->Attribute("yPos"));
               int ID= atoi(xAgent->Attribute("ID"))-min_ID;

               peds_t[frameNr].push_back(ID);
               _xCor[ID][frameNr] =  x*M2CM;
               _yCor[ID][frameNr] =  y*M2CM;
               if(frameNr < _firstFrame[ID]) {
                    _firstFrame[ID] = frameNr;
               }
               if(frameNr > _lastFrame[ID]) {
                    _lastFrame[ID] = frameNr;
               }
          }
          frameNr++;
     }

}

void Method_A::LoadTrajectory(const string& trajectoryfile, const FileFormat& _trajFormat)
{
    Log->Write("INFO:\tthe format of the trajectory is ",_trajFormat);
    if(_trajFormat == FORMAT_XML_PLAIN) // read traje
    {
         TiXmlDocument docGeo(trajectoryfile);
         if (!docGeo.LoadFile()) {
              Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
              Log->Write("ERROR: \t could not parse the trajectories file <%s>",trajectoryfile.c_str());
              exit(EXIT_FAILURE);
         }
         TiXmlElement* xRootNode = docGeo.RootElement();
         InitializeVariables(xRootNode);	//initialize some global variables
    }

    else if(_trajFormat == FORMAT_PLAIN)
    {
         InitializeVariables(trajectoryfile);
    }
}

void Method_A::GetPedsParametersInFrame(int frame, std::map< int, std::vector<int> > &pdt)
{
     const std::vector<int>& ids=pdt[frame];

     int PedNum = ids.size();
     IdInFrame = new int[PedNum];
     XInFrame = new double[PedNum];
     YInFrame = new double[PedNum];
     VInFrame = new double[PedNum];

     for(int i=0; i<PedNum;i++)
     {
          int id = ids[i];
          XInFrame[i] = _xCor[id][frame];
          YInFrame[i] = _yCor[id][frame];
          int Tpast = frame - _deltaF;
          int Tfuture = frame + _deltaF;
          VInFrame[i] = GetVinFrame(frame, Tpast, Tfuture, id, _firstFrame, _lastFrame, _xCor, _yCor, _vComponent);
          IdInFrame[i] = id+min_ID;

          bool IspassLine=false;
          if(frame >_firstFrame[id]&&!PassLine[id])
          {
        	  IspassLine = IsPassLine(_areaForMethod_A._lineStartX,
        			  _areaForMethod_A._lineStartY,
					  _areaForMethod_A._lineEndX,
					  _areaForMethod_A._lineEndY, _xCor[id][frame - 1],
					  _yCor[id][frame - 1], _xCor[id][frame],
					  _yCor[id][frame]);
          }
          if(IspassLine==true)
          {
        	  PassLine[id] = true;
        	  ClassicFlow++;
        	  V_deltaT+=VInFrame[i];
          }
     }
}

bool Method_A::IsPassLine(double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y)
{
     point_2d Line_pt0(Line_startX, Line_startY);
     point_2d Line_pt1(Line_endX, Line_endY);
     segment edge0(Line_pt0, Line_pt1);

     point_2d Traj_pt0(pt1_X, pt1_Y);
     point_2d Traj_pt1(pt2_X, pt2_Y);
     segment edge1(Traj_pt0, Traj_pt1);

     return(intersects(edge0, edge1));
}

double Method_A::GetVinFrame(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor, char VComponent)
{

     double v=0.0;

     if(VComponent == 'X') {
          if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Xcor[ID][Tfuture] - Xcor[ID][Tpast])/(2.0 * _deltaF);  //one dimensional velocity
          } else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Xcor[ID][Tfuture] - Xcor[ID][Tnow])/(_deltaF);  //one dimensional velocity
          } else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID])) {
               v = _fps*CMtoM*(Xcor[ID][Tnow] - Xcor[ID][Tpast])/( _deltaF);  //one dimensional velocity
          }
     }
     if(VComponent == 'Y') {
          if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Ycor[ID][Tfuture] - Ycor[ID][Tpast])/(2.0 * _deltaF);  //one dimensional velocity
          } else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*(Ycor[ID][Tfuture] - Ycor[ID][Tnow])/(_deltaF);  //one dimensional velocity
          } else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID])) {
               v = _fps*CMtoM*(Ycor[ID][Tnow] - Ycor[ID][Tpast])/( _deltaF);  //one dimensional velocity
          }
     }
     if(VComponent == 'B') {
          if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][Tpast]),2))/(2.0 * _deltaF);  //two dimensional velocity
          } else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID])) {
               v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tfuture] - Xcor[ID][Tnow]),2)+pow((Ycor[ID][Tfuture] - Ycor[ID][Tnow]),2))/(_deltaF);
          } else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID])) {
               v = _fps*CMtoM*sqrt(pow((Xcor[ID][Tnow] - Xcor[ID][Tpast]),2)+pow((Ycor[ID][Tnow] - Ycor[ID][Tpast]),2))/(_deltaF);  //two dimensional velocity
          }
     }

     return fabs(v);
}

void Method_A::FlowRate_Velocity(int DeltaT, int fps, const vector<int>& AccumPeds, const vector<double>& AccumVelocity, const string& ofile)
{

     FILE *fFD_FlowVelocity;
     string fdFlowVelocity = ofile;
     if((fFD_FlowVelocity=CreateFile(fdFlowVelocity))==NULL) {
          Log->Write("cannot open the file to write the Flow-Velocity data\n");
          exit(0);
     }
     fprintf(fFD_FlowVelocity,"#Flow rate(1/s)	Mean velocity(m/s)\n");
     int TotalTime=AccumPeds.size();  // the total Frame of in the data file
     int TotalPeds=AccumPeds[TotalTime-1];  //the total pedestrians in the data file
     if(TotalPeds>0)
     {
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
		 for(int i=firstPassT+DeltaT; i<TotalTime; i+=DeltaT) {
			 int N1 = AccumPeds[i-DeltaT];  // the total number of pedestrians pass the line at this time
			 int N2 = AccumPeds[i];
			 int t_N1 = pedspassT[N1];
			 int t_N2 = pedspassT[N2];
			 if(N1!=N2)
			 {
				 double flow_rate=fps*(N2-N1)*1.00/(t_N2-t_N1);
				 double MeanV=(AccumVelocity[i]-AccumVelocity[i-DeltaT])/(AccumPeds[i]-AccumPeds[i-DeltaT]);
				 fprintf(fFD_FlowVelocity,"%.3f\t%.3f\n",flow_rate,MeanV);
			 }
		 }
		 fclose(fFD_FlowVelocity);
		 delete []pedspassT;
     }
     else
     {
    	 Log->Write("INFO:\tNo person passing the reference line given by Method A!\n");
     }
}

FILE* Method_A::CreateFile(const string& filename)
{
     //first try to create the file
     FILE* fHandle= fopen(filename.c_str(),"w");
     if(fHandle) return fHandle;

     unsigned int found=filename.find_last_of("/\\");
     string dir = filename.substr(0,found)+"/";
     //string file= filename.substr(found+1);

     // the directories are probably missing, create it
     if (mkpath((char*)dir.c_str())==-1) {
          Log->Write("ERROR:\tcannot create the directory <%s>",dir.c_str());
          return NULL;
     }
     //second and last attempt
     return fopen(filename.c_str(),"w");
}

#if defined(_WIN32)

int Method_A::mkpath(char* file_path)
{
     assert(file_path && *file_path);
     char* p;
     for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
          *p='\0';

          if (_mkdir(file_path)==-1) {

               if (errno!=EEXIST) {
                    *p='/';
                    return -1;
               }
          }
          *p='/';
     }
     return 0;
}

#else

int Method_A::mkpath(char* file_path, mode_t mode)
{
     assert(file_path && *file_path);
     char* p;
     for (p=strchr(file_path+1, '/'); p; p=strchr(p+1, '/')) {
          *p='\0';

          if (mkdir(file_path, mode)==-1) {

               if (errno!=EEXIST) {
                    *p='/';
                    return -1;
               }
          }
          *p='/';
     }
     return 0;
}

#endif
