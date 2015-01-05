/*
 * Test.cpp
 *
 *  Created on: Jan 5, 2015
 *      Author: piccolo
 */

#include "Method_A.h"
using namespace std;

int main(int argc, char **argv)
{

     std::string trajectoryfile="asdfasdf.txt";
     std::string outputfile="basvas";

     MeasurementArea_L  area;
     area._id=1;
     area._lineStartX= 1;
     area._lineStartX= 2;
     area._lineStartX= 2;
     area._lineStartX= 3;

     Method_A method_A ;
     //method_A.SetOutputFileName(outputfile);
     //method_A.SetTrajectoryFile(trajectoryfile);
     //method_A.SetMeasurementArea(area);

     bool result=method_A.Process(trajectoryfile, outputfile, area);

     if(result)
     {
          cout<<"success"<<endl;
     }
     else
     {
          cout<<"failure"<<endl;
     }

}


