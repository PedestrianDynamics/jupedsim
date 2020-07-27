/**
 * \file        PedData.h
 * \date        Oct 10, 2014
 * \version     v0.8.3
 * \copyright   <2009-2018> Forschungszentrum Jülich GmbH. All rights reserved.
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

#ifndef PEDDATA_H_
#define PEDDATA_H_

#include "../general/Macros.h"
#include "../tinyxml/tinyxml.h"

#include <algorithm> // std::min_element, std::max_element
#include <boost/algorithm/string.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <map>
#include <string>
#include <vector>
namespace ub = boost::numeric::ublas;

#include <filesystem>

namespace fs = std::filesystem;


#define CMtoM 0.0001
#define M2CM 10000

class PedData
{
public:
    PedData();
    virtual ~PedData();
    int GetMinFrame() const;
    int GetMinID() const;
    int GetNumFrames() const;
    int GetNumPeds() const;
    float GetFps() const;
    fs::path GetTrajName() const;
    fs::path GetProjectRootDir() const;
    std::map<int, std::vector<int>> GetPedIDsByFrameNr() const;
    ub::matrix<double> GetXCor() const;
    ub::matrix<double> GetYCor() const;
    ub::matrix<double> GetZCor() const;
    ub::matrix<double> GetId() const;
    int * GetFirstFrame() const;
    int * GetLastFrame() const;
    std::vector<int> GetIndexInFrame(int frame, const std::vector<int> & ids, double zPos) const;
    std::vector<int> GetIdInFrame(int frame, const std::vector<int> & ids) const;
    std::vector<int> GetIdInFrame(int frame, const std::vector<int> & ids, double zPos) const;
    std::vector<double> GetXInFrame(int frame, const std::vector<int> & ids, double zPos) const;
    std::vector<double> GetYInFrame(int frame, const std::vector<int> & ids, double zPos) const;
    std::vector<double> GetXInFrame(int frame, const std::vector<int> & ids) const;
    std::vector<double> GetYInFrame(int frame, const std::vector<int> & ids) const;
    std::vector<double> GetZInFrame(int frame, const std::vector<int> & ids) const;
    std::vector<double> GetZInFrame(int frame, const std::vector<int> & ids, double zPos) const;
    std::vector<double> GetVInFrame(int frame, const std::vector<int> & ids, double zPos) const;
    bool ReadData(
        const fs::path & projectRootDir,
        const fs::path & outputDir,
        const fs::path & path,
        const fs::path & filename,
        const FileFormat & _trajformat,
        int deltaF,
        std::string vComponent,
        const bool IgnoreBackwardMovement);
    fs::path GetOutputLocation() const;


private:
    bool InitializeVariables(const fs::path & filename);
    void CreateGlobalVariables(int numPeds, int numFrames);
    double GetInstantaneousVelocity(
        int Tnow,
        int Tpast,
        int Tfuture,
        int ID,
        int * Tfirst,
        int * Tlast,
        const ub::matrix<double> & Xcor,
        const ub::matrix<double> & Ycor) const;
    double GetInstantaneousVelocity1(
        int Tnow,
        int Tpast,
        int Tfuture,
        int ID,
        int * Tfirst,
        int * Tlast,
        const ub::matrix<double> & Xcor,
        const ub::matrix<double> & Ycor) const;

private:
    fs::path _trajName;
    fs::path _projectRootDir;
    fs::path _outputLocation = "";
    int _minFrame            = 0;
    int _minID               = 1;
    int _maxID               = 0;
    int _numFrames           = 0; // total number of frames
    int _numPeds             = 0; // total number of pedestrians
    float _fps               = 16;
    std::map<int, std::vector<int>> _pedIDsByFrameNr;

    int _deltaF                  = 5;
    std::string _vComponent      = "B";
    bool _IgnoreBackwardMovement = false;

    int * _firstFrame = nullptr; // Record the first frame of each pedestrian
    int * _lastFrame  = nullptr; // Record the last frame of each pedestrian
    ub::matrix<double> _xCor;
    ub::matrix<double> _yCor;
    ub::matrix<double> _zCor;
    ub::matrix<double> _id;
    ub::matrix<std::string> _vComp;
};

#endif /* PEDDATA_H_ */
