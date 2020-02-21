/**
 * \file        Method_A.cpp
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * In this file functions related to method A are defined.
 *
 *
 **/

#include "Method_A.h"

#include "../Analysis.h"

#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_A::Method_A()
{
    _classicFlow     = 0;
    _vDeltaT         = 0;
    _firstFrame      = nullptr;
    _passLine        = nullptr;
    _deltaT          = 100;
    _fps             = 16;
    _areaForMethod_A = nullptr;
}

Method_A::~Method_A() {}

bool Method_A::Process(
    const PedData & peddata,
    const fs::path & scriptsLocation,
    const double & zPos_measureArea)
{
    _trajName        = peddata.GetTrajName();
    _projectRootDir  = peddata.GetProjectRootDir();
    _scriptsLocation = scriptsLocation;
    _outputLocation  = peddata.GetOutputLocation();
    _peds_t          = peddata.GetPedsFrame();
    _xCor            = peddata.GetXCor();
    _yCor            = peddata.GetYCor();
    _firstFrame      = peddata.GetFirstFrame();
    _fps             = peddata.GetFps();
    _measureAreaId   = boost::lexical_cast<string>(_areaForMethod_A->_id);
    _passLine        = new bool[peddata.GetNumPeds()];
    string outputRhoV;
    outputRhoV.append("#Time [s]\t	Cumulative pedestrians\n");
    for(int i = 0; i < peddata.GetNumPeds(); i++) {
        _passLine[i] = false;
    }
    Log->Write("------------------------Analyzing with Method A-----------------------------");
    bool PedInGeometry = false;
    for(std::map<int, std::vector<int>>::iterator ite = _peds_t.begin(); ite != _peds_t.end();
        ite++) {
        int frameNr = ite->first;                      //index starts by 0
        int frid    = frameNr + peddata.GetMinFrame(); // frame in traj file
        if(!(frid % 100)) {
            Log->Write("frame ID = %d", frid);
        }
        vector<int> ids = peddata.GetIndexInFrame(frameNr, _peds_t[frameNr], zPos_measureArea);
        const vector<double> VInFrame = peddata.GetVInFrame(frameNr, ids, zPos_measureArea);
        if(VInFrame.size() > 0) {
            GetAccumFlowVelocity(frameNr, ids, VInFrame);
            char tmp[30];
            sprintf(tmp, "%.2f\t%d\n", frid / _fps, _classicFlow);
            outputRhoV.append(tmp);
            PedInGeometry = true;
        }
    }
    if(PedInGeometry) {
        FlowRate_Velocity(peddata.GetFps(), _accumPedsPassLine, _accumVPassLine);
        WriteFile_N_t(outputRhoV);
    } else {
        Log->Write("Warning: No pedestrian exists on the plane of the selected Measurement area!!");
    }
    delete[] _passLine;
    return true;
}

void Method_A::WriteFile_N_t(string data)
{
    fs::path tmp = ("_id_" + _measureAreaId + ".dat");
    fs::path FD_FlowVelocity("Fundamental_Diagram/FlowVelocity");
    tmp = _outputLocation / FD_FlowVelocity / ("Flow_NT_" + _trajName.string() + tmp.string());
    // _outputLocation.string()+"Fundamental_Diagram/FlowVelocity/Flow_NT_"+_trajName+"_id_"+_measureAreaId+".dat";
    string fN_t = tmp.string();
    ofstream file(fN_t);
    if(file.is_open()) {
        file << data;
        file.close();
        fs::path tmp2 = _outputLocation / FD_FlowVelocity;
        // _outputLocation.string()+"Fundamental_Diagram/FlowVelocity/";
        string METHOD_A_LOCATION = tmp2.string();
        //string file_N_t ="Flow_NT_"+_trajName+"_id_"+_measureAreaId+".dat";
        string file_N_t = fN_t; //@todo: this is redundant
    } else {
        Log->Write("ERROR: could not create the file " + fN_t);
    }
}

