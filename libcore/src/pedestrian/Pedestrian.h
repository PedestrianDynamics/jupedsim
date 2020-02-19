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
#include <time.h>
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
    double _height;
    double _age;
    double _premovement   = 0;
    double _riskTolerance = 0;
    std::string _gender;

    //gcfm specific parameters
    double _mass;      // Mass: 1
    double _tau;       // Reaction time: 0.5
    double _T;         // OV function
    double _deltaT;    // step size
    JEllipse _ellipse; // the shape of this pedestrian
    Point _V0;         //vector V0


    double _V0UpStairs;
    double _V0DownStairs;
    double _EscalatorUpStairs;
    double _EscalatorDownStairs;
    double _V0IdleEscalatorUpStairs;
    double _V0IdleEscalatorDownStairs;
    //location parameters
    std::string _roomCaption;
    int _roomID;
    int _subRoomID;
    int _subRoomUID;
    int _oldRoomID;
    int _oldSubRoomID;
    Point _lastE0;

    NavLine * _navLine;            // current exit line
    std::map<int, int> _mentalMap; // map the actual room to a destination
    std::vector<int> _destHistory;
    std::vector<int> _trip;

    Point _lastPosition;
    int _lastCellPosition;

    ///state of doors with time stamps
    std::map<int, Knowledge> _knownDoors;

    /// distance to nearest obstacle that blocks the sight of ped.
    double _distToBlockade;
    //routing parameters
    /// new orientation after 10 seconds
    double _reroutingThreshold;
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

    /// necessary for smooth turning at sharp bend
    double _updateRate;
    double _turninAngle;
    bool _reroutingEnabled;
    bool _tmpFirstOrientation; // possibility to get rid of this variable
    bool _newOrientationFlag;  //this is used in the DirectionGeneral::GetTarget()

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

    double _FED_In;
    double _FED_Heat;

    std::shared_ptr<WalkingSpeed> _WalkingSpeed         = nullptr;
    std::shared_ptr<ToxicityAnalysis> _ToxicityAnalysis = nullptr;

    int _lastGoalID  = -1;
    bool _insideGoal = false;
    bool _waiting    = false;
    Point _waitingPos;

