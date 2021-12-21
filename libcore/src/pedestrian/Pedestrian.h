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
#include "geometry/NavLine.h"
#include "geometry/SubroomType.h"

#include <map>
#include <queue>
#include <set>
#include <vector>

class Building;
class NavLine;
class Router;
class WalkingSpeed;
class Pedestrian
{
private:
    //generic parameters, independent from models
    int _id;        //starting with 1
    int _exitIndex; // current exit
    int _group;
    int _desiredFinalDestination;
    double _premovement = 0;

    //gcfm specific parameters
    double _mass;      // Mass: 1
    double _tau;       // Reaction time: 0.5
    double _t;         // OV function
    double _deltaT;    // step size
    JEllipse _ellipse; // the shape of this pedestrian
    Point _v0;         //vector V0


    double _v0UpStairs;
    double _v0DownStairs;
    double _v0EscalatorUpStairs;
    double _v0EscalatorDownStairs;
    /// c in f() and g() for v0 transition on stairs up
    double _smoothFactorUpStairs;
    /// c in f() and g() for v0 transition on stairs down
    double _smoothFactorDownStairs;
    /// c in f() and g() for v0 transition on escalators up
    double _smoothFactorEscalatorUpStairs;
    /// c in f() and g() for v0 transition on escalators down
    double _smoothFactorEscalatorDownStairs;
    int _router_id{0};
    Point _lastE0;

    NavLine * _navLine;            // current exit line
    std::map<int, int> _mentalMap; // map the actual room to a destination
    Point _lastPosition;

    /// a new orientation starts after this time
    double _timeBeforeRerouting;

    int _newOrientationDelay; //2 seconds, in steps

    bool _reroutingEnabled;

    // the current time in the simulation
    static double _minPremovementTime;
    static AgentColorMode _colorMode;

    /// the router responsible for this pedestrian
    Router * _router;
    /// a pointer to the complete building
    Building * _building;

    static int _agentsCreated;

    int _lastGoalID  = -1;
    bool _insideGoal = false;
    bool _waiting    = false;
    Point _waitingPos;

public:
    // constructors
    Pedestrian();

    explicit Pedestrian(const StartDistribution & agentsParameters, Building & building);
    virtual ~Pedestrian();

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
    void SetID(int i);
    void SetTau(double tau);
    void SetEllipse(const JEllipse & e);

    void SetRouterId(int id) { _router_id = id; }

    const Router * GetRouter() const { return _router; }

    double GetT() const;
    void SetT(double T);
    //TODO: merge this two functions
    void SetExitIndex(int i);
    void SetExitLine(const NavLine * l);
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
    void SetRouter(Router * router);
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
    int GetExitIndex() const;
    NavLine * GetExitLine() const;
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
    int GetNextDestination();
    double GetDistanceToNextTarget() const;

    /**
      * The elevation is computed using the plane
      * equation given in the subroom.
      * @return the z coordinate of the pedestrian.
      */

    double GetElevation() const;

    /**
      * Compute and update the route.
      * This method should be called at each time step;
      *
      * @return -1 if no route could be found. The ID of the
      * next target is returned otherwise.
      *
      */
    int FindRoute();

    ///write the pedestrian path (room and exit taken ) to file
    /// in the format room1:exit1>room2:exit2
    std::string GetPath();

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

    /**
      * return the pedestrian color used for visualiation.
      * Default mode is coded by velocity.
      * @return a value in [-1 255]
      */
    int GetColor() const;

    void UpdateReroutingTime();
    void RerouteIn(double time);
    bool IsReadyForRerouting() const;

    /**
      * clear the parameter related to the re routing
      */
    void ResetRerouting();

    int GetGroup() const;
    void SetGroup(int group);

    /**
      * @return the total number of pedestrians objects created.
      * This is useful for the linked-cells algorithm, since it uses the ID of the pedestrians
      * and the  maximal count must be known in advance.
      */
    static int GetAgentsCreated();

    /**
      * Set the color mode for the pedestrians
      * @param mode
      */
    static void SetColorMode(AgentColorMode mode);

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
