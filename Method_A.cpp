/*
 * Method_A.cpp
 *
 *  Created on: Jan 5, 2015
 *      Author: piccolo
 */

#include <Method_A.h>
#include <iostream>


Method_A::Method_A()
{

}

Method_A::~Method_A()
{

}

bool Method_A::Process (const PedData& peddata, const string& projectRootDir, const MeasurementArea_L& area, int deltaF, char vComponent, int deltaT)
{
     FILE *_fN_t;
     string trajName = peddata.GetTrajName();
     OpenFile_N_t(_fN_t, projectRootDir, trajName);
     _peds_t = peddata.GetPedsFrame();
     for(int frameNr = 0; frameNr < peddata.GetNumFrames(); frameNr++ )
     	{
     		int frid =  frameNr + peddata.GetMinFrame();
            GetPedsParametersInFrame(frameNr, _peds_t);
     		_accumPedsPassLine.push_back(ClassicFlow);
     		_accumVPassLine.push_back(V_deltaT);
     		fprintf(_fN_t,"%d\t%d\n",frid, ClassicFlow);
     	}

     	string FD_FlowVelocity=  _projectRootDir+"./Output/Fundamental_Diagram/FlowVelocity/FDFlowVelocity_"+trajName+".dat";
     	FlowRate_Velocity(deltaT,peddata.GetFps(), _accumPedsPassLine,_accumVPassLine,FD_FlowVelocity);

     	//delete [] PassLine;
     fclose(_fN_t);
     return true;
}

void Method_A::OpenFile_N_t(FILE *& file, const string& projectRootDir, const string& filename)
{
	string fN_t= _projectRootDir+"./Output/Fundamental_Diagram/FlowVelocity/Flow_NT_"+filename+"_Out.dat";
	if((file=CreateFile(fN_t))==NULL)
	{
		Log->Write("cannot open the file %s  t\n", fN_t.c_str() );
		exit(EXIT_FAILURE);
	}
	fprintf(file,"#Frame\t	Cumulative pedestrians\n");
}
bool Method_A::Process(const string& projectRootDir, const string& trajectoryfile, const FileFormat& _trajFormat,const string& outputfile, const MeasurementArea_L& area, int deltaF, char vComponent, int deltaT)
{

	_deltaF = deltaF;
	_vComponent = vComponent;
	_areaForMethod_A = area;
	_projectRootDir = projectRootDir;

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
     PassLine = new bool[numPeds];
     for(int i=0; i<numPeds; i++) {
          PassLine[i] = false;
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
          IdInFrame[i] = id+_minID;

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
