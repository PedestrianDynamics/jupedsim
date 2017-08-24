/**
 * \file        PedData.cpp
 * \date        Feb 10, 2016
 * \version     v0.8
 * \copyright   <2009-2015> Forschungszentrum J��lich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * In this file functions related to reading data from files are defined.
 *
 *
 **/

#include "PedData.h"
#include <cmath>
#include <string>

using std::string;
using std::map;
using std::vector;
using std::ifstream;


PedData::PedData()
{
}

PedData::~PedData()
{

}

bool PedData::ReadData(const string& projectRootDir, const string& path, const string& filename, const FileFormat& trajformat, int deltaF, std::string vComponent, const bool IgnoreBackwardMovement)
{
     _minID = INT_MAX;
     _minFrame = INT_MAX;
     _deltaF = deltaF;
     _vComponent = vComponent;
     _IgnoreBackwardMovement=IgnoreBackwardMovement;
     _projectRootDir = projectRootDir;
     _trajName = filename;

     string fullTrajectoriesPathName= path+"./"+_trajName;
     Log->Write("INFO:\tthe name of the trajectory is: <%s>",_trajName.c_str());

     bool result=true;
     if(trajformat == FORMAT_XML_PLAIN)
     {
          TiXmlDocument docGeo(fullTrajectoriesPathName);
          if (!docGeo.LoadFile()) {
               Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
               Log->Write("ERROR: \t could not parse the trajectories file <%s>",fullTrajectoriesPathName.c_str());
               return false;
          }
           TiXmlElement* xRootNode = docGeo.RootElement();
          result=InitializeVariables(xRootNode);	//initialize some global variables
     }

     else if(trajformat == FORMAT_PLAIN)
     {
          result=InitializeVariables(fullTrajectoriesPathName);
     }
     return result;
}

