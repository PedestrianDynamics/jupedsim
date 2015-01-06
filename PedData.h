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
#include "general/Macros.h"
#include "tinyxml/tinyxml.h"
#include <algorithm>    // std::min_element, std::max_element
using namespace  std;
using namespace boost::geometry;

#define M2CM 100
#define CMtoM 0.01

class PedData
{
public:
     PedData(const string& filename, const FileFormat& _trajformat);
     virtual ~PedData();
     int GetMinFrame() const;
     int GetMinID() const;
     int GetNumFrames() const;
     int GetFps() const;
     string GetTrajName() const;
     map<int , vector<int> > GetPedsFrame() const;

private:
     bool ReadData(const string& filename, const FileFormat& _trajformat);
     void InitializeVariables(const string& filename);
     void InitializeVariables(TiXmlElement* xRootNode);
     void CreateGlobalVariables(int numPeds, int numFrames);


private:

     string _trajName;
     int _minFrame;
     int _minID;
     int _numFrames;  // total number of frames
     int _numPeds; // total number of pedestrians
     int _fps;
     map<int , vector<int> > _peds_t;

     int *IdInFrame;     // save the ped ID in the geometry in this frame, which is the same order with VInFrame and only used for outputting individual density and velocity.
     double *XInFrame;   // save the X coordinates of pedestrian in the geometry in this frame
     double *YInFrame;   // save the Y coordinates of pedestrian in the geometry in this frame
     double *VInFrame;   // save the instantaneous velocity of pedestrians in the geometry in this frame

	 std::vector<int> _IdsTXT;   // the Id data from txt format trajectory data
	 std::vector<int> _FramesTXT;  // the Frame data from txt format trajectory data
	 int *_firstFrame;   // Record the first frame of each pedestrian
	 int *_lastFrame;    // Record the last frame of each pedestrian
	 double **_xCor;
	 double **_yCor;

};

#endif /* PEDDATA_H_ */
