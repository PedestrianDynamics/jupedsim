/**
 * \file        StartDistribution
 * \date        Apr 15, 2015
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
 *
 *
 **/
#include "StartDistribution.h"

#include "AgentsParameters.h"
#include "Pedestrian.h"
#include "geometry/SubRoom.h"

#include <Logger.h>

StartDistribution::StartDistribution(int seed)
{
    _roomID             = -1;
    _subroomID          = -1;
    _subroomUID         = -1;
    _nPeds              = -1;
    _groupID            = -1;
    _goalID             = -1;
    _routerID           = -1;
    _routeID            = -1;
    _startX             = NAN;
    _startY             = NAN;
    _startZ             = NAN;
    _xMin               = -FLT_MAX;
    _xMax               = FLT_MAX;
    _yMin               = -FLT_MAX;
    _yMax               = FLT_MAX;
    _groupParameters    = nullptr;
    static bool _seeded = false; // seed only once, not every time
    if(!_seeded) {
        _generator = std::default_random_engine(seed);
        _seeded    = true;
    }
}

StartDistribution::~StartDistribution() {}


std::default_random_engine StartDistribution::GetGenerator()
{
    return _generator;
}

int StartDistribution::GetAgentsNumber() const
{
    return _nPeds;
}

void StartDistribution::SetRoomID(int id)
{
    _roomID = id;
}

int StartDistribution::GetGoalId() const
{
    return _goalID;
}

void StartDistribution::SetGoalId(int goalId)
{
    _goalID = goalId;
}

int StartDistribution::GetGroupId() const
{
    return _groupID;
}

void StartDistribution::SetGroupId(int groupId)
{
    _groupID = groupId;
}

int StartDistribution::GetRoomId() const
{
    return _roomID;
}

void StartDistribution::SetRoomId(int roomId)
{
    _roomID = roomId;
}

int StartDistribution::GetSubroomID() const
{
    return _subroomID;
}

void StartDistribution::SetSubroomID(int subroomId)
{
    _subroomID = subroomId;
}

int StartDistribution::GetSubroomUID() const
{
    return _subroomUID;
}

void StartDistribution::SetSubroomUID(int subroomUId)
{
    _subroomUID = subroomUId;
}


int StartDistribution::GetRouteId() const
{
    return _routeID;
}

void StartDistribution::SetRouteId(int routeId)
{
    _routeID = routeId;
}

int StartDistribution::GetRouterId() const
{
    return _routerID;
}

void StartDistribution::SetRouterId(int routerId)
{
    _routerID = routerId;
}

void StartDistribution::SetAgentsNumber(int N)
{
    _nPeds = N;
}


Pedestrian * StartDistribution::GenerateAgent(Building * building, std::vector<Point> & positions)
{
    Pedestrian * ped = new Pedestrian();
    // PedIndex
    ped->SetFinalDestination(GetGoalId());
    ped->SetGroup(GetGroupId());
    ped->SetRouter(building->GetRoutingEngine()->GetRouter(_routerID));
    ped->SetBuilding(building);
    ped->SetPremovementTime(GetPremovementTime());

    // a und b setzen muss vor v0 gesetzt werden,
    // da sonst v0 mit Null überschrieben wird
    JEllipse E = JEllipse();
    E.SetAv(_groupParameters->GetAtau());
    E.SetAmin(_groupParameters->GetAmin());
    E.SetBmax(_groupParameters->GetBmax());
    E.SetBmin(_groupParameters->GetBmin());
    E.DoStretch(_groupParameters->StretchEnabled());
    ped->SetEllipse(E);
    ped->SetTau(_groupParameters->GetTau());
    ped->SetT(_groupParameters->GetT());
    ped->SetV0Norm(
        _groupParameters->GetV0(),
        _groupParameters->GetV0UpStairs(),
        _groupParameters->GetV0DownStairs(),
        _groupParameters->GetEscalatorUpStairs(),
        _groupParameters->GetEscalatorDownStairs());
    ped->SetSmoothFactorEscalatorUpStairs(_groupParameters->_smoothFactorEscalatorUpStairs);
    ped->SetSmoothFactorEscalatorDownStairs(_groupParameters->_smoothFactorEscalatorDownStairs);
    ped->SetSmoothFactorUpStairs(_groupParameters->_smoothFactorUpStairs);
    ped->SetSmoothFactorDownStairs(_groupParameters->_smoothFactorDownStairs);

    // first default Position
    int index = -1;

    //in the case a range was specified
    for(unsigned int a = 0; a < positions.size(); a++) {
        Point pos = positions[a];
        if((_xMin <= pos._x) && (pos._x <= _xMax) && (_yMin <= pos._y) && (pos._y < _yMax)) {
            index = a;
            break;
        }
    }
    if(index == -1) {
        if(positions.size()) {
            LOG_ERROR(
                "Cannot distribute pedestrians in the mentioned area "
                "[{:2f},{:2f},{:2f},{:2f}], Specifying a subroom_id may help. {:d} positions were "
                "available. Index {:d}",
                _xMin,
                _xMax,
                _yMin,
                _yMax,
                positions.size(),
                index);
            exit(EXIT_FAILURE);
        }
    } else {
        Point pos = positions[index];
        ped->SetPos(pos); //true for the initial position
        positions.erase(positions.begin() + index);

        const Point & start_pos = Point(_startX, _startY);

        if(!std::isnan(start_pos._x) && !std::isnan(start_pos._y)) {
            ped->SetPos(start_pos); //true for the initial position
            LOG_INFO(
                "fixed position for ped {:d} in Room {:d} {}",
                ped->GetID(),
                GetRoomId(),
                start_pos.toString().c_str());
        }
    }
    return ped;
}

void StartDistribution::SetStartPosition(double x, double y, double z)
{
    if(_nPeds != 1) {
        LOG_WARNING("You cannot specify the same start position for many agents. This starting "
                    "position will be ignored");
        return;
    }
    _startX = x;
    _startY = y;
    _startZ = z;
}

Point StartDistribution::GetStartPosition() const
{
    return Point(_startX, _startY);
}

AgentsParameters * StartDistribution::GetGroupParameters() const
{
    return _groupParameters;
}

void StartDistribution::SetGroupParameters(AgentsParameters * groupParameters)
{
    _groupParameters = groupParameters;
}

void StartDistribution::Getbounds(double bounds[4])
{
    bounds[0] = _xMin;
    bounds[1] = _xMax;
    bounds[2] = _yMin;
    bounds[3] = _yMax;
}

void StartDistribution::Setbounds(double bounds[4])
{
    _xMin = bounds[0];
    _xMax = bounds[1];
    _yMin = bounds[2];
    _yMax = bounds[3];
}

void StartDistribution::InitPremovementTime(double mean, double stdv)
{
    if(stdv <= 0) {
        stdv = judge;
    }
    _premovementTime = std::normal_distribution<double>(mean, stdv);
}

double StartDistribution::GetPremovementTime() const
{
    if(_premovementTime.stddev() == judge) {
        return _premovementTime.mean();
    } else {
        return _premovementTime(_generator);
    }
}

void StartDistribution::InitRiskTolerance(std::string distribution_type, double para1, double para2)
{
    _distribution_type = distribution_type;
    if(distribution_type == "normal") {
        if(para2 <= 0) {
            para2 = judge;
        }
        _riskTolerance = std::normal_distribution<double>(para1, para2);
    }
    if(distribution_type == "beta") {
        _risk_beta_dist = boost::math::beta_distribution<>(para1, para2);
    }
}
