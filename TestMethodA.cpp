/*
 * Test.cpp
 *
 *  Created on: Jan 5, 2015
 *      Author: piccolo
 */

#include "Method_A.h"
#include "PedData.h"

using namespace std;

int main(int argc, char **argv)
{
     std::string trajectoryfile="AO_240.xml";
     //FileFormat trajFormat=FORMAT_PLAIN;
     FileFormat trajFormat=FORMAT_XML_PLAIN;
     int deltaF=5;
     char vComponent='Y';
     int deltaT=100;
     string projectRootDir="./";
     MeasurementArea_L  area;
     area._id=1;
     area._lineStartX= -225;
     area._lineStartY= 0;
     area._lineEndX= 400;
     area._lineEndY= 0;

     PedData data(projectRootDir, trajectoryfile, trajFormat, deltaF, vComponent);
     Method_A method_A ;
     method_A.SetMeasurementArea(area);
     method_A.SetMeasurementAreaId(area._id);
     method_A.SetTimeInterval(deltaT);
     bool result=method_A.Process(data);


     if(result)
     {
          cout<<"success"<<endl;
     }
     else
     {
          cout<<"failure"<<endl;
     }

}


