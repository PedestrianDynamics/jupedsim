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


class Method_A
{
public:
     Method_A();
     virtual ~Method_A();

     //void SetTrajectoryFile(const std::string& file);
     //void SetMeasurementArea (const MeasurementArea_L& area);
     //void SetOutputFileName (const std::string& file);

     bool Process(const std::string& trajectoryfile, const std::string& outputfile, const MeasurementArea_L& area);

private:
     std::string _trajectoryfile;
     MeasurementArea_L  _area;
     std::string _outputfile;
};

#endif /* METHOD_A_H_ */
