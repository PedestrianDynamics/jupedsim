/**
 * \file        PedData.cpp
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J��lich GmbH. All rights reserved.
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
#include  <cmath>

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

bool PedData::ReadData(const string& projectRootDir, const string& path, const string& filename, const FileFormat& trajformat, int deltaF, char vComponent)
{
     _minID = INT_MAX;
     _minFrame = INT_MAX;
     _deltaF = deltaF;
     _vComponent = vComponent;
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
                         Log->Write("INFO:\tFrame rate fps: <%d>", _fps);
                    }

               }
               else if ( line[0] != '#' && !(line.empty()) )
               {

                    std::vector<std::string> strs;
                    boost::split(strs, line , boost::is_any_of("\t "),boost::token_compress_on);

                    if(strs.size() <4)
                    {
                         Log->Write("ERROR:\t There is an error in the file at line %d", lineNr);
                         return false;
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

     _minID = *min_element(_IdsTXT.begin(),_IdsTXT.end());
     _minFrame = *min_element(_FramesTXT.begin(),_FramesTXT.end());

     //Total number of frames
     _numFrames = *max_element(_FramesTXT.begin(),_FramesTXT.end()) - _minFrame+1;

     //Total number of agents
     _numPeds = *max_element(_IdsTXT.begin(),_IdsTXT.end()) - _minID+1;
     vector<int> Ids_temp=_IdsTXT;
     sort (Ids_temp.begin(), Ids_temp.end());
     Ids_temp.erase(unique(Ids_temp.begin(), Ids_temp.end()), Ids_temp.end());
     if((unsigned)_numPeds!=Ids_temp.size())
     {
          Log->Write("Error:\tThe index of ped ID is not continuous. Please modify the trajectory file!");
          return false;
     }
     CreateGlobalVariables(_numPeds, _numFrames);

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
          _firstFrame[i] = _FramesTXT[firstFrameIndex[i]] - _minFrame;
          _lastFrame[i] = _FramesTXT[lastFrameIndex[i]] - _minFrame;
     }

     for(unsigned int i = 0; i < _IdsTXT.size(); i++)
     {
          int ID = _IdsTXT[i] - _minID;
          int frm = _FramesTXT[i] - _minFrame;
          double x = xs[i]*M2CM;
          double y = ys[i]*M2CM;
          _xCor[ID][frm] = x;
          _yCor[ID][frm] = y;
     }

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
          Log->Write("ERROR:\tRoot element value is not 'geometry'.");
          return false;
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
          _numPeds=atoi(xHeader->FirstChild("agents")->FirstChild()->Value());
          Log->Write("INFO:\tmax num of peds N=%d", _numPeds);
     }

     //framerate
     if(xHeader->FirstChild("frameRate")) {
          _fps=atoi(xHeader->FirstChild("frameRate")->FirstChild()->Value());
          Log->Write("INFO:\tFrame rate fps: <%d>", _fps);
     }

     CreateGlobalVariables(_numPeds, _numFrames);

     //processing the frames node
     TiXmlNode*  xFramesNode = xRootNode->FirstChild("frame");
     if (!xFramesNode) {
          Log->Write("ERROR: \tThe geometry should have at least one frame");
          return false;
     }

     // obtaining the minimum id and minimum frame
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame"))
     {
          int frm = atoi(xFrame->Attribute("ID"));
          if(frm < _minFrame)
          {
               _minFrame = frm;
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
     int frameNr=0;
     for(TiXmlElement* xFrame = xRootNode->FirstChildElement("frame"); xFrame;
               xFrame = xFrame->NextSiblingElement("frame")) {

          //todo: can be parallelized with OpenMP
          for(TiXmlElement* xAgent = xFrame->FirstChildElement("agent"); xAgent;
                    xAgent = xAgent->NextSiblingElement("agent")) {

               //get agent id, x, y
               double x= atof(xAgent->Attribute("x"));
               double y= atof(xAgent->Attribute("y"));
               int ID= atoi(xAgent->Attribute("ID"))-_minID;

               _peds_t[frameNr].push_back(ID);
               _xCor[ID][frameNr] =  x*M2CM;
               _yCor[ID][frameNr] =  y*M2CM;
               if(frameNr < _firstFrame[ID])
               {
                    _firstFrame[ID] = frameNr;
               }
               if(frameNr > _lastFrame[ID])
               {
                    _lastFrame[ID] = frameNr;
               }
          }
          frameNr++;
     }
     return true;
}

vector<double> PedData::GetVInFrame(int frame, const vector<int>& ids) const
{
     vector<double> VInFrame;
     for(unsigned int i=0; i<ids.size();i++)
     {
          int id = ids[i];
          int Tpast = frame - _deltaF;
          int Tfuture = frame + _deltaF;
          double v = GetInstantaneousVelocity(frame, Tpast, Tfuture, id, _firstFrame, _lastFrame, _xCor, _yCor);
          VInFrame.push_back(v);
     }
     return VInFrame;
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

double PedData::GetInstantaneousVelocity(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor) const
{

     double v=0.0;

     if(_vComponent == 'X')
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
     if(_vComponent == 'Y')
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
     if(_vComponent == 'B')
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

void PedData::CreateGlobalVariables(int numPeds, int numFrames)
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

int PedData::GetFps() const
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