public:
    // public member
    int _ticksInThisRoom;

    // constructors
    Pedestrian();
    explicit Pedestrian(const StartDistribution & agentsParameters, Building & building);
    virtual ~Pedestrian();

    // Setter-Funktionen
    void SetID(int i);
    void SetRoomID(int i, std::string roomCaption);
    void SetSubRoomID(int i);
    void SetSubRoomUID(int i);
    void SetMass(double m);
    void SetTau(double tau);
    void SetEllipse(const JEllipse & e);

    double GetT() const;
    void SetT(double T);
    //TODO: merge this two functions
    void SetExitIndex(int i);
    void SetExitLine(const NavLine * l);

    void SetFEDIn(double FED_In);
    double GetFEDIn();

    void SetFEDHeat(double FED_Heat);
    double GetFEDHeat();

    void Setdt(double dt);
    double Getdt();

    // Eigenschaften der Ellipse
    void SetPos(const Point & pos, bool initial = false); // setzt x und y-Koordinaten
    void SetCellPos(int cp);
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
    void SetTrip(const std::vector<int> & trip);
    void SetRouter(Router * router);

    // Getter-Funktionen
    const std::vector<int> & GetTrip() const;
    int GetID() const;
    int GetRoomID() const;
    int GetSubRoomID() const;
    int GetSubRoomUID() const;
    double GetMass() const;
    double GetTau() const;
    const JEllipse & GetEllipse() const;
    int GetExitIndex() const;
    Router * GetRouter() const;
    NavLine * GetExitLine() const;
    double GetUpdateRate() const;
    Point GetLastE0() const;
    void SetLastE0(Point E0);
    // Eigenschaften der Ellipse
    const Point & GetPos() const;
    int GetCellPos() const;
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
    void ClearMentalMap(); //erase the peds memory

    /**
      * Update the knowledge of the pedestrian
      * @param door
      * @param ttime
      * @param state
      * @param quality
      */
    void AddKnownClosedDoor(int door, double ttime, bool state, double quality, double latency);

    /***
      * @return the knowledge of the pedstrian
      */
    std::map<int, Knowledge> & GetKnownledge();

    /**
      * @return all previous destinations used by this pedestrian
      */
    const std::vector<int> & GetLastDestinations() const;

    /**
      * For convenience
      * @return a string representation of the knowledge
      */
    const std::string GetKnowledgeAsString() const;

    /**
      * clear all information related to the knowledge about closed doors
      */
    void ClearKnowledge();

    RoutingStrategy GetRoutingStrategy() const;
    int GetUniqueRoomID() const;
    int GetNextDestination();
    int GetLastDestination();
    double GetDistanceToNextTarget() const;
    double GetDisTanceToPreviousTarget() const;
    bool GetNewEventFlag();
    void SetNewEventFlag(bool flag);
    bool ChangedSubRoom();
    void RecordActualPosition();
    double GetDistanceSinceLastRecord();

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
    void WritePath(std::ofstream & file, Building * building = nullptr);

    ///write the pedestrian path (room and exit taken ) to file
    /// in the format room1:exit1>room2:exit2
    std::string GetPath();

    /**
      * Dump the parameters of this pedestrians.
      * @param ID, the id of the pedestrian
      * @param pa, the parameter to display (0 for all parameters)
      */
    void Dump(int ID, int pa = 0) const;

    /**
      * observe the reference pedestrians and collect some data, e.g distance to exit
      * and average/maximal velocities
      *
      * @param maxObservationTime in sec.
      * @return false, if the observation time is over and the observation data can be retrieved
      */
    bool Observe(double maxObservationTime = -1);

    /**
      * @return true, if reference pedestrian have been selected
      * and the observation process has started
      */
    bool IsObserving();

    /**
      * return the observation data in an array
      *
      * @param exitID, the concerned exit
      * @param data, a float array to store the data
      */
    void GetObservationData(int exitID, float * data);

    /**
      * @return true if the time spent in jam exceed the patience time
      * @see GetPatienceTime
      */
    bool IsFeelingLikeInJam();

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
    void SetSpotlight(bool spotlight);

    /**
      * Set/Get the spotlight value. If true,
      * this pedestrians will be coloured and all other grey out.
      * @param spotlight true for enabling, false for disabling
      */
    bool GetSpotlight();

    /***
      * Set/Get the time after which this pedestrian will start taking actions.
      */
    void SetPremovementTime(double time);

    /***
      * Set/Get the time after which this pedestrian will start taking actions.
      */
    double GetPremovementTime();

    /***
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
    void UpdateJamData();
    void UpdateReroutingTime();
    double GetReroutingTime();
    void RerouteIn(double time);
    bool IsReadyForRerouting();

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

    double GetAge() const;
    void SetAge(double age);
    std::string GetGender() const;
    void SetGender(std::string gender);
    double GetHeight() const;
    void SetHeight(double height);
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

    void
    WalkingUpstairs(double c, SubRoom * sub, double ped_elevation, double & walking_speed) const;
    void
    WalkingDownstairs(double & walking_speed, double c, SubRoom * sub, double ped_elevation) const;


    void SetTox(std::shared_ptr<ToxicityAnalysis> toxicityAnalysis);

    void ConductToxicityAnalysis();

    bool Relocate(std::function<void(const Pedestrian &)> flowupdater);

    const std::shared_ptr<ToxicityAnalysis> & getToxicityAnalysis();

    void EnterGoal();

    void LeaveGoal();

    int GetLastGoalID() const;

    bool IsInsideGoal() const;

    bool IsInsideWaitingAreaWaiting() const;

    const Point & GetWaitingPos() const;

    void SetWaitingPos(const Point & waitingPos);

    bool IsWaiting() const;

    void StartWaiting();
    void EndWaiting();

    bool IsOutside();
};
