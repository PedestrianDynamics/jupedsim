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
#include "JPSfire/B_walking_speed/WalkingSpeed.h"
#include "JPSfire/C_toxicity_analysis/ToxicityAnalysis.h"
#include "PedDistributor.h"
#include "general/Macros.h"
#include "geometry/NavLine.h"
#include "pedestrian/Knowledge.h"

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
    double _premovement   = 0;
    double _riskTolerance = 0;

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
    double _v0IdleEscalatorUpStairs;
    double _v0IdleEscalatorDownStairs;
    int _roomID;
    int _subRoomID;
    int _subRoomUID;
    int _oldRoomID;
    int _oldSubRoomID;
    Point _lastE0;

    NavLine * _navLine;            // current exit line
    std::map<int, int> _mentalMap; // map the actual room to a destination
    std::vector<int> _destHistory;

    Point _lastPosition;

    ///state of doors with time stamps
    std::map<int, Knowledge> _knownDoors;

    /// distance to nearest obstacle that blocks the sight of ped.
    double _distToBlockade;
    /// a new orientation starts after this time
    double _timeBeforeRerouting;
    /// actual time im Jam
    double _timeInJam;
    /// time after which the ped feels to be in jam
    double _patienceTime;
    /// data from the last <_recordingTime> seconds will be kept
    double _recordingTime;
    /// store the last positions
    std::queue<Point> _lastPositions;
    /// store the last velocities
    std::queue<Point> _lastVelocites;
    /// routing strategy followed
    RoutingStrategy _routingStrategy;

    int _newOrientationDelay; //2 seconds, in steps

    bool _reroutingEnabled;

    // the current time in the simulation
    static double _globalTime;
    static double _minPremovementTime;
    static AgentColorMode _colorMode;
    bool _spotlight;

    /// the router responsible for this pedestrian
    Router * _router;
    /// a pointer to the complete building
    Building * _building;

    static int _agentsCreated;

    double _fedIn;
    double _fedHeat;

    std::shared_ptr<WalkingSpeed> _walkingSpeed         = nullptr;
    std::shared_ptr<ToxicityAnalysis> _toxicityAnalysis = nullptr;

    int _lastGoalID  = -1;
    bool _insideGoal = false;
    bool _waiting    = false;
    Point _waitingPos;

