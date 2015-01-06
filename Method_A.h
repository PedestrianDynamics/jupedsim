/*
 * Method_A.h
 *
 *  Created on: Jan 5, 2015
 *      Author: piccolo
 */

#ifndef METHOD_A_H_
#define METHOD_A_H_

#include <string>
#include "MeasurementArea.h"
#include "PedData.h"
#include <vector>
#include "general/Macros.h"
#include "tinyxml/tinyxml.h"
#include "IO/OutputHandler.h"
#define M2CM 100
#define CMtoM 0.01

using namespace boost::geometry;
typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double> > segment;

using namespace std;

class Method_A
{
public:
     Method_A();
     virtual ~Method_A();

     //void SetTrajectoryFile(const std::string& file);
     //void SetMeasurementArea (const MeasurementArea_L& area);
     //void SetOutputFileName (const std::string& file);

     bool Process(const string& projectRootDir, const string& trajectoryfile, const FileFormat& _trajFormat,  const string& outputfile, const MeasurementArea_L& area, int deltaF, char vComponent, int deltaT);

     bool Process (const PedData& peddata, const string& projectRootDir, const MeasurementArea_L& area, int deltaF, char vComponent, int deltaT);
private:
     string _trajectoryfile;
     MeasurementArea_L  _area;
     string _outputfile;
     vector<int> _accumPedsPassLine; // the accumulative pedestrians pass a line with time
     vector<double> _accumVPassLine; // the accumulative instantaneous velocity of the pedestrians pass a line
     map<int , vector<int> > _peds_t;



     bool *PassLine;

     int *IdInFrame;     // save the ped ID in the geometry in this frame, which is the same order with VInFrame and only used for outputting individual density and velocity.
     double *XInFrame;   // save the X coordinates of pedestrian in the geometry in this frame
     double *YInFrame;   // save the Y coordinates of pedestrian in the geometry in this frame
     double *VInFrame;   // save the instantaneous velocity of pedestrians in the geometry in this frame
     int ClassicFlow;    // the number of pedestrians pass a line in a certain time
     double V_deltaT;    // define this is to measure cumulative velocity each pedestrian pass a measure line each time step to calculate the <v>delat T=sum<vi>/N
     OutputHandler* Log;

     int _deltaF;         // half of the time interval that used to calculate instantaneous velocity of ped i.
     char _vComponent;        // to mark whether x, y or x and y coordinate are used when calculating the velocity
     MeasurementArea_L _areaForMethod_A;
     string _projectRootDir;

     /**
           * Calculate the Flow rate during a certain time interval DeltaT and the mean velocity passing a line.
           * Note: here the time interval in calculating the flow rate is modified.
           * it is the actual time between the first person and last person
           * passing the line in DeltaT.
           *
           * @param DeltaT
           * @param fps
           * @param AccumPeds
           * @param AccumVelocity
           * @param ofile
           */
     	  void OpenFile_N_t(FILE *& file, const string& projectRootDir, const string& filename);

          void FlowRate_Velocity(int DeltaT, int fps, const std::vector<int>& AccumPeds,
                    const std::vector<double>& AccumVelocity, const std::string& ofile);


          void CreateGlobalVariables(int numPeds, int numFrames);

          void GetPedsParametersInFrame(int frame, std::map< int, std::vector<int> > &pdt);

          bool IsPassLine(double Line_startX,double Line_startY, double Line_endX, double Line_endY,double pt1_X, double pt1_Y,double pt2_X, double pt2_Y);

          double GetVinFrame(int Tnow,int Tpast, int Tfuture, int ID, int *Tfirst, int *Tlast, double **Xcor, double **Ycor, char VComponent);

};

#endif /* METHOD_A_H_ */
