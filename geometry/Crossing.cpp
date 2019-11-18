/**
 * \file        Crossing.cpp
 * \date        Nov 16, 2010
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
#include "Crossing.h"

#include "SubRoom.h"
#include "general/Format.h"
#include "general/Logger.h"


Crossing::Crossing()
{
    _id           = -1;
    _doorUsage    = 0;
    _tempDoorUsage = 0;
    _maxDoorUsage = (std::numeric_limits<int>::max)(); //avoid name conflicts in windows winmindef.h
    _outflowRate  = (std::numeric_limits<double>::max)();
    _lastPassingTime     = 0;
    _lastFlowMeasurement = 0;
    _DT                  = 1;
    _DN                  = 1;
    _partialDoorUsage    = 0;
    _closingTime         = 0;

    _state = DoorState::OPEN;
}

bool Crossing::IsExit() const
{
    return false;
}

bool Crossing::IsOpen() const
{
    return _state == DoorState::OPEN;
}

bool Crossing::IsClose() const
{
    return _state == DoorState::CLOSE;
}

bool Crossing::IsTempClose() const
{
    return _state == DoorState::TEMP_CLOSE;
}

bool Crossing::IsTransition() const
{
    return false;
}

void Crossing::Close(bool event)
{
    if(_state != DoorState::CLOSE) {
        _state        = DoorState::CLOSE;
        _closeByEvent = event;
    } else {
        _closeByEvent = (event || _closeByEvent);
    }
}

void Crossing::TempClose(bool event)
{
    if(_state != DoorState::TEMP_CLOSE) {
        _state        = DoorState::TEMP_CLOSE;
        _closeByEvent = event;
    } else {
        _closeByEvent = (event || _closeByEvent);
    }
}

void Crossing::Open(bool)
{
    _state = DoorState::OPEN;
    _closeByEvent = false;
}

bool Crossing::IsInSubRoom(int subroomID) const
{
    bool r1, r2;
    if(_subRoom1 != nullptr)
        r1 = _subRoom1->GetSubRoomID() == subroomID;
    else
        r1 = false;
    if(_subRoom2 != nullptr)
        r2 = _subRoom2->GetSubRoomID() == subroomID;
    else
        r2 = false;
    return (r1 || r2);
}

/* gibt den ANDEREN Subroom != subroomID zurück
 * roomID wird hier nicht benötigt, aber in Transition::GetOtherSubRoom()
 * (virtuelle Funktion) */
SubRoom * Crossing::GetOtherSubRoom(int roomID, int subroomID) const
{
    if(_subRoom1->GetSubRoomID() == subroomID)
        return _subRoom2;
    else if(_subRoom2->GetSubRoomID() == subroomID)
        return _subRoom1;
    else {
        Logging::Warning(fmt::format(
            check_fmt("Crossing::GetOtherSubroom No exit found on the other side ID={}, roomID={}, "
                      "subroomID={}"),
            GetID(),
            roomID,
            subroomID));
        return nullptr;
    }
}


// Ausgabe
void Crossing::WriteToErrorLog() const
{
    Logging::Debug(fmt::format(check_fmt("{}"), *this));
}

// TraVisTo Ausgabe
std::string Crossing::GetDescription() const
{
    //return "";
    std::string geometry;
    char tmp[CLENGTH] = "";
    sprintf(
        tmp,
        "\t\t<crossing ID=\"%d\" color = \"250\" caption=\"%d_%d\">\n",
        GetUniqueID(),
        GetID(),
        GetUniqueID());
    geometry.append(tmp);
    //geometry.append("\t\t<door color=\"250\">\n");
    sprintf(
        tmp,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\" />\n",
        (GetPoint1()._x) * FAKTOR,
        (GetPoint1()._y) * FAKTOR,
        _subRoom1->GetElevation(GetPoint1()) * FAKTOR);
    geometry.append(tmp);
    sprintf(
        tmp,
        "\t\t\t<point xPos=\"%.2f\" yPos=\"%.2f\" zPos=\"%.2f\" />\n",
        (GetPoint2()._x) * FAKTOR,
        (GetPoint2()._y) * FAKTOR,
        _subRoom1->GetElevation(GetPoint2()) * FAKTOR);
    geometry.append(tmp);
    geometry.append("\t\t</crossing>\n");
    return geometry;
}

int Crossing::CommonSubroomWith(Crossing * other, SubRoom *& subroom)
{
    int result = 0;
    if(_subRoom1 && (_subRoom1 == other->_subRoom1 || _subRoom1 == other->_subRoom2)) {
        ++result;
        subroom = _subRoom1;
    }
    if(_subRoom2 && (_subRoom2 == other->_subRoom1 || _subRoom2 == other->_subRoom2)) {
        ++result;
        subroom = _subRoom2;
    }
    return result;
}

void Crossing::IncreaseDoorUsage(int number, double time)
{
    _doorUsage += number;
    _tempDoorUsage += number;
    _lastPassingTime = time;
    _flowAtExit += std::to_string(time) + "  " + std::to_string(_doorUsage) + "\n";
}

