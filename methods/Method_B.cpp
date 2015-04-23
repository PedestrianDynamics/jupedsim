/**
 * \file        Method_B.cpp
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J��lich GmbH. All rights reserved.
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
     _xCor = NULL;
     _yCor = NULL;
     _tIn = NULL;
     _tOut = NULL;
     _DensityPerFrame = NULL;
     _fps = 10;
     _NumPeds =0;
     _areaForMethod_B = NULL;
}

Method_B::~Method_B()
{

}

bool Method_B::Process (const PedData& peddata)
{
     _trajName = peddata.GetTrajName();
     _projectRootDir = peddata.GetProjectRootDir();
     _fps =peddata.GetFps();
     _peds_t = peddata.GetPedsFrame();
     _NumPeds = peddata.GetNumPeds();
     _xCor = peddata.GetXCor();
     _yCor = peddata.GetYCor();
     _measureAreaId = boost::lexical_cast<string>(_areaForMethod_B->_id);
     _tIn = new int[_NumPeds];				// Record the time of each pedestrian entering measurement area
     _tOut = new int[_NumPeds];
     for (int i=0; i<_NumPeds; i++)
     {
          _tIn[i] = 0;
          _tOut[i] = 0;
     }
     GetTinTout(peddata.GetNumFrames());
     Log->Write("------------------------Analyzing with Method B-----------------------------");
     if(_areaForMethod_B->_length<0)
     {
          Log->Write("Error:\tThe measurement area length for method B is not assigned!");
          exit(0);
     }
     else
     {
          GetFundamentalTinTout(_DensityPerFrame,_areaForMethod_B->_length);
     }
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
     Method_C method_C;
     for(int frameNr = 0; frameNr < numFrames; frameNr++ )
     {
          vector<int> ids=_peds_t[frameNr];
          int pedsinMeasureArea=0;
          for(unsigned int i=0; i< ids.size(); i++)
          {
               int ID = ids[i];
               int x = _xCor[ID][frameNr];
               int y = _yCor[ID][frameNr];
               if(within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly))
               {
                    pedsinMeasureArea++;
               }
               if(within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly)&&!(IsinMeasurezone[ID])) {
                    _tIn[ID]=frameNr;
                    IsinMeasurezone[ID] = true;
               }
               if((!within(make<point_2d>( (x), (y)), _areaForMethod_B->_poly))&&IsinMeasurezone[ID]) {
                    _tOut[ID]=frameNr;
                    IsinMeasurezone[ID] = false;
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
     string fdTinTout=_projectRootDir+"./Output/Fundamental_Diagram/TinTout/FDTinTout_"+_trajName+"_id_"+_measureAreaId+".dat";;
     if((fFD_TinTout=Analysis::CreateFile(fdTinTout))==NULL)
     {
          Log->Write("cannot open the file to write the TinTout data\n");
          exit(0);
     }
     fprintf(fFD_TinTout,"#person Index\t	density_i(m^(-2))\t	velocity_i(m/s)\n");
     for(int i=0; i<_NumPeds; i++)
     {
          double velocity_temp=_fps*CMtoM*LengthMeasurementarea/(_tOut[i]-_tIn[i]);
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
