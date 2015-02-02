/**
 * \file        Method_C.cpp
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
 * In this file functions related to method C are defined.
 *
 *
 **/

#include "Method_C.h"


using std::string;
using std::vector;


Method_C::Method_C()
{
     _minFrame = 0;
     _fClassicRhoV = NULL;
     _areaForMethod_C = NULL;
}

Method_C::~Method_C()
{

}

bool Method_C::Process (const PedData& peddata)
{
     _peds_t = peddata.GetPedsFrame();
     _minFrame = peddata.GetMinFrame();
     _trajName = peddata.GetTrajName();
     _projectRootDir = peddata.GetProjectRootDir();
     _measureAreaId = boost::lexical_cast<string>(_areaForMethod_C->_id);
     OpenFileMethodC();
     Log->Write("------------------------Analyzing with Method C-----------------------------");
     for(int frameNr = 0; frameNr < peddata.GetNumFrames(); frameNr++ )
     {
          int frid =  frameNr + _minFrame;
          if(!(frid%100))
          {
               Log->Write("frame ID = %d",frid);
          }
          vector<int> ids=_peds_t[frameNr];
          const vector<double> XInFrame = peddata.GetXInFrame(frameNr, ids);
          const vector<double> YInFrame = peddata.GetYInFrame(frameNr, ids);
          const vector<double> VInFrame = peddata.GetVInFrame(frameNr, ids);
          OutputClassicalResults(frameNr, ids.size(),XInFrame,YInFrame,VInFrame);
     }
     fclose(_fClassicRhoV);
     return true;
}

void Method_C::OpenFileMethodC()
{
     string results_C= _projectRootDir+"./Output/Fundamental_Diagram/Classical_Voronoi/rho_v_Classic_"+_trajName+"_id_"+_measureAreaId+".dat";
     if((_fClassicRhoV=Analysis::CreateFile(results_C))==NULL) {
          Log->Write("Warning:\tcannot open file %s to write classical density and velocity\n", results_C.c_str());
          exit(EXIT_FAILURE);
     }
     fprintf(_fClassicRhoV,"#Frame \tclassical density(m^(-2))\t	classical velocity(m/s)\n");
}

void Method_C::OutputClassicalResults(int frmNr, int numPedsInFrame,const vector<double>& XInFrame,const vector<double>& YInFrame,const vector<double>& VInFrame) const
{
     int frmId = frmNr+ _minFrame;
     double ClassicDensity = GetClassicalDensity(XInFrame, YInFrame, numPedsInFrame, _areaForMethod_C->_poly);
     double ClassicVelocity = GetClassicalVelocity(XInFrame, YInFrame, VInFrame, numPedsInFrame);
     fprintf(_fClassicRhoV,"%d\t%.3f\t%.3f\n", frmId, ClassicDensity,ClassicVelocity);
}

double Method_C::GetClassicalDensity(const vector<double>& xs, const vector<double>& ys, int pednum, polygon_2d measurearea ) const
{
     int pedsinMeasureArea=0;
     for(int i=0; i<pednum; i++) {
          if(within(make<point_2d>(xs[i], ys[i]), measurearea)) {
               pedsinMeasureArea++;
          }
     }

     return pedsinMeasureArea/(area(_areaForMethod_C->_poly)*CMtoM*CMtoM);
}

double Method_C::GetClassicalVelocity(const vector<double>& xs, const vector<double>& ys, const vector<double>& VInFrame, int pednum) const
{
     int pedsinMeasureArea=0;
     double velocity = 0;
     for(int i=0; i<pednum; i++)
     {
          if(within(make<point_2d>(xs[i], ys[i]), _areaForMethod_C->_poly))
          {
               velocity+=VInFrame[i];
               pedsinMeasureArea++;
          }
     }
     if(pedsinMeasureArea!=0)
     {
          velocity /= (1.0*pedsinMeasureArea);
     } else {
          velocity = 0;
     }
     return velocity;

}

void Method_C::SetMeasurementArea (MeasurementArea_B* area)
{
     _areaForMethod_C = area;
}

