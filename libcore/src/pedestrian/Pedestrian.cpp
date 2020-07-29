/**
 * \file        Pedestrian.cpp
 * \date        Sep 30, 2010
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
#include "Pedestrian.h"

#include "JPSfire/generic/FDSMeshStorage.h"
#include "Knowledge.h"
#include "geometry/Building.h"
#include "geometry/SubRoom.h"
#include "geometry/WaitingArea.h"

#include <Logger.h>
#include <cassert>

// initialize the static variables
double Pedestrian::_globalTime         = 0.0;
int Pedestrian::_agentsCreated         = 1;
double Pedestrian::_minPremovementTime = std::numeric_limits<double>::max();
AgentColorMode Pedestrian::_colorMode  = BY_VELOCITY;
std::vector<int> colors                = {
    0,
    255,
    35,
    127,
    90,
};


Pedestrian::Pedestrian()
{
    _id                        = _agentsCreated; //default id
    _exitIndex                 = -1;
    _group                     = -1;
    _desiredFinalDestination   = FINAL_DEST_OUT;
    _premovement               = 0;
    _riskTolerance             = 0;
    _mass                      = 1;
    _tau                       = 0.5;
    _t                         = 1.0;
    _deltaT                    = 0.01;
    _ellipse                   = JEllipse();
    _v0                        = Point(0, 0);
    _v0UpStairs                = 0.6;
    _v0DownStairs              = 0.6;
    _v0EscalatorUpStairs       = 0.8;
    _v0EscalatorDownStairs     = 0.8;
    _v0IdleEscalatorUpStairs   = 0.6;
    _v0IdleEscalatorDownStairs = 0.6;
    _roomID                    = -1;
    _subRoomID                 = -1;
    _subRoomUID                = -1;
    _oldRoomID                 = std::numeric_limits<int>::min();
    _oldSubRoomID              = std::numeric_limits<int>::min();
    _lastE0                    = Point(0, 0);
    _navLine                   = nullptr;
    _mentalMap                 = std::map<int, int>();
    _destHistory               = std::vector<int>();
    _lastPosition              = Point(J_NAN, J_NAN);
    _distToBlockade            = 0.0;
    // new orientation after 10 seconds, value is incremented
    _timeBeforeRerouting = 0.0;
    _timeInJam           = 0.0;
    _patienceTime        = 5.0; // time after which the ped feels to be in jam
    _recordingTime       = 20;  //seconds
    _routingStrategy     = ROUTING_GLOBAL_SHORTEST;
    _newOrientationDelay = 0; //0 seconds, in steps
    _reroutingEnabled    = false;
    _router              = nullptr;
    _building            = nullptr;
    _spotlight           = false;

    _agentsCreated++; //increase the number of object created
    _fedIn            = 0.0;
    _fedHeat          = 0.0;
    _walkingSpeed     = nullptr;
    _toxicityAnalysis = nullptr;
    _waitingPos = Point(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
}

Pedestrian::Pedestrian(const StartDistribution & agentsParameters, Building & building) :
    _group(agentsParameters.GetGroupId()),
    _desiredFinalDestination(agentsParameters.GetGoalId()),
    _premovement(agentsParameters.GetPremovementTime()),
    _roomID(agentsParameters.GetRoomId()),
    _subRoomID(agentsParameters.GetSubroomID()),
    _subRoomUID(building.GetRoom(_roomID)->GetSubRoom(_subRoomID)->GetUID()),
    _lastPosition(),

    _patienceTime(agentsParameters.GetPatience()),
    _router(building.GetRoutingEngine()->GetRouter(agentsParameters.GetRouterId())),
    _building(&building)
{
    _roomID              = -1;
    _subRoomID           = -1;
    _subRoomUID          = -1;
    _oldRoomID           = -1;
    _oldSubRoomID        = -1;
    _exitIndex           = -1;
    _id                  = _agentsCreated; //default id
    _mass                = 1;
    _tau                 = 0.5;
    _t                   = 1.0;
    _newOrientationDelay = 0; //0 seconds, in steps
    _ellipse             = JEllipse();
    _navLine             = nullptr;
    _router              = nullptr;
    _building            = nullptr;
    // new orientation after 10 seconds, value is incremented
    _timeBeforeRerouting       = 0.0;
    _reroutingEnabled          = false;
    _timeInJam                 = 0.0;
    _patienceTime              = 5.0; // time after which the ped feels to be in jam
    _desiredFinalDestination   = FINAL_DEST_OUT;
    _mentalMap                 = std::map<int, int>();
    _destHistory               = std::vector<int>();
    _deltaT                    = 0.01;
    _v0                        = Point(0, 0);
    _lastPosition              = Point(0, 0);
    _recordingTime             = 20; //seconds
    _group                     = -1;
    _spotlight                 = false;
    _v0UpStairs                = 0.6;
    _v0DownStairs              = 0.6;
    _v0EscalatorUpStairs       = 0.8;
    _v0EscalatorDownStairs     = 0.8;
    _v0IdleEscalatorUpStairs   = 0.6;
    _v0IdleEscalatorDownStairs = 0.6;
    _distToBlockade            = 0.0;
    _routingStrategy           = ROUTING_GLOBAL_SHORTEST;
    _lastE0                    = Point(0, 0);
    _agentsCreated++; //increase the number of object created
    _fedIn            = 0.0;
    _fedHeat          = 0.0;
    _toxicityAnalysis = nullptr;
    _walkingSpeed     = nullptr;
    _waitingPos = Point(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
}


Pedestrian::~Pedestrian()
{
    delete _navLine;
}


void Pedestrian::SetID(int i)
{
    _id = i;
    if(i <= 0) {
        throw std::logic_error("Invalid pedestrians ID: Pedestrian ID should be > 0.");
    }
}

void Pedestrian::SetRoomID(int i)
{
    _roomID = i;
}

void Pedestrian::SetSubRoomID(int i)
{
    _subRoomID = i;
}

void Pedestrian::SetSubRoomUID(int i)
{
    _subRoomUID = i;
}

void Pedestrian::SetTau(double tau)
{
    _tau = tau;
}

void Pedestrian::SetT(double T)
{
    _t = T;
}

void Pedestrian::SetEllipse(const JEllipse & e)
{
    _ellipse = e;
}

void Pedestrian::SetExitIndex(int i)
{
    _exitIndex                    = i;
    _mentalMap[GetUniqueRoomID()] = i;
}

void Pedestrian::SetExitLine(const NavLine * l)
{
    if(_navLine != nullptr)
        delete _navLine;
    if(l != nullptr) {
        _navLine = new NavLine(*l);
    }
}

void Pedestrian::SetPos(const Point & pos, bool initial)
{
    if((_globalTime >= _premovement) || initial) {
        _lastPosition = _ellipse.GetCenter();
        _ellipse.SetCenter(pos);
        //save the last values for the records
        _lastPositions.push(pos);
        unsigned int max_size = _recordingTime / _deltaT;
        if(_lastPositions.size() > max_size) {
            _lastPositions.pop();
        }
    }
}

void Pedestrian::SetV(const Point & v)
{
    if(_globalTime >= _premovement) {
        _ellipse.SetV(v);
        //save the last values for the records
        _lastVelocites.push(v);

        unsigned int max_size = _recordingTime / _deltaT;
        if(_lastVelocites.size() > max_size) {
            _lastVelocites.pop();
        }
    }
}

void Pedestrian::SetV0Norm(
    double v0,
    double v0UpStairs,
    double v0DownStairs,
    double escalatorUp,
    double escalatorDown,
    double v0IdleEscalatorUp,
    double v0IdleEscalatorDown)
{
    _ellipse.SetV0(v0);
    _v0DownStairs              = v0DownStairs;
    _v0UpStairs                = v0UpStairs;
    _v0EscalatorUpStairs       = escalatorUp;
    _v0EscalatorDownStairs     = escalatorDown;
    _v0IdleEscalatorUpStairs   = v0IdleEscalatorUp;
    _v0IdleEscalatorDownStairs = v0IdleEscalatorDown;
}


void Pedestrian::SetFEDIn(double FED_In)
{
    _fedIn = FED_In;
}
double Pedestrian::GetFEDIn() const
{
    return _fedIn;
}

void Pedestrian::SetFEDHeat(double FED_Heat)
{
    _fedHeat = FED_Heat;
}
double Pedestrian::GetFEDHeat() const
{
    return _fedHeat;
}

void Pedestrian::SetDeltaT(double dt)
{
    _deltaT = dt;
}

int Pedestrian::GetID() const
{
    return _id;
}

int Pedestrian::GetRoomID() const
{
    return _roomID;
}

int Pedestrian::GetSubRoomID() const
{
    return _subRoomID;
}

int Pedestrian::GetOldRoomID() const
{
    return _oldRoomID;
}

int Pedestrian::GetOldSubRoomID() const
{
    return _oldSubRoomID;
}

int Pedestrian::GetSubRoomUID() const
{
    return _subRoomUID;
}

double Pedestrian::GetMass() const
{
    return _mass;
}

double Pedestrian::GetTau() const
{
    return _tau;
}

double Pedestrian::GetT() const
{
    return _t;
}

const JEllipse & Pedestrian::GetEllipse() const
{
    return _ellipse;
}

int Pedestrian::GetExitIndex() const
{
    return _exitIndex;
}

NavLine * Pedestrian::GetExitLine() const
{
    return _navLine;
}

// return the unique subroom Identifier

int Pedestrian::GetUniqueRoomID() const
{
    return _roomID * 1000 + _subRoomID;
}

RoutingStrategy Pedestrian::GetRoutingStrategy() const
{
    return _routingStrategy;
}

// returns the exit Id corresponding to the
// unique subroom identifier

int Pedestrian::GetNextDestination()
{
    if(_mentalMap.count(GetUniqueRoomID()) == 0) {
        return -1;
    }
    return _mentalMap[GetUniqueRoomID()];
}

Point Pedestrian::GetLastE0() const
{
    return _lastE0;
}
void Pedestrian::SetLastE0(Point E0)
{
    _lastE0 = E0;
}

bool Pedestrian::ChangedSubRoom() const
{
    return (ChangedRoom() || _oldSubRoomID != _subRoomID) &&
           _oldSubRoomID != std::numeric_limits<int>::min();
}

bool Pedestrian::ChangedRoom() const
{
    return _oldRoomID != _roomID && _oldRoomID != std::numeric_limits<int>::min();
}

const std::vector<int> & Pedestrian::GetLastDestinations() const
{
    return _destHistory;
}

std::string Pedestrian::GetKnowledgeAsString() const
{
    std::string key;
    for(auto && knowledge : _knownDoors) {
        //skip low quality information
        if(knowledge.second.GetQuality() < 0.2) {
            continue;
        }

        int door = knowledge.first;
        if(key.empty()) {
            key.append(std::to_string(door));
        } else {
            key.append(":" + std::to_string(door));
        }
    }
    return key;
}

const Point & Pedestrian::GetPos() const
{
    return _ellipse.GetCenter();
}

const Point & Pedestrian::GetV() const
{
    return _ellipse.GetV();
}

const Point & Pedestrian::GetV0() const
{
    return _v0;
}


double Pedestrian::GetV0Norm() const
{
    // @todo: we need to know the difference of the ped_elevation to the old_nav_elevation, and use this in the function f.
    //detect the walking direction based on the elevation
    SubRoom * sub        = _building->GetRoom(_roomID)->GetSubRoom(_subRoomID);
    double ped_elevation = sub->GetElevation(_ellipse.GetCenter());
    if(_navLine == nullptr) {
        LOG_ERROR("ped {:d} has no navline", _id);
        return std::max(0., _ellipse.GetV0());
    }
    const Point & target = _navLine->GetCentre();
    double nav_elevation = sub->GetElevation(target);
    double delta         = nav_elevation - ped_elevation;
    double walking_speed = 0;
    // we are walking on an even plane
    //TODO: move _ellipse.GetV0() to _V0Plane
    if(fabs(delta) < J_EPS) {
        //FIXME std::normal_distribution generated V0's that are very small or even < 0
        //assume absolute v_min according to Weidmann
        walking_speed = std::max(0., _ellipse.GetV0());

    }
    // we are walking downstairs
    else {
        double c = 15.0;
        // c should be chosen so that the func grows fast (but smooth) from 0 to 1
        // However we have to pay attention to tau. The velocity adaptation
        // from v to v0 in the driven force takes tau time.
        if(delta < 0) {
            double maxSubElevation  = sub->GetMaxElevation();
            double stairLength      = maxSubElevation - sub->GetMinElevation();
            double stairInclination = acos(sub->GetCosAngleWithHorizontal());
            double f = 2.0 / (1 + exp(-c * stairInclination * (maxSubElevation - ped_elevation) *
                                      (maxSubElevation - ped_elevation))) -
                       1;
            double g = 2.0 / (1 + exp(-c * stairInclination *
                                      (maxSubElevation - ped_elevation - stairLength) *
                                      (maxSubElevation - ped_elevation - stairLength))) -
                       1;
            double speed_down = _v0DownStairs;
            if(sub->GetType() == "escalator") {
                speed_down = _v0EscalatorDownStairs;
            } else if(sub->GetType() == "idle_escalator") {
                speed_down = _v0IdleEscalatorDownStairs;
            }
            walking_speed = (1 - f * g) * _ellipse.GetV0() + f * g * speed_down;
        }
        //we are walking upstairs
        else {
            double minSubElevation  = sub->GetMinElevation();
            double stairHeight      = sub->GetMaxElevation() - minSubElevation;
            double stairInclination = acos(sub->GetCosAngleWithHorizontal());
            double f = 2.0 / (1 + exp(-c * stairInclination * (minSubElevation - ped_elevation) *
                                      (minSubElevation - ped_elevation))) -
                       1;
            double g = 2.0 / (1 + exp(-c * stairInclination *
                                      (ped_elevation - minSubElevation - stairHeight) *
                                      (ped_elevation - minSubElevation - stairHeight))) -
                       1;

            //FIXME std::normal_distribution generated V0's that are very small or even < 0
            double speed_up = std::max(0.0, _v0UpStairs);

            if(sub->GetType() == "escalator") {
                speed_up = _v0EscalatorUpStairs;
            } else if(sub->GetType() == "idle_escalator") {
                speed_up = _v0IdleEscalatorUpStairs;
            }
            walking_speed = (1 - f * g) * _ellipse.GetV0() + f * g * speed_up;
        }
    }

    //IF execution of WalkingInSmoke depending on JPSfire section in INI file
    if(_walkingSpeed && _walkingSpeed->ReduceWalkingSpeed()) {
        walking_speed = _walkingSpeed->WalkingInSmoke(this, walking_speed);
    }
    //WHERE should the call to that routine be placed properly?
    //only executed every 3 seconds
    return walking_speed;
}

void Pedestrian::ConductToxicityAnalysis()
{
    if(_toxicityAnalysis->ConductToxicityAnalysis()) {
        _toxicityAnalysis->HazardAnalysis(this);
    }
}

// get axis in the walking direction
double Pedestrian::GetLargerAxis() const
{
    return _ellipse.GetEA();
}
// get axis in the shoulder direction = orthogonal to the walking direction
double Pedestrian::GetSmallerAxis() const
{
    return _ellipse.GetEB();
}

void Pedestrian::SetPhiPed()
{
    double cosPhi;
    double sinPhi;
    double vx = GetV()._x;
    double vy = GetV()._y;

    if(fabs(vx) > J_EPS || fabs(vy) > J_EPS) {
        double normv = sqrt(vx * vx + vy * vy);
        cosPhi       = vx / normv;
        sinPhi       = vy / normv;
    } else {
        cosPhi = GetEllipse().GetCosPhi();
        sinPhi = GetEllipse().GetSinPhi();
    }
    _ellipse.SetCosPhi(cosPhi);
    _ellipse.SetSinPhi(sinPhi);
}


void Pedestrian::InitV0(const Point & target)
{
    const Point & pos = GetPos();
    Point delta       = target - pos;

    _v0 = delta.Normalized();
}


const Point & Pedestrian::GetV0(const Point & target)
{
    // Molification around the targets makes little sense
    const Point & pos = GetPos();
    Point delta       = target - pos;
    Point new_v0      = delta.Normalized();

    double t = _newOrientationDelay++ * _deltaT / (1.0 + 100 * _distToBlockade);

    //Handover new target
    _v0 = _v0 + (new_v0 - _v0) * (1 - exp(-t / _tau));
    return _v0;
}

double Pedestrian::GetTimeInJam() const
{
    return _timeInJam;
}

void Pedestrian::SetSmoothTurning()
{
    _newOrientationDelay = 0;
}

bool Pedestrian::IsFeelingLikeInJam() const
{
    return (_patienceTime < _timeInJam);
}

//reduce the felt time in Jam by half
void Pedestrian::ResetTimeInJam()
{
    _timeInJam = 0.0;
}

void Pedestrian::UpdateTimeInJam()
{
    _timeInJam += _deltaT;
}

void Pedestrian::UpdateReroutingTime()
{
    _timeBeforeRerouting -= _deltaT;
}

void Pedestrian::RerouteIn(double time)
{
    _reroutingEnabled    = true;
    _timeBeforeRerouting = time;
}

bool Pedestrian::IsReadyForRerouting() const
{
    return (_reroutingEnabled && (_timeBeforeRerouting <= 0.0));
}


int Pedestrian::GetGroup() const
{
    return _group;
}

void Pedestrian::SetGroup(int group)
{
    _group = group;
}

void Pedestrian::ResetRerouting()
{
    _reroutingEnabled    = false;
    _timeBeforeRerouting = -1.00;
}

void Pedestrian::SetRecordingTime(double timeInSec)
{
    _recordingTime = timeInSec;
}

double Pedestrian::GetRecordingTime() const
{
    return _recordingTime;
}

double Pedestrian::GetMeanVelOverRecTime() const
{
    //just few position were saved
    if(_lastPositions.size() < 2) {
        return _ellipse.GetV().Norm();
    }
    return fabs((_lastPositions.back() - _lastPositions.front()).Norm() / _recordingTime);
}

double Pedestrian::GetDistanceToNextTarget() const
{
    return (_navLine->DistTo(GetPos()));
}

void Pedestrian::SetFinalDestination(int finale)
{
    _desiredFinalDestination = finale;
}

int Pedestrian::GetFinalDestination() const
{
    return _desiredFinalDestination;
}

std::string Pedestrian::GetPath()
{
    std::map<int, int>::iterator iter;
    std::string path;

    for(iter = _mentalMap.begin(); iter != _mentalMap.end(); iter++) {
        std::stringstream ss; //create a stringstream
        ss << iter->first / 1000 << ":" << iter->second
           << ">"; //@todo:ar.graf: has this to do with roomNr*1000+subroom and is now wrong?
        path.append(ss.str());
    }
    return path;
}

double Pedestrian::GetGlobalTime()
{
    return _globalTime;
}

void Pedestrian::SetRouter(Router * router)
{
    _router          = router;
    _routingStrategy = router->GetStrategy();
}

int Pedestrian::FindRoute()
{
    if(_router == nullptr) {
        LOG_ERROR("One or more routers does not exist! Check your router_ids");
        return -1;
    }
    return _router->FindExit(this);
}

double Pedestrian::GetElevation() const
{
    return _building->GetRoom(_roomID)->GetSubRoom(_subRoomID)->GetElevation(GetPos());
}

void Pedestrian::SetGlobalTime(double time)
{
    _globalTime = time;
}

double Pedestrian::GetPatienceTime() const
{
    return _patienceTime;
}

void Pedestrian::SetPatienceTime(double patienceTime)
{
    _patienceTime = patienceTime;
}

void Pedestrian::SetPremovementTime(double pretime)
{
    if(pretime < _minPremovementTime) {
        _minPremovementTime = pretime;
    }
    _premovement = pretime;
}

double Pedestrian::GetMinPremovementTime()
{
    return _minPremovementTime;
}

double Pedestrian::GetPremovementTime() const
{
    return _premovement;
}

void Pedestrian::SetRiskTolerance(double tol)
{
    if(tol > 1) {
        tol = 1;
    }
    if(tol < 0) {
        tol = 0;
    }
    _riskTolerance = tol;
}

double Pedestrian::GetRiskTolerance() const
{
    return _riskTolerance;
}

const Building * Pedestrian::GetBuilding()
{
    return _building;
}

void Pedestrian::SetBuilding(Building * building)
{
    _building = building;
}

void Pedestrian::SetWalkingSpeed(std::shared_ptr<WalkingSpeed> walkingSpeed)
{
    _walkingSpeed = walkingSpeed;
}

void Pedestrian::SetTox(std::shared_ptr<ToxicityAnalysis> toxicityAnalysis)
{
    _toxicityAnalysis = toxicityAnalysis;
}

void Pedestrian::SetSpotlight(bool spotlight)
{
    _spotlight = spotlight;
}

bool Pedestrian::GetSpotlight() const
{
    return _spotlight;
}

void Pedestrian::SetColorMode(AgentColorMode mode)
{
    _colorMode = mode;
}

int Pedestrian::GetAgentsCreated()
{
    return _agentsCreated;
}

int Pedestrian::GetColor() const
{
    //default color is by velocity
    std::string key;

    switch(_colorMode) {
        case BY_SPOTLIGHT: {
            if(!_spotlight) {
                return -1;
            }
            break;
        }

        case BY_VELOCITY: {
            int color = -1;
            double v0 = GetV0Norm();
            if(v0 != 0.0) {
                double v = GetV().Norm();
                color    = static_cast<int>(v / v0 * 255);
            }
            return color;
        }

        // Hash the knowledge represented as String
        case BY_KNOWLEDGE: {
            key = GetKnowledgeAsString();
            if(key.empty()) {
                return -1;
            }
        } break;

        case BY_ROUTER:
        case BY_ROUTE: {
            key = std::to_string(_routingStrategy);
        } break;

        case BY_GROUP: {
            key = std::to_string(_group); // @todo find a better solution to get
                                          // colors clearly distinguishable form
                                          // each other
            return (colors[_group % colors.size()]);
        }

        case BY_FINAL_GOAL: {
            key = std::to_string(_desiredFinalDestination);
        } break;

        case BY_INTERMEDIATE_GOAL: {
            key = std::to_string(_exitIndex);
        } break;

        default:
            break;
    }

    std::hash<std::string> hash_fn;
    return hash_fn(key) % 255;
}


int Pedestrian::GetLastGoalID() const
{
    return _lastGoalID;
}

bool Pedestrian::IsInsideWaitingAreaWaiting() const
{
    if(_insideGoal) {
        auto itr = _building->GetAllGoals().find(_desiredFinalDestination);
        if(itr != _building->GetAllGoals().end()) {
            Goal * goal = itr->second;
            if(auto wa = dynamic_cast<WaitingArea *>(goal)) {
                return wa->IsWaiting(Pedestrian::GetGlobalTime(), _building);
            }
        }
    }
    return false;
}

void Pedestrian::EnterGoal()
{
    _insideGoal = true;
    _lastGoalID = _desiredFinalDestination;
}

void Pedestrian::LeaveGoal()
{
    _insideGoal = false;
}

bool Pedestrian::IsWaiting() const
{
    return _waiting;
}

void Pedestrian::StartWaiting()
{
    _waiting = true;
}

void Pedestrian::EndWaiting()
{
    _waiting = false;
}

const Point & Pedestrian::GetWaitingPos() const
{
    return _waitingPos;
}

void Pedestrian::SetWaitingPos(const Point & waitingPos)
{
    _waitingPos = waitingPos;
}

void Pedestrian::UpdateRoom(int roomID, int subRoomID)
{
    _oldRoomID    = _roomID;
    _oldSubRoomID = _subRoomID;
    _roomID       = roomID;
    _subRoomID    = subRoomID;
}

const std::queue<Point> & Pedestrian::GetLastPositions() const
{
    return _lastPositions;
}

Point Pedestrian::GetLastPosition() const
{
    return _lastPosition;
}

std::string Pedestrian::ToString() const
{
    std::string message = fmt::format(
        FMT_STRING("------> ped {:d} <-------\n"
                   ">> Room/Subroom [{:d} / {:d}]\n"
                   ">> Destination [ {:d} ]\n"
                   ">> Final Destination [ {:d} ]\n"
                   ">> Position [{:.2f}, {:.2f}]\n"
                   ">> Velocity [{:.2f}, {:.2f}]  Norm = [{:.2f}]\n"),
        _id,
        _roomID,
        _subRoomID,
        _exitIndex,
        _desiredFinalDestination,
        GetPos()._x,
        GetPos()._y,
        GetV()._x,
        GetV()._y,
        GetV().Norm());

    return message;
}

std::ostream & operator<<(std::ostream & out, const Pedestrian & pedestrian)
{
    return out << pedestrian.ToString();
}