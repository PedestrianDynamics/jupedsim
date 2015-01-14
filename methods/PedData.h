/*
 * PedData.h
 *
 *  Created on: Jan 6, 2015
 *      Author: piccolo
 */

#ifndef PEDDATA_H_
#define PEDDATA_H_

#include <string>
#include <vector>
#include <map>
#include "general/Macros.h"
#include "tinyxml/tinyxml.h"
#include "IO/OutputHandler.h"
#include <algorithm>    // std::min_element, std::max_element
#include <boost/algorithm/string.hpp>
using namespace  std;
extern OutputHandler* Log;

#define M2CM 100
#define CMtoM 0.01

class PedData
{
public:
     PedData(const string& projectRootDir, const string& path, const string& filename, const FileFormat& _trajformat, int deltaF, char vComponent);
     virtual ~PedData();
     int GetMinFrame() const;
     int GetMinID() const;
     int GetNumFrames() const;
     int GetNumPeds() const;
     int GetFps() const;
     string GetTrajName() const;
     string GetProjectRootDir() const;
     map<int , vector<int>> GetPedsFrame() const;
     double** GetXCor() const;
     double** GetYCor() const;
     int* GetFirstFrame() const;
     int* GetLastFrame() const;
     vector<int> GetIdInFrame(const vector<int>& ids) const;
     vector<double> GetXInFrame(int frame, const vector<int>& ids) const;
     vector<double> GetYInFrame(int frame, const vector<int>& ids) const;
     vector<double> GetVInFrame(int frame, const vector<int>& ids) const;

private:
     bool ReadData(const string& projectRootDir, const string& path, const string& filename, const FileFormat& _trajformat, int deltaF, char vComponent);
     void InitializeVariables();
     void InitializeVariables(TiXmlElement* xRootNode);
     void CreateGlobalVariables(int numPeds, int numFrames);
     double GetInstantaneousVelocity(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor) const;


private:

     string _trajName;
     string _projectRootDir;
     int _minFrame;
     int _minID;
     int _numFrames;  // total number of frames
     int _numPeds; // total number of pedestrians
     int _fps;
     map<int , vector<int>> _peds_t;

     int _deltaF;
     char _vComponent;

	 int *_firstFrame;   // Record the first frame of each pedestrian
	 int *_lastFrame;    // Record the last frame of each pedestrian
	 double **_xCor;
	 double **_yCor;

};

#endif /* PEDDATA_H_ */