public:
    // constructors
    Pedestrian();

    explicit Pedestrian(const StartDistribution & agentsParameters, Building & building);
    virtual ~Pedestrian();

    // Setter-Funktionen
    void SetID(int i);
    void SetRoomID(int i);
    void SetSubRoomID(int i);
    void SetSubRoomUID(int i);
    void SetTau(double tau);
    void SetEllipse(const JEllipse & e);

    double GetT() const;
    void SetT(double T);
    //TODO: merge this two functions
    void SetExitIndex(int i);
    void SetExitLine(const NavLine * l);

    void SetFEDIn(double FED_In);
    double GetFEDIn() const;

    void SetFEDHeat(double FED_Heat);
    double GetFEDHeat() const;

    void SetDeltaT(double dt);

    // Eigenschaften der Ellipse
    void SetPos(const Point & pos, bool initial = false); // setzt x und y-Koordinaten
    void SetV(const Point & v); // setzt x und y-Koordinaten der Geschwindigkeit
    void SetV0Norm(
        double v0,
        double v0UpStairs,
        double v0DownStairs,
        double escalatorUp,
        double escalatorDown,
        double v0IdleEscalatorUp,
        double v0IdleEscalatorDown);
    void SetSmoothTurning(); // activate the smooth turning with a delay of 2 sec
    void SetPhiPed();
    void SetFinalDestination(int UID);
    void SetRouter(Router * router);

    int GetID() const;
    int GetRoomID() const;
    int GetSubRoomID() const;
    int GetSubRoomUID() const;
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
    double GetTimeInJam() const;
    int GetFinalDestination() const;

    /**
      * @return all previous destinations used by this pedestrian
      */
    const std::vector<int> & GetLastDestinations() const;

    /**
      * For convenience
      * @return a string representation of the knowledge
      */
    std::string GetKnowledgeAsString() const;

    RoutingStrategy GetRoutingStrategy() const;
    int GetUniqueRoomID() const;
    int GetNextDestination();
    double GetDistanceToNextTarget() const;

    /**
     * Checks if between the last two calls of 'UpdateRoom(int, int)' the Suboom of the pedestrian
     * has changed. Changing the SubRoom means, the pedestrian has moved to a different SubRoom or Room.
     * @return pedestrians has moved to a different SubRoom.
     */
    bool ChangedSubRoom() const;

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

    /**
      * @return true if the time spent in jam exceed the patience time
      * @see GetPatienceTime
      */
    bool IsFeelingLikeInJam() const;

    /**
      * Set/Get the patience time.
      * Higher value will cause the agent to almost never changes its current path.
      * Small values will increase the frequency of looking for alternative
      */
    double GetPatienceTime() const;

    /**
      * Set/Get the patience time.
      * Higher value will cause the agent to almost never changes its current path.
      * Small values will increase the frequency of looking for alternative
      */
    void SetPatienceTime(double patienceTime);

    /**
      * Set/Get the spotlight value. If true,
      * this pedestrians will be coloured and all other grey out.
      * @param spotlight true for enabling, false for disabling
      */
    [[maybe_unused]] void SetSpotlight(bool spotlight);

    /**
      * Set/Get the spotlight value. If true,
      * this pedestrians will be coloured and all other grey out.
      * @param spotlight true for enabling, false for disabling
      */
    bool GetSpotlight() const;

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
      * Set/Get the risk tolerance of a pedestrians.
      * The value should be in the interval [0 1].
      * It will be truncated accordingly if not in that interval.
      */
    void SetRiskTolerance(double tol);

    /**
      * Set/Get the risk tolerance of a pedestrians.
      * The value should be in the interval [0 1].
      * It will be truncated accordingly if not in that interval.
      */
    double GetRiskTolerance() const;

    /**
      * return the pedestrian color used for visualiation.
      * Default mode is coded by velocity.
      * @return a value in [-1 255]
      */
    int GetColor() const;

    void ResetTimeInJam();
    void UpdateTimeInJam();
    void UpdateReroutingTime();
    void RerouteIn(double time);
    bool IsReadyForRerouting() const;

    /**
      * clear the parameter related to the re routing
      */
    void ResetRerouting();

    /**
      * Set/Get the time period for which the data of the pedestrian should be kept.
      * The results are used by the quickest path router
      */
    void SetRecordingTime(double timeInSec);

    /**
      * Set/Get the time period for which the data of the pedestrian should be kept
      * The results are used by the quickest path router
      */
    double GetRecordingTime() const;

    /**
      * @return the average velocity over the recording period
      */
    double GetMeanVelOverRecTime() const;

    int GetGroup() const;
    void SetGroup(int group);

    static double GetGlobalTime();
    static void SetGlobalTime(double time);

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

    void SetWalkingSpeed(std::shared_ptr<WalkingSpeed> walkingSpeed);

    void SetTox(std::shared_ptr<ToxicityAnalysis> toxicityAnalysis);

    void ConductToxicityAnalysis();

    void EnterGoal();

    void LeaveGoal();

    int GetLastGoalID() const;

    bool IsInsideWaitingAreaWaiting() const;

    const Point & GetWaitingPos() const;

    void SetWaitingPos(const Point & waitingPos);

    bool IsWaiting() const;

    void StartWaiting();
    void EndWaiting();

    /**
     * Updates the room information the pedestrian is located. The information the pedestrian was
     * in before is saved in \m _oldRoomID, _oldSubRoomID
     * @param roomID
     * @param subRoomID
     * @param
     */
    void UpdateRoom(int roomID, int subRoomID);

    /**
     * Checks if between the last two calls of 'UpdateRoom(int, int)' the Room of the pedestrian
     * has changed.
     * @return pedestrians has moved to a different Room.
     */
    bool ChangedRoom() const;

    /**
     * Returns the ID of the room before the last call of 'UpdateRoom(int, int)'
     * @return ID of the former room the pedestrian was in
     */
    int GetOldRoomID() const;

    /**
     * Returns the ID of the subroom before the last call of 'UpdateRoom(int, int)'
     * @return ID of the former room the pedestrian was in
     */
    int GetOldSubRoomID() const;

    const std::queue<Point> & GetLastPositions() const;

    Point GetLastPosition() const;
};

std::ostream & operator<<(std::ostream & out, const Pedestrian & pedestrian);
