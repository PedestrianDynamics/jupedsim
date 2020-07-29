/**
 * \file        PedData.cpp
 * \date        Feb 10, 2016
 * \version     v0.8
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
 * In this file functions related to reading data from files are defined.
 *
 *
 **/

#include "PedData.h"

#include <Logger.h>
#include <cmath>
#include <fstream>
#include <string>
#include <unordered_set>

using std::ifstream;
using std::map;
using std::string;
using std::vector;

PedData::PedData() {}

PedData::~PedData() {}
bool PedData::ReadData(
    const fs::path & projectRootDir,
    const fs::path & outputLocation,
    const fs::path & path,
    const fs::path & filename,
    const FileFormat & trajformat,
    int deltaF,
    std::string vComponent,
    const bool IgnoreBackwardMovement)
{
    _minID                  = INT_MAX;
    _maxID                  = INT_MAX;
    _minFrame               = INT_MAX;
    _deltaF                 = deltaF;
    _vComponent             = vComponent;
    _IgnoreBackwardMovement = IgnoreBackwardMovement;
    _projectRootDir         = projectRootDir;
    _outputLocation         = outputLocation;
    _trajName               = filename;
    fs::path p(path);
    p /= _trajName;
    fs::path fullTrajectoriesPathName = path / _trajName;
    LOG_INFO("the name of the trajectory is: <{}>", _trajName.string());
    LOG_INFO("full name of the trajectory is: <{}>", fullTrajectoriesPathName.string());
    bool result = true;
    if(trajformat == FORMAT_XML_PLAIN) {
        LOG_WARNING("Input trajectory file for jpsreport should be in txt format");
    }

    else if(trajformat == FORMAT_PLAIN) {
        result = InitializeVariables(fullTrajectoriesPathName);
    }
    return result;
}