void Method_A::GetAccumFlowVelocity(
    int frame,
    const vector<int> & ids,
    const vector<double> & VInFrame)
{
    for(auto const i : ids) {
        bool IspassLine = false;
        if(frame > _firstFrame[i] && !_passLine[i]) {
            IspassLine = IsPassLine(
                _areaForMethod_A->_lineStartX,
                _areaForMethod_A->_lineStartY,
                _areaForMethod_A->_lineEndX,
                _areaForMethod_A->_lineEndY,
                _xCor(i, frame - 1),
                _yCor(i, frame - 1),
                _xCor(i, frame),
                _yCor(i, frame));
        }
        if(IspassLine == true) {
            _passLine[i] = true;
            _classicFlow++;
            _vDeltaT += VInFrame[i];
        }
    }
    _accumPedsPassLine.push_back(_classicFlow);
    _accumVPassLine.push_back(_vDeltaT);
}

bool Method_A::IsPassLine(
    double Line_startX,
    double Line_startY,
    double Line_endX,
    double Line_endY,
    double pt1_X,
    double pt1_Y,
    double pt2_X,
    double pt2_Y)
{
    point_2d Line_pt0(Line_startX, Line_startY);
    point_2d Line_pt1(Line_endX, Line_endY);
    segment edge0(Line_pt0, Line_pt1);

    point_2d Traj_pt0(pt1_X, pt1_Y);
    point_2d Traj_pt1(pt2_X, pt2_Y);
    segment edge1(Traj_pt0, Traj_pt1);

    return (intersects(edge0, edge1));
}

void Method_A::FlowRate_Velocity(
    int fps,
    const vector<int> & AccumPeds,
    const vector<double> & AccumVelocity)
{
    FILE * fFD_FlowVelocity;
    fs::path tmp("_id_" + _measureAreaId + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "FlowVelocity" /
          ("FDFlowVelocity_" + _trajName.string() + tmp.string());
    string fdFlowVelocity = tmp.string();

    if((fFD_FlowVelocity = Analysis::CreateFile(fdFlowVelocity)) == nullptr) {
        Log->Write("cannot open the file to write the Flow-Velocity data\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fFD_FlowVelocity, "#Flow rate(1/s)	\t Mean velocity(m/s)\n");
    int TotalTime = AccumPeds.size();         // the total Frame of in the data file
    int TotalPeds = AccumPeds[TotalTime - 1]; //the total pedestrians in the data file
    if(TotalPeds > 0) {
        int firstPassT = -1; // the first time that there are pedestrians pass the line
        int * pedspassT =
            new int[TotalPeds + 1]; // the time for certain pedestrian passing the line
        for(int i = 0; i <= TotalPeds; i++) {
            pedspassT[i] = -1;
        }

        for(int ix = 0; ix < TotalTime; ix++) {
            if(AccumPeds[ix] > 0 && firstPassT < 0) {
                firstPassT = ix;
            }
            if(pedspassT[AccumPeds[ix]] < 0) {
                pedspassT[AccumPeds[ix]] = ix;
            }
        }
        for(int i = firstPassT + _deltaT; i < TotalTime; i += _deltaT) {
            int N1 = AccumPeds
                [i - _deltaT]; // the total number of pedestrians pass the line at this time
            int N2   = AccumPeds[i];
            int t_N1 = pedspassT[N1];
            int t_N2 = pedspassT[N2];
            if(N1 != N2) {
                double flow_rate = fps * (N2 - N1) * 1.00 / (t_N2 - t_N1);
                double MeanV     = (AccumVelocity[i] - AccumVelocity[i - _deltaT]) /
                               (AccumPeds[i] - AccumPeds[i - _deltaT]);
                fprintf(fFD_FlowVelocity, "%.3f\t%.3f\n", flow_rate, MeanV);
            }
        }
        fclose(fFD_FlowVelocity);
        delete[] pedspassT;
    } else {
        Log->Write("INFO:\tNo person passing the reference line given by Method A!\n");
    }
}

void Method_A::SetMeasurementArea(MeasurementArea_L * area)
{
    _areaForMethod_A = area;
}

void Method_A::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