bool PedData::InitializeVariables(const string& filename)
{
     vector<double> xs;
     vector<double> ys;
     vector<double> zs;
     vector<string> vcmp; // the direction identification for velocity calculation
     vector<int> _IdsTXT;   // the Id data from txt format trajectory data
     vector<int> _FramesTXT;  // the Frame data from txt format trajectory data
     //string fullTrajectoriesPathName= _projectRootDir+"./"+_trajName;
     ifstream  fdata;
     fdata.open(filename.c_str());
     if (fdata.is_open() == false)
     {
          Log->Write("ERROR: \t could not parse the trajectories file <%s>",filename.c_str());
          return false;
     }
     else
     {
          string line;
          int lineNr=1;
          int pos_id=0;
          int pos_fr=1;
          int pos_x=2;
          int pos_y=3;
          int pos_z=4;
          int pos_vd=5; //velocity direction
          int indx=1, index = 0 , lastindex = 0;
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
                         Log->Write("INFO:\tFrame rate fps: <%.2f>", _fps);
                    }

                    if(line.find("ID") != std::string::npos &&
                       line.find("FR") != std::string::npos &&
                       line.find("X") != std::string::npos &&
                       line.find("Y") != std::string::npos &&
                       line.find("Z") != std::string::npos)
                    {
                         // looking for this line
                         // #ID	 FR  X Y Z
                         std::cout << "in if" << std::endl;
                    	std::vector<std::string> strs1;
                    	line.erase(0,1);
                    	boost::split(strs1, line , boost::is_any_of("\t "),boost::token_compress_on);
                    	vector<string>::iterator it_id;
                    	it_id=find(strs1.begin(),strs1.end(),"ID");
                    	pos_id = std::distance(strs1.begin(), it_id);
                    	it_id=find(strs1.begin(),strs1.end(),"FR");
                    	pos_fr = std::distance(strs1.begin(), it_id);
                    	it_id=find(strs1.begin(),strs1.end(),"X");
                    	pos_x = std::distance(strs1.begin(), it_id);
                    	it_id=find(strs1.begin(),strs1.end(),"Y");
                    	pos_y = std::distance(strs1.begin(), it_id);
                    	it_id=find(strs1.begin(),strs1.end(),"Z");
                        pos_z = std::distance(strs1.begin(), it_id);
                        it_id=find(strs1.begin(),strs1.end(),"VD");
                        pos_vd = std::distance(strs1.begin(), it_id);
                    }

               }
               else if ( line[0] != '#' && !(line.empty()) )
               {
                    static int once = 1;
                    if (lineNr % 100000 == 0)
                         std::cout << "lineNr " << lineNr<< std::endl;
                    std::vector<std::string> strs;
                    boost::algorithm::trim_right(line);
                    boost::split(strs, line , boost::is_any_of("\t "),boost::token_compress_on);
                    if(once && strs.size() < 5)
                    {
                         once = 0;
                         Log->Write("INFO: pos_id: %d", pos_id);
                         Log->Write("INFO: pos_fr: %d", pos_fr);
                         Log->Write("INFO: pos_x: %d", pos_x);
                         Log->Write("INFO: pos_y: %d", pos_y);
                         Log->Write("INFO: pos_z: %d", pos_z);
                         Log->Write("WARNING:\t assuming z=0 for all data");
                    }
                    lastindex = index;
                    index = atoi(strs[pos_id].c_str());
                    if (index != lastindex){
                         indx += 1;
                    }
                    _IdsTXT.push_back(indx);
                    _FramesTXT.push_back(atoi(strs[pos_fr].c_str()));
                    xs.push_back(atof(strs[pos_x].c_str()));
                    ys.push_back(atof(strs[pos_y].c_str()));
                    if(strs.size() >= 5)
                         zs.push_back(atof(strs[pos_z].c_str()));
                    else
                         zs.push_back(0);

                    if(_vComponent=="F")
                    {
                         if(strs.size() >= 6 && pos_vd < (int)strs.size() )
                         {
                              vcmp.push_back(strs[pos_vd].c_str());
                         }
                         else
                         {
                              Log->Write("ERROR:\t There is no indicator for velocity component in trajectory file or ini file!!");
                              return false;
                         }
                    }
               }
               lineNr++;
          }
          Log->Write("INFO:\t Finished reading the data");
          
     }
     fdata.close();
     Log->Write("INFO: Got %d lines", _IdsTXT.size());
     _minID = *min_element(_IdsTXT.begin(),_IdsTXT.end());
     Log->Write("INFO: minID: %d", _minID);
     _minFrame = *min_element(_FramesTXT.begin(),_FramesTXT.end());
     Log->Write("INFO: minFrame: %d", _minFrame);
     //Total number of frames
     _numFrames = *max_element(_FramesTXT.begin(),_FramesTXT.end()) - _minFrame+1;
     Log->Write("INFO: numFrames: %d", _numFrames);

     //Total number of agents
     _numPeds = *max_element(_IdsTXT.begin(),_IdsTXT.end()) - _minID+1;
     Log->Write("INFO: Total number of Agents: %d", _numPeds);
     CreateGlobalVariables(_numPeds, _numFrames);
     Log->Write("INFO: Create Global Variables done");
     
     for(int i=_minID;i<_minID+_numPeds; i++)
     {
    	 int firstFrameIndex=INT_MAX;   //The first frame index of a pedestrian
    	 int lastFrameIndex=-1;    //The last frame index of a pedestrian
    	 int actual_totalframe=0;  //The total data points of a pedestrian in the trajectory
    	 for (auto j = _IdsTXT.begin(); j != _IdsTXT.end(); ++j)
    	     {
    	         if (*j ==i)
    	         {
    	             int pos = std::distance(_IdsTXT.begin(), j);
    	             if(pos<firstFrameIndex)
    	             {
    	            	 firstFrameIndex=pos;
    	             }
    	             if(pos>lastFrameIndex)
    	             {
    	            	 lastFrameIndex=pos;
    	             }
    	             actual_totalframe++;
    	         }
    	     }
    	 if(lastFrameIndex==0)
    	 {
    		 Log->Write("Warning:\tThere is no trajectory for ped with ID <%d>!",i);
    	 }
    	 _firstFrame[i-_minID] = _FramesTXT[firstFrameIndex] - _minFrame;
    	 _lastFrame[i-_minID] = _FramesTXT[lastFrameIndex] - _minFrame;

	     int expect_totalframe=_lastFrame[i-_minID]-_firstFrame[i-_minID]+1;
	     if(actual_totalframe != expect_totalframe)
	     {
		    Log->Write("Error:\tThe trajectory of ped with ID <%d> is not continuous. Please modify the trajectory file!",i);
		    return false;
	     }
     }
     Log->Write("convert x and y");
     for(unsigned int i = 0; i < _IdsTXT.size(); i++)
     {
          int ID = _IdsTXT[i] - _minID;
          int frm = _FramesTXT[i] - _minFrame;
          double x = xs[i]*M2CM;
          double y = ys[i]*M2CM;
          double z = zs[i]*M2CM;
          _xCor[ID][frm] = x;
          _yCor[ID][frm] = y;
          _zCor[ID][frm] = z;
          if(_vComponent == "F")
          {
        	  _vComp[ID][frm] = vcmp[i];
          }
          else
          {
        	  _vComp[ID][frm] = _vComponent;
          }
     }
     Log->Write("Save the data for each frame");
     
     //save the data for each frame
     for (unsigned int i = 0; i < _FramesTXT.size(); i++ )
     {
          int id = _IdsTXT[i]-_minID;
          int t =_FramesTXT[i]- _minFrame;
          _peds_t[t].push_back(id);

     }

     return true;
}