// init _xCor, _yCor and _zCor
bool PedData::InitializeVariables(const fs::path & filename)
{
    vector<double> xs;
    vector<double> ys;
    vector<double> zs;
    vector<string> vcmp;    // the direction identification for velocity calculation
    vector<int> _IdsTXT;    // the Id data from txt format trajectory data
    vector<int> _FramesTXT; // the Frame data from txt format trajectory data
    //string fullTrajectoriesPathName= _projectRootDir+"./"+_trajName;
    ifstream fdata;
    fdata.open(filename.string());
    if(fdata.is_open() == false) {
        LOG_ERROR("could not open the trajectories file <{}>", filename.string());
        return false;
    } else {
        string line;
        int lineNr    = 1;
        int pos_id    = 0;
        int pos_fr    = 1;
        int pos_x     = 2;
        int pos_y     = 3;
        int pos_z     = 4;
        int pos_vd    = 5; //velocity direction
        int fps_found = 0;
        while(getline(fdata, line)) {
            boost::algorithm::trim(line);
            //looking for the framerate which is supposed to be at the second position
            if(line[0] == '#') {
                if(line.find("framerate") != std::string::npos) {
                    std::vector<std::string> strs;
                    line.erase(0, 1); // remove #
                    boost::split(strs, line, boost::is_any_of(":"), boost::token_compress_on);
                    if(strs.size() > 1) {
                        _fps = atof(strs[1].c_str());
                        if(_fps == 0.0) // in case not valid fps found
                        {
                            LOG_ERROR("Could not convert fps <{}>", strs[1]);
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        LOG_ERROR("Frame rate fps not defined");
                        exit(EXIT_FAILURE);
                    }
                    LOG_INFO("Frame rate fps: <{:.2f}>", _fps);
                    fps_found = 1;
                }
                if(line.find("ID") != std::string::npos && line.find("FR") != std::string::npos &&
                   line.find("X") != std::string::npos && line.find("Y") != std::string::npos &&
                   line.find("Z") != std::string::npos) {
                    // looking for this line
                    // #ID  FR  X Y Z
                    //std::cout << "0. line <" << line << ">\n";
                    std::vector<std::string> strs1;
                    line.erase(0, 1);
                    //std::cout << "1. line <" << line << ">\n";
                    boost::split(strs1, line, boost::is_any_of("\t\r "), boost::token_compress_on);
                    // std::cout << "str size = " << strs1.size() << "\n";
                    // for(auto s: strs1)
                    //      std::cout << "s <" << s  << ">\n";

                    vector<string>::iterator it_id;
                    it_id  = find(strs1.begin(), strs1.end(), "ID");
                    pos_id = std::distance(strs1.begin(), it_id);
                    it_id  = find(strs1.begin(), strs1.end(), "FR");
                    pos_fr = std::distance(strs1.begin(), it_id);
                    it_id  = find(strs1.begin(), strs1.end(), "X");
                    pos_x  = std::distance(strs1.begin(), it_id);
                    it_id  = find(strs1.begin(), strs1.end(), "Y");
                    pos_y  = std::distance(strs1.begin(), it_id);
                    it_id  = find(strs1.begin(), strs1.end(), "Z");
                    pos_z  = std::distance(strs1.begin(), it_id);
                    it_id  = find(strs1.begin(), strs1.end(), "VD");
                    pos_vd = std::distance(strs1.begin(), it_id);
                }

            } else if(line[0] != '#' && !(line.empty())) {
                static int once = 1;
                std::vector<std::string> strs;
                boost::split(strs, line, boost::is_any_of("\t "), boost::token_compress_on);
                if(lineNr % 100000 == 0)
                    LOG_INFO("lineNr {}", lineNr);

                if(once) // && strs.size() < 5
                {
                    once = 0;
                    LOG_INFO("pos_id: {}", pos_id);
                    LOG_INFO("pos_fr: {}", pos_fr);
                    LOG_INFO("pos_x: {}", pos_x);
                    LOG_INFO("pos_y: {}", pos_y);
                    LOG_INFO("pos_z: {}", pos_z);
                    LOG_INFO("pos_vd: {}", pos_vd);
                }
                _IdsTXT.push_back(atoi(strs[pos_id].c_str()));
                _FramesTXT.push_back(atoi(strs[pos_fr].c_str()));
                xs.push_back(atof(strs[pos_x].c_str()));
                ys.push_back(atof(strs[pos_y].c_str()));

                if(strs.size() >= 5)
                    zs.push_back(atof(strs[pos_z].c_str()));
                else
                    zs.push_back(0);

                if(_vComponent == "F") {
                    if(strs.size() >= 6 && pos_vd < (int) strs.size()) {
                        vcmp.push_back(strs[pos_vd].c_str());
                    } else {
                        LOG_ERROR("There is no indicator for velocity component in "
                                  "trajectory file or ini file!!");
                        return false;
                    }
                }
            }
            lineNr++;
        } // while
        if(fps_found == 0) {
            LOG_ERROR("Frame rate fps ia not defined ");
            exit(EXIT_FAILURE);
        }
        LOG_INFO("Finished reading the data");
    }

    fdata.close();
    LOG_INFO("Got {} lines", _IdsTXT.size());
    _minID = *min_element(_IdsTXT.begin(), _IdsTXT.end());
    _maxID = *max_element(_IdsTXT.begin(), _IdsTXT.end());
    LOG_INFO("minID: {}", _minID);
    LOG_INFO("maxID: {}", _maxID);
    _minFrame = *min_element(_FramesTXT.begin(), _FramesTXT.end());
    LOG_INFO("minFrame: {}", _minFrame);
    //Total number of frames
    _numFrames = *max_element(_FramesTXT.begin(), _FramesTXT.end()) - _minFrame + 1;
    LOG_INFO("numFrames: {}", _numFrames);

    //Total number of agents

    std::unordered_set<int> s;
    std::vector<int> unique_ids(_IdsTXT);
    auto end = std::remove_if(
        unique_ids.begin(), unique_ids.end(), [&s](int const & i) { return !s.insert(i).second; });

    unique_ids.erase(end, unique_ids.end());
    _numPeds = unique_ids.size();
    LOG_INFO("INFO: Total number of Agents: {}", _numPeds);
    CreateGlobalVariables(_numPeds, _numFrames);
    LOG_INFO("Create Global Variables done");
    for(int i = 0; i < (int) unique_ids.size(); i++) {
        int firstFrameIndex   = INT_MAX; //The first frame index of a pedestrian
        int lastFrameIndex    = -1;      //The last frame index of a pedestrian
        int actual_totalframe = 0;       //The total data points of a pedestrian in the trajectory
        int pos_i             = i;       //std::distance(_IdsTXT.begin(), &i);
        for(auto j = _IdsTXT.begin(); j != _IdsTXT.end(); ++j) {
            if(*j == unique_ids[i]) {
                int pos = std::distance(_IdsTXT.begin(), j);
                if(pos < firstFrameIndex) {
                    firstFrameIndex = pos;
                }
                if(pos > lastFrameIndex) {
                    lastFrameIndex = pos;
                }
                actual_totalframe++;
            }
        }
        if(lastFrameIndex <= 0 || firstFrameIndex == INT_MAX) {
            LOG_WARNING("There is no trajectory for ped with ID <{}>!", unique_ids[i]);
            continue;
        }
        _firstFrame[pos_i]    = _FramesTXT[firstFrameIndex] - _minFrame;
        _lastFrame[pos_i]     = _FramesTXT[lastFrameIndex] - _minFrame;
        int expect_totalframe = _lastFrame[pos_i] - _firstFrame[pos_i] + 1;
        if(actual_totalframe != expect_totalframe) {
            LOG_ERROR(
                "The trajectory of ped with ID <{}> is not continuous. Please modify the "
                "trajectory file!",
                _IdsTXT[pos_i]);
            LOG_ERROR(
                "actual_totalfame = <{}>, expected_totalframe = <{}>",
                actual_totalframe,
                expect_totalframe);
            return false;
        }
    }
    LOG_INFO("convert x and y");
    for(unsigned int i = 0; i < _IdsTXT.size(); i++) {
        int id_pos = 0; // position in array unique_ids
        //---------- get position of index in unique index vector ---------------
        auto it_uid = std::find(unique_ids.begin(), unique_ids.end(), _IdsTXT[i]);
        if(it_uid == unique_ids.end()) {
            LOG_ERROR("Id {} does not exist in file", _IdsTXT[i]);
            return false;
        } else {
            id_pos = std::distance(unique_ids.begin(), it_uid);
        }
        //--------------------
        int frm  = _FramesTXT[i] - _minFrame;
        double x = xs[i] * M2CM;
        double y = ys[i] * M2CM;
        double z = zs[i] * M2CM;

        /* structure of these matrices
           * line:  position id in unique_ids
           * column: frame id - minFrame
           */
        _xCor(id_pos, frm) = x;
        _yCor(id_pos, frm) = y;
        _zCor(id_pos, frm) = z;
        _id(id_pos, frm)   = _IdsTXT[i];
        if(_vComponent == "F") {
            _vComp(id_pos, frm) = vcmp[i];
        } else {
            _vComp(id_pos, frm) = _vComponent;
        }
    }
    LOG_INFO("Save the data for each frame");

    //save the data for each frame
    for(unsigned int i = 0; i < _FramesTXT.size(); i++) {
        int id_pos = 0;
        auto itIds = std::find(unique_ids.begin(), unique_ids.end(), _IdsTXT[i]);
        if(itIds == unique_ids.end()) {
            LOG_ERROR("Id {} does not exist in file", _IdsTXT[i]);
            return false;
        } else {
            id_pos = std::distance(unique_ids.begin(), itIds);
        }
        int t = _FramesTXT[i] - _minFrame;
        /* structure of peds_t
           *
           * index: frame id - minFrame, value: position id in unique_ids
           */

        _pedIDsByFrameNr[t].push_back(id_pos);
        // std::cout << "frame: " << _FramesTXT[i] << " t: " << t << " > " << id_pos << "\n";
    }

    return true;
}

vector<double> PedData::GetVInFrame(int frame, const vector<int> & ids, double zPos) const
{
    vector<double> VInFrame;
    for(unsigned int i = 0; i < ids.size(); i++) {
        int id      = ids[i];
        int Tpast   = frame - _deltaF;
        int Tfuture = frame + _deltaF;
        double v    = GetInstantaneousVelocity1(
            frame, Tpast, Tfuture, id, _firstFrame, _lastFrame, _xCor, _yCor);
        // TODO: zPos is set to 10000001.0 if it's None. Needed for this if-clause. But why?
        if(zPos < 1000000.0) {
            if(fabs(_zCor(id, frame) - zPos * M2CM) < J_EPS_EVENT) {
                VInFrame.push_back(v);
            }
        } else {
            VInFrame.push_back(v);
        }
    }
    return VInFrame;
}

vector<double> PedData::GetXInFrame(int frame, const vector<int> & ids, double zPos) const
{
    vector<double> XInFrame;
    for(int id : ids) {
        // TODO: zPos is set to 10000001.0 if it's None. Needed for this if-clause. But why?
        if(zPos < 1000000.0) {
            if(fabs(_zCor(id, frame) - zPos * M2CM) < J_EPS_EVENT) {
                XInFrame.push_back(_xCor(id, frame));
            }
        } else {
            XInFrame.push_back(_xCor(id, frame));
        }
    }
    return XInFrame;
}

vector<double> PedData::GetXInFrame(int frame, const vector<int> & ids) const
{
    vector<double> XInFrame;
    for(int id : ids) {
        XInFrame.push_back(_xCor(id, frame));
    }
    return XInFrame;
}

vector<double> PedData::GetYInFrame(int frame, const vector<int> & ids, double zPos) const
{
    vector<double> YInFrame;
    for(unsigned int i = 0; i < ids.size(); i++) {
        int id = ids[i];
        // TODO: zPos is set to 10000001.0 if it's None. Needed for this if-clause. But why?
        if(zPos < 1000000.0) {
            if(fabs(_zCor(id, frame) - zPos * M2CM) < J_EPS_EVENT) {
                YInFrame.push_back(_yCor(id, frame));
            }
        } else {
            YInFrame.push_back(_yCor(id, frame));
        }
    }
    return YInFrame;
}

vector<double> PedData::GetYInFrame(int frame, const vector<int> & ids) const
{
    vector<double> YInFrame;
    for(unsigned int i = 0; i < ids.size(); i++) {
        int id = ids[i];
        YInFrame.push_back(_yCor(id, frame));
    }
    return YInFrame;
}

vector<double> PedData::GetZInFrame(int frame, const vector<int> & ids) const
{
    vector<double> ZInFrame;
    for(unsigned int i = 0; i < ids.size(); i++) {
        int id = ids[i];
        ZInFrame.push_back(_zCor(id, frame));
    }
    return ZInFrame;
}
vector<double> PedData::GetZInFrame(int frame, const vector<int> & ids, double zPos) const
{
    vector<double> ZInFrame;
    for(unsigned int i = 0; i < ids.size(); i++) {
        int id = ids[i];
        // TODO: zPos is set to 10000001.0 if it's None. Needed for this if-clause. But why?
        if(zPos < 1000000.0) {
            if(fabs(_zCor(id, frame) - zPos * M2CM) < J_EPS_EVENT) {
                ZInFrame.push_back(_zCor(id, frame));
            }
        } else {
            ZInFrame.push_back(_zCor(id, frame));
        }
    }
    return ZInFrame;
}

vector<int> PedData::GetIdInFrame(int frame, const vector<int> & ids) const
{
    vector<int> IdInFrame;
    for(int id : ids) {
        IdInFrame.push_back(_id(id, frame));
    }
    return IdInFrame;
}

vector<int> PedData::GetIndexInFrame(int frame, const vector<int> & ids, double zPos) const
{
    vector<int> IdInFrame;
    for(int id : ids) {
        // TODO: zPos is set to 10000001.0 if it's None. Needed for this if-clause. But why?
        if(zPos < 1000000.0) {
            if(fabs(_zCor(id, frame) - zPos * M2CM) < J_EPS_EVENT) {
                IdInFrame.push_back(id);
            }
        } else {
            IdInFrame.push_back(id);
        }
    }
    return IdInFrame;
}

vector<int> PedData::GetIdInFrame(int frame, const vector<int> & ids, double zPos) const
{
    vector<int> IdInFrame;
    for(int id : ids) {
        // TODO: zPos is set to 10000001.0 if it's None. Needed for this if-clause. But why?
        if(zPos < 1000000.0) {
            if(fabs(_zCor(id, frame) - zPos * M2CM) < J_EPS_EVENT) {
                //IdInFrame.push_back(id +_minID);
                IdInFrame.push_back(_id(id, frame));
            }
        } else {
            // IdInFrame.push_back(id +_minID);
            IdInFrame.push_back(_id(id, frame));
        }
    }
    return IdInFrame;
}

double PedData::GetInstantaneousVelocity(
    int Tnow,
    int Tpast,
    int Tfuture,
    int ID,
    int * Tfirst,
    int * Tlast,
    const ub::matrix<double> & Xcor,
    const ub::matrix<double> & Ycor) const
{
    std::string vcmp = _vComp(ID, Tnow);
    double v         = 0.0;
    //check the component used in the calculation of velocity
    if(vcmp == "X" || vcmp == "X+" || vcmp == "X-") {
        if((Tpast >= Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM * (Xcor(ID, Tfuture) - Xcor(ID, Tpast)) /
                (2.0 * _deltaF); //one dimensional velocity
        } else if((Tpast < Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM * (Xcor(ID, Tfuture) - Xcor(ID, Tnow)) /
                (_deltaF); //one dimensional velocity
        } else if((Tpast >= Tfirst[ID]) && (Tfuture > Tlast[ID])) {
            v = _fps * CMtoM * (Xcor(ID, Tnow) - Xcor(ID, Tpast)) /
                (_deltaF); //one dimensional velocity
        }
        if((vcmp == "X+" && v < 0) || (vcmp == "X-" && v > 0)) //no moveback
            v = 0;
    } else if(vcmp == "Y" || vcmp == "Y+" || vcmp == "Y-") {
        if((Tpast >= Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM * (Ycor(ID, Tfuture) - Ycor(ID, Tpast)) /
                (2.0 * _deltaF); //one dimensional velocity
        } else if((Tpast < Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM * (Ycor(ID, Tfuture) - Ycor(ID, Tnow)) /
                (_deltaF); //one dimensional velocity
        } else if((Tpast >= Tfirst[ID]) && (Tfuture > Tlast[ID])) {
            v = _fps * CMtoM * (Ycor(ID, Tnow) - Ycor(ID, Tpast)) /
                (_deltaF); //one dimensional velocity
        }
        if((vcmp == "Y+" && v < 0) || (vcmp == "Y-" && v > 0)) //no moveback
            v = 0;
    } else if(vcmp == "B") {
        if((Tpast >= Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM *
                sqrt(
                    pow((Xcor(ID, Tfuture) - Xcor(ID, Tpast)), 2) +
                    pow((Ycor(ID, Tfuture) - Ycor(ID, Tpast)), 2)) /
                (2.0 * _deltaF); //two dimensional velocity
        } else if((Tpast < Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM *
                sqrt(
                    pow((Xcor(ID, Tfuture) - Xcor(ID, Tnow)), 2) +
                    pow((Ycor(ID, Tfuture) - Ycor(ID, Tnow)), 2)) /
                (_deltaF);
        } else if((Tpast >= Tfirst[ID]) && (Tfuture > Tlast[ID])) {
            v = _fps * CMtoM *
                sqrt(
                    pow((Xcor(ID, Tnow) - Xcor(ID, Tpast)), 2) +
                    pow((Ycor(ID, Tnow) - Ycor(ID, Tpast)), 2)) /
                (_deltaF); //two dimensional velocity
        }
    }

    return fabs(v);
}

double PedData::GetInstantaneousVelocity1(
    int Tnow,
    int Tpast,
    int Tfuture,
    int ID,
    int * Tfirst,
    int * Tlast,
    const ub::matrix<double> & Xcor,
    const ub::matrix<double> & Ycor) const
{
    std::string vcmp = _vComp(ID, Tnow); // the vcmp is the angle from 0 to 360
    if(vcmp == "X+") {
        vcmp = "0";
    } else if(vcmp == "Y+") {
        vcmp = "90";
    }
    if(vcmp == "X-") {
        vcmp = "180";
    }
    if(vcmp == "Y-") {
        vcmp = "270";
    }
    double v = 0.0;
    if(vcmp != "B") //check the component used in the calculation of velocity
    {
        float alpha = atof(vcmp.c_str()) * 2 * M_PI / 360.0;

        if((Tpast >= Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM *
                ((Xcor(ID, Tfuture) - Xcor(ID, Tpast)) * cos(alpha) +
                 (Ycor(ID, Tfuture) - Ycor(ID, Tpast)) * sin(alpha)) /
                (2.0 * _deltaF);
        } else if((Tpast < Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM *
                ((Xcor(ID, Tfuture) - Xcor(ID, Tnow)) * cos(alpha) +
                 (Ycor(ID, Tfuture) - Ycor(ID, Tnow)) * sin(alpha)) /
                (_deltaF); //one dimensional velocity
        } else if((Tpast >= Tfirst[ID]) && (Tfuture > Tlast[ID])) {
            v = _fps * CMtoM *
                ((Xcor(ID, Tnow) - Xcor(ID, Tpast)) * cos(alpha) +
                 (Ycor(ID, Tnow) - Ycor(ID, Tpast)) * sin(alpha)) /
                (_deltaF); //one dimensional velocity
        }
        if(_IgnoreBackwardMovement &&
           v < 0) //if no move back and pedestrian moves back, his velocity is set as 0;
        {
            v = 0;
        }

    } else if(vcmp == "B") {
        if((Tpast >= Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM *
                sqrt(
                    pow((Xcor(ID, Tfuture) - Xcor(ID, Tpast)), 2) +
                    pow((Ycor(ID, Tfuture) - Ycor(ID, Tpast)), 2)) /
                (2.0 * _deltaF); //two dimensional velocity
        } else if((Tpast < Tfirst[ID]) && (Tfuture <= Tlast[ID])) {
            v = _fps * CMtoM *
                sqrt(
                    pow((Xcor(ID, Tfuture) - Xcor(ID, Tnow)), 2) +
                    pow((Ycor(ID, Tfuture) - Ycor(ID, Tnow)), 2)) /
                (_deltaF);
        } else if((Tpast >= Tfirst[ID]) && (Tfuture > Tlast[ID])) {
            v = _fps * CMtoM *
                sqrt(
                    pow((Xcor(ID, Tnow) - Xcor(ID, Tpast)), 2) +
                    pow((Ycor(ID, Tnow) - Ycor(ID, Tpast)), 2)) /
                (_deltaF); //two dimensional velocity
        }
    }

    return v;
}

void PedData::CreateGlobalVariables(int numPeds, int numFrames)
{
    LOG_INFO("Enter CreateGlobalVariables with numPeds={} and numFrames={}", numPeds, numFrames);
    LOG_INFO("allocate memory for xCor");
    _xCor = ub::matrix<double>(numPeds, numFrames);
    LOG_INFO("allocate memory for yCor");
    _yCor = ub::matrix<double>(numPeds, numFrames);
    LOG_INFO("allocate memory for zCor");
    _zCor = ub::matrix<double>(numPeds, numFrames);
    LOG_INFO("allocate memory for index");
    _id = ub::matrix<double>(numPeds, numFrames);
    LOG_INFO("allocate memory for vComp");
    _vComp = ub::matrix<std::string>(numPeds, numFrames);
    LOG_INFO("Finished memory allocation");
    _firstFrame = new int[numPeds]; // Record the first frame of each pedestrian
    _lastFrame  = new int[numPeds]; // Record the last frame of each pedestrian
    for(int i = 0; i < numPeds; i++) {
        for(int j = 0; j < numFrames; j++) {
            _xCor(i, j)  = 0;
            _yCor(i, j)  = 0;
            _zCor(i, j)  = 0;
            _vComp(i, j) = "B";
        }
        _firstFrame[i] = INT_MAX;
        _lastFrame[i]  = INT_MIN;
    }
    LOG_INFO("Leave CreateGlobalVariables()");
}


int PedData::GetMinFrame() const
{
    return _minFrame;
}

int PedData::GetMinID() const
{
    return _minID;
}

int PedData::GetNumFrames() const
{
    return _numFrames;
}

int PedData::GetNumPeds() const
{
    return _numPeds;
}

float PedData::GetFps() const
{
    return _fps;
}

fs::path PedData::GetTrajName() const
{
    return _trajName;
}

map<int, vector<int>> PedData::GetPedIDsByFrameNr() const
{
    return _pedIDsByFrameNr;
}

ub::matrix<double> PedData::GetXCor() const
{
    return _xCor;
}
ub::matrix<double> PedData::GetYCor() const
{
    return _yCor;
}

ub::matrix<double> PedData::GetZCor() const
{
    return _zCor;
}

int * PedData::GetFirstFrame() const
{
    return _firstFrame;
}
int * PedData::GetLastFrame() const
{
    return _lastFrame;
}

fs::path PedData::GetProjectRootDir() const
{
    return _projectRootDir;
}

fs::path PedData::GetOutputLocation() const
{
    return _outputLocation;
}
