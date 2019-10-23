/**
 * \file        Method_B.cpp
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2017> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * In this file functions related to method B are defined.
 *
 *
 **/

#include "Method_B.h"

using std::string;
using std::vector;


Method_B::Method_B()
{
     _tIn = nullptr;
     _tOut = nullptr;
     _entrancePoint = {};
     _exitPoint = {};
     _DensityPerFrame = nullptr;
     _fps = 10;
     _NumPeds =0;
     _areaForMethod_B = nullptr;
}

Method_B::~Method_B()
{

}

bool Method_B::Process (const PedData& peddata)
{
     Log->Write("------------------------Analyzing with Method B-----------------------------");
     _trajName = peddata.GetTrajName();
     _projectRootDir = peddata.GetProjectRootDir();
     _outputLocation = peddata.GetOutputLocation();

     _fps =peddata.GetFps();
     _peds_t = peddata.GetPedsFrame();
     _NumPeds = peddata.GetNumPeds();
     _xCor = peddata.GetXCor();
     _yCor = peddata.GetYCor();
     _measureAreaId = boost::lexical_cast<string>(_areaForMethod_B->_id);
     _tIn = new int[_NumPeds];				// Record the time of each pedestrian entering measurement area
     _tOut = new int[_NumPeds];
     std::vector<Point> entp(_NumPeds);
     std::vector<Point> extp(_NumPeds);
     _entrancePoint = entp;
     _exitPoint = extp;
     for (int i=0; i<_NumPeds; i++)
     {
          _tIn[i] = 0;
          _tOut[i] = 0;
          _entrancePoint[i] = Point(0,0);
          _exitPoint[i] = Point(0,0);
     }
     GetTinTout(peddata.GetNumFrames());

     if(_areaForMethod_B->_length<0)
     {
          Log->Write("INFO:\tThe measurement area length for method B is not assigned!");
     }
     else
     {
          Log->Write("INFO:\tThe measurement area length for method B is %.3f", _areaForMethod_B->_length);
     }
     GetFundamentalTinTout(_DensityPerFrame,_areaForMethod_B->_length);

     delete []_tIn;
     delete []_tOut;
     return true;
}

void Method_B::GetTinTout(int numFrames)
{
     bool* IsinMeasurezone = new bool[_NumPeds];
     for (int i=0; i<_NumPeds; i++)
     {
          IsinMeasurezone[i] = false;
     }
     _DensityPerFrame = new double[numFrames];
     //Method_C method_C;
     for(int frameNr = 0; frameNr < numFrames; frameNr++ )
     {
          vector<int> ids=_peds_t[frameNr];
          int pedsinMeasureArea=0;
          for(unsigned int i=0; i< ids.size(); i++)
          {
               int ID = ids[i];
               int x = _xCor(ID,frameNr);
               int y = _yCor(ID,frameNr);
               if(within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly))
               {
                    pedsinMeasureArea++;
               }
               if(within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly)&&!(IsinMeasurezone[ID])) {
                    _tIn[ID]=frameNr;
                    IsinMeasurezone[ID] = true;
                    _entrancePoint[ID]._x = x*CMtoM;
                    _entrancePoint[ID]._y = y*CMtoM;
                    std::cout << "ID: "<< ID  << " x: " << x*CMtoM << " y: " << y*CMtoM<< std::endl;
               }
               if((!within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly))&&IsinMeasurezone[ID]) {
                    _tOut[ID]=frameNr;
                    _exitPoint[ID]._x = x*CMtoM;
                    _exitPoint[ID]._y = y*CMtoM;
                    IsinMeasurezone[ID] = false;
                    std::cout <<  "ID: "<< ID  << " OUT x: " << x*CMtoM << " y: " << y*CMtoM << std::endl;
               }
          }
          _DensityPerFrame[frameNr] = pedsinMeasureArea/(area(_areaForMethod_B->_poly)*CMtoM*CMtoM);
     }
     delete []IsinMeasurezone;
}

void Method_B::GetFundamentalTinTout(double *DensityPerFrame,double LengthMeasurementarea)
{

     FILE *fFD_TinTout;
     Log->Write("---------Fundamental diagram from Method B will be calculated!------------------");
     fs::path tmp("_id_"+_measureAreaId+".dat");
     tmp = _outputLocation / "Fundamental_Diagram" / "TinTout" / ("FDTinTout_" + _trajName.string() + tmp.string());
//     string fdTinTout=_outputLocation.string()+"Fundamental_Diagram/TinTout/FDTinTout_"+_trajName+"_id_"+_measureAreaId+".dat";
     string fdTinTout = tmp.string();

     if((fFD_TinTout=Analysis::CreateFile(fdTinTout))==nullptr)
     {
          Log->Write("ERROR:\tcannot open the file to write the TinTout data\n");
          exit(EXIT_FAILURE);
     }
     fprintf(fFD_TinTout,"#person Index\t	density_i(m^(-2))\t	velocity_i(m/s)\n");
     for(int i=0; i<_NumPeds; i++)
     {
          if(LengthMeasurementarea < 0) {
               double dxq = (_entrancePoint[i]._x - _exitPoint[i]._x)*(_entrancePoint[i]._x - _exitPoint[i]._x);
               double dyq = (_entrancePoint[i]._y - _exitPoint[i]._y)*(_entrancePoint[i]._y - _exitPoint[i]._y);
               LengthMeasurementarea = std::sqrt(dxq + dyq);
          }
          std::cout << "i: "<< i << ", Tin: " << _tIn[i] << ", Tout: " << _tOut[i]
                    << ", PointIn (" << _entrancePoint[i]._x << ", " << _entrancePoint[i]._y
                    << "), PointOut (" << _exitPoint[i]._x << ", " << _exitPoint[i]._y
                    << "), L: "<< LengthMeasurementarea<< std::endl;

          double velocity_temp=_fps*LengthMeasurementarea/(_tOut[i]-_tIn[i]);
          std::cout << ">> i: " << i << " vel = " << velocity_temp << std::endl;
          double density_temp=0;
          for(int j=_tIn[i]; j<_tOut[i]; j++)
          {
               density_temp+=DensityPerFrame[j];
          }
          density_temp/=(_tOut[i]-_tIn[i]);
          fprintf(fFD_TinTout,"%d\t%f\t%f\n",i+1,density_temp,velocity_temp);
     }
     fclose(fFD_TinTout);

}

void Method_B::SetMeasurementArea (MeasurementArea_B* area)
{
     _areaForMethod_B = area;
}