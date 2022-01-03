/**
 * \file        Pedestrian.h
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
#pragma once

#include "AgentsParameters.h"
#include "Ellipse.h"
#include "PedDistributor.h"
#include "general/Macros.h"
#include "geometry/Line.h"
#include "geometry/SubroomType.h"

#include <map>

class Building;
class Router;
class WalkingSpeed;
class Pedestrian
{
private:
    //generic parameters, independent from models
    int _id;                           //starting with 1
    int _exitIndex               = -1; // current exit
    int _group                   = -1;
    int _desiredFinalDestination = FINAL_DEST_OUT;
    double _premovement          = 0;

    //gcfm specific parameters
    double _mass   = 1;      // Mass: 1
    double _tau    = 0.5;    // Reaction time: 0.5
    double _t      = 1.0;    // OV function
    double _deltaT = 0.01;   // step size
    JEllipse _ellipse{};     // the shape of this pedestrian
    Point _v0 = Point(0, 0); //vector V0


    double _v0UpStairs            = 0.6;
    double _v0DownStairs          = 0.6;
    double _v0EscalatorUpStairs   = 0.8;
    double _v0EscalatorDownStairs = 0.8;
    /// c in f() and g() for v0 transition on stairs up
    double _smoothFactorUpStairs = 15;
    /// c in f() and g() for v0 transition on stairs down
    double _smoothFactorDownStairs = 15;
    /// c in f() and g() for v0 transition on escalators up
    double _smoothFactorEscalatorUpStairs = 15;
    /// c in f() and g() for v0 transition on escalators down
    double _smoothFactorEscalatorDownStairs = 15;
    int _router_id{0};
    Point _lastE0 = Point(0, 0);

    Line _navLine; // current exit line
    Point _lastPosition = Point(J_NAN, J_NAN);

    int _newOrientationDelay = 0;

    // the current time in the simulation
    static double _minPremovementTime;

    /// a pointer to the complete building
    Building * _building = nullptr;

    static int _agentsCreated;

    int _lastGoalID  = -1;
    bool _insideGoal = false;
    bool _waiting    = false;
    Point _waitingPos =
        Point(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

public:
    // constructors
    Pedestrian();

    ~Pedestrian() = default;

    bool InPremovement(double now);

    /** 
     * Select desired speed based on the type of subroom
     * where the agent is moving. 
     * @param type, type of subroom
     * @param delta, negative when moving downstairs and positiv otherwise. 
     * Nearly zero for horizontal movement.
     */
    double SelectV0(SubroomType type, double delta) const;
    /** 
     * Select Smooth factor based on the type of subroom
     * where the agent is moving. 
     * @param type, type of subroom
     * @param delta, negative when moving downstairs and positiv otherwise. 
     * Nearly zero for horizontal movement.
     */
    double SelectSmoothFactor(SubroomType type, double delta) const;
    void SetSmoothFactorUpStairs(double c);
    void SetSmoothFactorDownStairs(double c);
    void SetSmoothFactorEscalatorUpStairs(double c);
    void SetSmoothFactorEscalatorDownStairs(double c);
    void SetTau(double tau);
    void SetEllipse(const JEllipse & e);
    void SetRouterId(int id) { _router_id = id; }

    double GetT() const;
    void SetT(double T);
    //TODO: merge this two functions
    void SetDestination(int i);
    void SetExitLine(const Line * l);
    void SetDeltaT(double dt);
    // Eigenschaften der Ellipse
    void SetPos(const Point & pos); // setzt x und y-Koordinaten
    void SetV(const Point & v);     // setzt x und y-Koordinaten der Geschwindigkeit
    void SetV0Norm(
        double v0,
        double v0UpStairs,
        double v0DownStairs,
        double escalatorUp,
        double escalatorDown);
    void SetSmoothTurning(); // activate the smooth turning with a delay of 2 sec
    void SetPhiPed();
    void SetFinalDestination(int UID);
    int GetRouterID() const;

    double GetV0UpStairsNorm() const;
    double GetV0DownStairsNorm() const;
    double GetV0EscalatorUpNorm() const;
    double GetV0EscalatorDownNorm() const;

    double GetSmoothFactorUpStairs() const;
    double GetSmoothFactorDownStairs() const;
    double GetSmoothFactorUpEscalators() const;
    double GetSmoothFactorDownEscalators() const;

    int GetID() const;
    double GetMass() const;
    double GetTau() const;
    const JEllipse & GetEllipse() const;
    int GetDestination() const;
    const Line & GetExitLine() const;
    Point GetLastE0() const;
    void SetLastE0(Point E0);
    // Eigenschaften der Ellipse
    const Point & GetPos() const;
    const Point & GetV() const;
    const Point & GetV0() const;
    const Point & GetV0(const Point & target);
    void InitV0(const Point & target);

    /**
      * the desired speed is the projection of the speed on the horizontal plane.
      * @return the norm of the desired speed.
      */
    double GetV0Norm() const;

    ///get axis in the walking direction
    double GetLargerAxis() const;
    ///get axis in the shoulder direction = orthogonal to the walking direction
    double GetSmallerAxis() const;
    int GetFinalDestination() const;

    int GetUniqueRoomID() const;
    double GetDistanceToNextTarget() const;

    /**
      * The elevation is computed using the plane
      * equation given in the subroom.
      * @return the z coordinate of the pedestrian.
      */

    double GetElevation() const;

    /**
      * ToString the parameters of this pedestrians.
      * @param ID, the id of the pedestrian
      * @param pa, the parameter to display (0 for all parameters)
      */
    std::string ToString() const;

    /***
      * Set/Get the time after which this pedestrian will start taking actions.
      */
    void SetPremovementTime(double time);

    /***
      * Set/Get the time after which this pedestrian will start taking actions.
      */
    double GetPremovementTime() const;

    /**
      * Get min Premovement time of all pedestrians
      */
    static double GetMinPremovementTime();

    int GetGroup() const;
    void SetGroup(int group);

    /**
      * @return the total number of pedestrians objects created.
      * This is useful for the linked-cells algorithm, since it uses the ID of the pedestrians
      * and the  maximal count must be known in advance.
      */
    static int GetAgentsCreated();

    /**
      * Set/Get the Building object
      */
    const Building * GetBuilding();

    /**
      * Set/Get the Building object
      */
    void SetBuilding(Building * building);

    void EnterGoal();

    void LeaveGoal();

    int GetLastGoalID() const;

    bool IsInsideWaitingAreaWaiting(double time) const;

    const Point & GetWaitingPos() const;

    void SetWaitingPos(const Point & waitingPos);

    bool IsWaiting() const;

    void StartWaiting();
    void EndWaiting();

    Point GetLastPosition() const;
};

std::ostream & operator<<(std::ostream & out, const Pedestrian & pedestrian);