// initialize the global variables variables
bool PedData::InitializeVariables(TiXmlElement* xRootNode)
{
     if( ! xRootNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          return false;
     }
     if( xRootNode->ValueStr () != "trajectories" ) {
          Log->Write("ERROR:\tRoot element value is not 'trajectories'.");
          return false;
     }

     //Number of agents

     TiXmlNode*  xHeader = xRootNode->FirstChild("header"); // header
     if(xHeader->FirstChild("agents")) {
          _numPeds=atoi(xHeader->FirstChild("agents")->FirstChild()->Value());
          Log->Write("INFO:\tmax num of peds N=%d", _numPeds);
     }

     //framerate
     if(xHeader->FirstChild("frameRate")) {
          _fps=atoi(xHeader->FirstChild("frameRate")->FirstChild()->Value());
          Log->Write("INFO:\tFrame rate fps: <%.2f>", _fps);
     }


     //processing the frames node
     TiXmlNode*  xFramesNode = xRootNode->FirstChild("frame");
     if (!xFramesNode) {
          Log->Write("ERROR: \tThe geometry should have at least one frame");
          return false;
     }

     // obtaining the minimum id and minimum frame
     int maxFrame=0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame"))
     {
          int frm = atoi(xFrame->Attribute("ID"));
          if(frm < _minFrame)
          {
               _minFrame = frm;
          }
          if(frm>maxFrame)
          {
        	  maxFrame=frm;
          }
          for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
                    xAgent = xAgent->NextSiblingElement("agent"))
          {
               int id= atoi(xAgent->Attribute("ID"));
               if(id < _minID)
               {
                    _minID = id;
               }
          }
     }

     //counting the number of frames
     _numFrames = maxFrame-_minFrame+1;
     Log->Write("INFO:\tnum Frames = %d",_numFrames);

     CreateGlobalVariables(_numPeds, _numFrames);

     vector<int> totalframes;
     for (int i = 0; i <_numPeds; i++)
     {
    	 totalframes.push_back(0);
     }
     //int frameNr=0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame"))
     {
    	  int frameNr = atoi(xFrame->Attribute("ID")) - _minFrame;
          //todo: can be parallelized with OpenMP
          for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
                    xAgent = xAgent->NextSiblingElement("agent"))
          {
               //get agent id, x, y
               double x= atof(xAgent->Attribute("x"));
               double y= atof(xAgent->Attribute("y"));
               double z= atof(xAgent->Attribute("z"));
               int ID= atoi(xAgent->Attribute("ID"))-_minID;
               if(ID>=_numPeds)
               {
            	   Log->Write("ERROR:\t The number of agents are not corresponding to IDs. Maybe Ped IDs are not continuous in the first frame, please check!!");
            	   return false;
               }

               _peds_t[frameNr].push_back(ID);
               _xCor[ID][frameNr] =  x*M2CM;
               _yCor[ID][frameNr] =  y*M2CM;
               _zCor[ID][frameNr] =  z*M2CM;
               if(_vComponent == "F")
               {
            	   if(xAgent->Attribute("VD"))
            	   {
            	       _vComp[ID][frameNr] = *string(xAgent->Attribute("VD")).c_str();
            	   }
            	   else
            	   {
            		   Log->Write("ERROR:\t There is no indicator for velocity component in trajectory file or ini file!!");
            		   return false;
            	   }
               }
               else
			  {
				  _vComp[ID][frameNr] = _vComponent;
			  }

               if(frameNr < _firstFrame[ID])
               {
                    _firstFrame[ID] = frameNr;
               }
               if(frameNr > _lastFrame[ID])
               {
                    _lastFrame[ID] = frameNr;
               }
               totalframes[ID] +=1;
          }
          //frameNr++;
     }
     for(int id = 0; id<_numPeds; id++)
     {
         int actual_totalframe= totalframes[id];
         int expect_totalframe=_lastFrame[id]-_firstFrame[id]+1;
         if(actual_totalframe != expect_totalframe)
         {
             Log->Write("Error:\tThe trajectory of ped with ID <%d> is not continuous. Please modify the trajectory file!",id+_minID);
             return false;
         }
     }

     return true;
}