void Crossing::IncreasePartialDoorUsage(int number)
{
    _partialDoorUsage += number;
}

void Crossing::ResetPartialDoorUsage()
{
    _partialDoorUsage = 0;
}

int Crossing::GetDoorUsage() const
{
    return _doorUsage;
}

int Crossing::GetPartialDoorUsage() const
{
    return _partialDoorUsage;
}

void Crossing::ResetDoorUsage()
{
    _tempDoorUsage = 0;

    if((_outflowRate >= std::numeric_limits<double>::max())) {
        if(_maxDoorUsage < std::numeric_limits<double>::max()) {
            Open();
            Logging::Info(fmt::format(check_fmt("Reopening door {}"), _id));
            _closeByEvent = false;
        }
    }
}

int Crossing::GetMaxDoorUsage() const
{
    return _maxDoorUsage;
}


double Crossing::GetOutflowRate() const
{
    return _outflowRate;
}


double Crossing::GetLastPassingTime() const
{
    return _lastPassingTime;
}

const std::string & Crossing::GetFlowCurve() const
{
    return _flowAtExit;
}

void Crossing::SetOutflowRate(double outflow)
{
    _outflowRate = outflow;
}

void Crossing::SetMaxDoorUsage(int mdu)
{
    _maxDoorUsage = mdu;
}

double Crossing::GetClosingTime() const
{
    return _closingTime;
}

void Crossing::UpdateClosingTime(double dt)
{
    _closingTime -= dt;
}

double Crossing::GetDT()
{
    return _DT;
}

void Crossing::SetDT(double dt)
{
    _DT = dt;
}
int Crossing::GetDN()
{
    return _DN;
}

void Crossing::SetDN(int dn)
{
    _DN = dn;
}

bool Crossing::RegulateFlow(double time)
{
    bool change   = false;
    double number = GetPartialDoorUsage();
    double T      = time - _lastFlowMeasurement;
    double flow   = number / T;
    if(_outflowRate != std::numeric_limits<double>::max()) {
        if(flow > _outflowRate) {
            // _outflowRate > flow (=number/deltaTime)
            // _outflowRate = number/(deltaTime + t1)
            // --> [1]
            //---------------------------
            _closingTime = number / _outflowRate - T; //[1]

            this->TempClose();
            Logging::Info(fmt::format(
                check_fmt(
                    "Closing door {} DoorUsage={} (max={}) Flow={:.2f} (max={:.2f}) Time={:.2f}"),
                GetID(),
                GetDoorUsage(),
                GetMaxDoorUsage(),
                flow,
                _outflowRate,
                time));
            change = true;
        }
    }

    // close the door if _maxDoorUsage is reached
    if(_maxDoorUsage != std::numeric_limits<double>::max()) {
        if(_tempDoorUsage >= _maxDoorUsage) {
            Logging::Info(fmt::format(
                check_fmt("Closing door {} DoorUsage={} (>={}) Time={:.2f}"),
                GetID(),
                GetDoorUsage(),
                GetMaxDoorUsage(),
                time));
            this->Close();
            change = true;
        }
    }

    _lastFlowMeasurement = time + _closingTime;
    return change;
}

void Crossing::UpdateTemporaryState(double dt)
{
    // Only update doors which are temp closed by flow regulation
    if(IsTempClose() && !_closeByEvent) {
        // States if the door has to be opened due to flow control
        bool change = false;

        // Check if door needs to be opened due to flow control
        if(_outflowRate != std::numeric_limits<double>::max()) {
            UpdateClosingTime(dt);
            change = (_closingTime <= dt);
        }

        // Check of door is allowed to be opened due to max door usage
        if(_maxDoorUsage != std::numeric_limits<int>::max()) {
            change = change && (_tempDoorUsage < _maxDoorUsage);
        }

        // If needed opens the door
        if(change) {
            _closingTime = 0;
            Open();
        }
    }
}

DoorState Crossing::GetState() const
{
    return _state;
}

void Crossing::SetState(DoorState state)
{
    Crossing::_state = state;
}

std::string Crossing::toString() const
{
    std::stringstream tmp;

    tmp << this->GetPoint1().toString() << "--" << this->GetPoint2().toString();
    switch(_state) {
        case DoorState::OPEN:
            tmp << " open";
            break;
        case DoorState::CLOSE:
            tmp << " close";
            break;
        case DoorState::TEMP_CLOSE:
            tmp << " temp_close";
            break;
        case DoorState::Error:
            tmp << " Error";
            break;
    }
    return tmp.str();
}

namespace fmt
{
template <>
struct formatter<Crossing> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext & ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Crossing & cross, FormatContext & ctx)
    {
        return format_to(
            ctx.out(),
            "CROSS: {} ({}, {}) -- ({}, {}) -- Subroom {} <--> Subroom {}",
            cross.GetID(),
            cross.GetPoint1()._x,
            cross.GetPoint1()._y,
            cross.GetPoint2()._x,
            cross.GetPoint2()._y,
            cross.GetSubRoom1()->GetSubRoomID(),
            cross.GetSubRoom2()->GetSubRoomID());
    }
};
} // namespace fmt
