/*
 * PedData.cpp
 *
 *  Created on: Jan 6, 2015
 *      Author: piccolo
 */

#include <PedData.h>

PedData::PedData(const string& filename, const FileFormat& trajformat)
{
     ReadData(filename, trajformat);
}

PedData::~PedData()
{

}

bool PedData::ReadData(const string& filename, const FileFormat& trajformat)
{
     Log->Write("INFO:\tthe format of the trajectory is ",trajformat);
     if(trajformat == FORMAT_XML_PLAIN) // read traje
     {
          TiXmlDocument docGeo(filename);
          if (!docGeo.LoadFile()) {
               Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
               Log->Write("ERROR: \t could not parse the trajectories file <%s>",filename.c_str());
               return false;
          }
          TiXmlElement* xRootNode = docGeo.RootElement();
          InitializeVariables(xRootNode);	//initialize some global variables
     }

     else if(trajformat == FORMAT_PLAIN)
     {
          InitializeVariables(filename);
     }
     return true;
}

void PedData::InitializeVariables(const string& filename)
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

     _minID = *min_element(_IdsTXT.begin(),_IdsTXT.end());
     _minFrame = *min_element(_FramesTXT.begin(),_FramesTXT.end());

     //Total number of frames
     _numFrames = *max_element(_FramesTXT.begin(),_FramesTXT.end()) - _minFrame+1;

     //Total number of agents
     _numPeds = *max_element(_IdsTXT.begin(),_IdsTXT.end()) - _minID+1;
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

     bool IsinMeasurezone[_numPeds];  // Record whether pedestrian i is in measurement area or not
     for(unsigned int i = 0; i < _IdsTXT.size(); i++)
     {
          int ID = _IdsTXT[i] - _minID;
          int frm = _FramesTXT[i] - _minFrame;
          double x = xs[i]*M2CM;
          double y = ys[i]*M2CM;
          _xCor[ID][frm] = x;
          _yCor[ID][frm] = y;
          if(_fundamentalTinTout==true)
          {
               if(within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly)&&!(IsinMeasurezone[ID])) {
                    _tIn[ID]=frm;
                    IsinMeasurezone[ID] = true;
               }
               if((!within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly))&&IsinMeasurezone[ID]) {
                    _tOut[ID]=frm;
                    IsinMeasurezone[ID] = false;
               }
          }
     }

     //save the data for each frame
     for (unsigned int i = 0; i < _FramesTXT.size(); i++ )
     {
          int id = _IdsTXT[i]-_minID;
          int t =_FramesTXT[i]- _minFrame;
          peds_t[t].push_back(id);
     }

}

// initialize the global variables variables
void PedData::InitializeVariables(TiXmlElement* xRootNode)
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
          _numPeds=atoi(xHeader->FirstChild("agents")->FirstChild()->Value());
          Log->Write("INFO:\tmax num of peds N=%d", _numPeds);
     }

     //framerate
     if(xHeader->FirstChild("frameRate")) {
          _fps=atoi(xHeader->FirstChild("frameRate")->FirstChild()->Value());
          Log->Write("INFO:\tFrame rate fps=%d", _fps);
     }

     CreateGlobalVariables(_numPeds, _numFrames);
     bool IsinMeasurezone[_numPeds];  // Record whether pedestrian i is in measurement area or not
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
               double x= atof(xAgent->Attribute("xPos"));
               double y= atof(xAgent->Attribute("yPos"));
               int ID= atoi(xAgent->Attribute("ID"))-_minID;

               peds_t[frameNr].push_back(ID);
               _xCor[ID][frameNr] =  x*M2CM;
               _yCor[ID][frameNr] =  y*M2CM;
               if(frameNr < _firstFrame[ID]) {
                    _firstFrame[ID] = frameNr;
               }
               if(frameNr > _lastFrame[ID]) {
                    _lastFrame[ID] = frameNr;
               }
               if(_fundamentalTinTout==true)
               {
                    if(within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly)&&!(IsinMeasurezone[ID])) {
                         _tIn[ID]=frameNr;
                         IsinMeasurezone[ID] = true;
                    }
                    if((!within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly))&&IsinMeasurezone[ID]) {
                         _tOut[ID]=frameNr;
                         IsinMeasurezone[ID] = false;
                    }
               }
          }
          frameNr++;
     }

}

void PedData::GetPedsParametersInFrame(int frame, std::map< int, std::vector<int> > &pdt)
{
     const std::vector<int>& ids=pdt[frame];

     for(int i=0; i<ids.size();i++)
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
     _tIn = new int[numPeds];				// Record the time of each pedestrian entering measurement area
     _tOut = new int[numPeds];				// Record the time of each pedestrian exiting measurement area


     for(int i = 0; i <numPeds; i++) {
          for (int j = 0; j < numFrames; j++) {
               _xCor[i][j] = 0;
               _yCor[i][j] = 0;
          }
          _firstFrame[i] = INT_MAX;
          _lastFrame[i] = INT_MIN;
          _tIn[i] = 0;
          _tOut[i] = 0;
     }

     DensityPerFrame = new double[numFrames];
     for(int i=0; i<numFrames; i++) {
          DensityPerFrame[i]=0;
     }
     PassLine = new bool[numPeds];
     for(int i=0; i<numPeds; i++) {
          PassLine[i] = false;
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