vector<double> PedData::GetVInFrame(int frame, const vector<int>& ids, double zPos) const
{
     vector<double> VInFrame;
     for(unsigned int i=0; i<ids.size();i++)
     {
          int id = ids[i];
          int Tpast = frame - _deltaF;
		  int Tfuture = frame + _deltaF;
		  double v = GetInstantaneousVelocity1(frame, Tpast, Tfuture, id, _firstFrame, _lastFrame, _xCor, _yCor);
          if(zPos<1000000.0)
          {
        	  if(fabs(_zCor[id][frame]-zPos*M2CM)<J_EPS_EVENT)
        	  {
        		  VInFrame.push_back(v);
        	  }
          }
          else
          {
        	  VInFrame.push_back(v);
          }
     }
     return VInFrame;
}

vector<double> PedData::GetXInFrame(int frame, const vector<int>& ids, double zPos) const
{
     vector<double> XInFrame;
     for(int id:ids)
     {
    	 if(zPos<1000000.0)
		  {
    		 if(fabs(_zCor[id][frame]-zPos*M2CM)<J_EPS_EVENT)
			  {
				  XInFrame.push_back(_xCor[id][frame]);
			  }
		  }
		  else
		  {
			  XInFrame.push_back(_xCor[id][frame]);
		  }
     }
     return XInFrame;
}

vector<double> PedData::GetXInFrame(int frame, const vector<int>& ids) const
{
     vector<double> XInFrame;
     for(int id:ids)
     {
			  XInFrame.push_back(_xCor[id][frame]);
     }
     return XInFrame;
}

vector<double> PedData::GetYInFrame(int frame, const vector<int>& ids, double zPos) const
{
     vector<double> YInFrame;
     for(unsigned int i=0; i<ids.size();i++)
     {
          int id = ids[i];
          if(zPos<1000000.0)
          {
        	  if(fabs(_zCor[id][frame]-zPos*M2CM)<J_EPS_EVENT)
			  {
				  YInFrame.push_back(_yCor[id][frame]);
			  }
          }
          else
          {
        	  YInFrame.push_back(_yCor[id][frame]);
          }
     }
     return YInFrame;
}

vector<double> PedData::GetYInFrame(int frame, const vector<int>& ids) const
{
     vector<double> YInFrame;
     for(unsigned int i=0; i<ids.size();i++)
     {
          int id = ids[i];
          YInFrame.push_back(_yCor[id][frame]);
     }
     return YInFrame;
}

vector<double> PedData::GetZInFrame(int frame, const vector<int>& ids) const
{
     vector<double> ZInFrame;
     for(unsigned int i=0; i<ids.size();i++)
     {
          int id = ids[i];
          ZInFrame.push_back(_zCor[id][frame]);
     }
     return ZInFrame;
}

vector<int> PedData::GetIdInFrame(const vector<int>& ids) const
{
     vector<int> IdInFrame;
     for(int id:ids)
     {
          id = id +_minID;
          IdInFrame.push_back(id);
     }
     return IdInFrame;
}

vector<int> PedData::GetIdInFrame(int frame, const vector<int>& ids, double zPos) const
{
     vector<int> IdInFrame;
     for(int id:ids)
     {
          if(zPos<1000000.0)
          {
			  if(fabs(_zCor[id][frame]-zPos*M2CM)<J_EPS_EVENT)
			  {
				  IdInFrame.push_back(id +_minID);
			  }
          }
          else
          {
        	  IdInFrame.push_back(id +_minID);
          }
     }
     return IdInFrame;
}

