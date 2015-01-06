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
     std::string trajectoryfile="asdfasdf.txt";
     FileFormat trajFormat=FORMAT_XML_PLAIN;
     std::string outputfile="basvas";
     int deltaF=5;
     char vComponent='X';
     int deltaT=100;
     string projectRootDir="";
     MeasurementArea_L  area;
     area._id=1;
     area._lineStartX= 1;
     area._lineStartX= 2;
     area._lineStartX= 2;
     area._lineStartX= 3;

     PedData data(trajectoryfile, trajFormat);
     Method_A method_A ;
     //method_A.SetOutputFileName(outputfile);
     //method_A.SetTrajectoryFile(trajectoryfile);
     //method_A.SetMeasurementArea(area);

     //bool result=method_A.Process(projectRootDir, trajectoryfile, trajFormat, outputfile, area, deltaF, vComponent, deltaT);

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


