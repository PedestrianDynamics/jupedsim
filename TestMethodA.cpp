/*
 * Test.cpp
 *
 *  Created on: Jan 5, 2015
 *      Author: piccolo
 */

#include "Method_A.h"
#include "Method_B.h"
#include "Method_C.h"
#include "Method_D.h"
#include "PedData.h"

using namespace std;

int main_1(int argc, char **argv)
{
     std::string trajectoryfile="AO_240.txt";
     string path="./";
     FileFormat trajFormat=FORMAT_PLAIN;
     //FileFormat trajFormat=FORMAT_XML_PLAIN;
     int deltaF=5;
     char vComponent='Y';
     int deltaT=100;
     string projectRootDir="./";
     MeasurementArea_L*  area= new MeasurementArea_L();
     area->_id=1;
     area->_lineStartX= -225;
     area->_lineStartY= 0;
     area->_lineEndX= 400;
     area->_lineEndY= 0;

     MeasurementArea_B* areaB = new MeasurementArea_B();
     const double coor[][2] = {{210,53}, {210,-53}, {-30,-53}, {-30,53},{210,53}};
     polygon_2d poly;
     assign_points(poly, coor);
     correct(poly);
     areaB->_poly=poly;
     areaB->_id=2;
     areaB->_length=100;


     PedData data(projectRootDir, path, trajectoryfile, trajFormat, deltaF, vComponent);
     Method_A method_A ;
     method_A.SetMeasurementArea(area);
     method_A.SetTimeInterval(deltaT);
     bool result=method_A.Process(data);
     if(result)
     {
          cout<<"success with Method A"<<endl;
          delete area;
     }
     else
     {
          cout<<"failure"<<endl;
     }

     Method_C method_C;
     method_C.SetMeasurementArea(areaB);
     bool result_C =method_C.Process(data);
     if(result_C)
     {
          cout<<"success with Method C"<<endl;
     }
     else
     {
          cout<<"failure"<<endl;
     }

/*     Method_D method_D;
     method_D.SetMeasurementArea(areaB);
     method_D.SetMeasurementAreaId(areaB._id);
     bool result_D =method_C.Process(data);
     if(result_D)
     {
          cout<<"success with Method C"<<endl;
     }
     else
     {
          cout<<"failure"<<endl;
     }*/
     delete areaB;
     return 0;
}