double PedData::GetInstantaneousVelocity(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor) const
{
     std::string vcmp = _vComp[ID][Tnow];
     double v=0.0;
    //check the component used in the calculation of velocity
     if(vcmp == "X" || vcmp == "X+"|| vcmp == "X-")
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
          if((vcmp=="X+"&& v<0)||(vcmp=="X-"&& v>0))            //no moveback
               v=0;
     }
     else if(vcmp == "Y" || vcmp == "Y+"|| vcmp == "Y-")
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
          if((vcmp=="Y+"&& v<0)||(vcmp=="Y-"&& v>0))        //no moveback
               v=0;
     }
     else if(vcmp == "B")
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

     return fabs(v);
}

double PedData::GetInstantaneousVelocity1(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor) const
{
     std::string vcmp = _vComp[ID][Tnow];  // the vcmp is the angle from 0 to 360
     if(vcmp=="X+")
     {
    	 vcmp="0";
     }
     else if(vcmp=="Y+")
	 {
		 vcmp="90";
	 }
     if(vcmp=="X-")
	 {
		 vcmp="180";
	 }
     if(vcmp=="Y-")
	 {
		 vcmp="270";
	 }
     double v=0.0;
     if(vcmp != "B")  //check the component used in the calculation of velocity
     {
    	 float alpha=atof(vcmp.c_str())*2*M_PI/360.0;

    	 if((Tpast >=Tfirst[ID])&&(Tfuture <= Tlast[ID]))
		  {
			 v=  _fps*CMtoM*((Xcor[ID][Tfuture] - Xcor[ID][Tpast])*cos(alpha)+(Ycor[ID][Tfuture] - Ycor[ID][Tpast])*sin(alpha))/(2.0 * _deltaF);
		  }
		  else if((Tpast <Tfirst[ID])&&(Tfuture <= Tlast[ID]))
		  {
			   v = _fps*CMtoM*((Xcor[ID][Tfuture] - Xcor[ID][Tnow])*cos(alpha)+(Ycor[ID][Tfuture] - Ycor[ID][Tnow])*sin(alpha))/(_deltaF);  //one dimensional velocity
		  }
		  else if((Tpast >=Tfirst[ID])&&(Tfuture > Tlast[ID]))
		  {
			   v = _fps*CMtoM*((Xcor[ID][Tnow] - Xcor[ID][Tpast])*cos(alpha)+(Ycor[ID][Tnow] - Ycor[ID][Tpast])*sin(alpha))/( _deltaF);  //one dimensional velocity
		  }
		  if(_IgnoreBackwardMovement && v<0)           //if no move back and pedestrian moves back, his velocity is set as 0;
		  {
			   v=0;
		  }

     }
     else if(vcmp == "B")
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

     return v;
}

void PedData::CreateGlobalVariables(int numPeds, int numFrames)
{
     _xCor = new double* [numPeds];
     _yCor = new double* [numPeds];
     _zCor = new double* [numPeds];
     _vComp = new string* [numPeds];
     for (int i=0; i<numPeds; i++) {
          _xCor[i] = new double [numFrames];
          _yCor[i] = new double [numFrames];
          _zCor[i] = new double [numFrames];
          _vComp[i] =new string [numFrames];
     }
     _firstFrame = new int[numPeds];  // Record the first frame of each pedestrian
     _lastFrame = new int[numPeds];  // Record the last frame of each pedestrian
     for(int i = 0; i <numPeds; i++) {
          for (int j = 0; j < numFrames; j++) {
               _xCor[i][j] = 0;
               _yCor[i][j] = 0;
               _zCor[i][j] = 0;
               _vComp[i][j] ="B";
          }
          _firstFrame[i] = INT_MAX;
          _lastFrame[i] = INT_MIN;
     }

}


int PedData::GetMinFrame() const
{
     return _minFrame;
}

int PedData::GetMinID() const
{
     return _minID;
}

int PedData::GetNumFrames() const
{
     return _numFrames;
}

int PedData::GetNumPeds() const
{
     return _numPeds;
}

float PedData::GetFps() const
{
     return _fps;
}

string PedData::GetTrajName() const
{
     return _trajName;
}

map<int , vector<int> > PedData::GetPedsFrame() const
{
     return _peds_t;
}

double** PedData::GetXCor() const
{
     return _xCor;
}
double** PedData::GetYCor() const
{
     return _yCor;
}

double** PedData::GetZCor() const
{
     return _zCor;
}

int* PedData::GetFirstFrame() const
{
     return _firstFrame;
}
int* PedData::GetLastFrame() const
{
     return _lastFrame;
}

string PedData::GetProjectRootDir() const
{
     return _projectRootDir;
}
