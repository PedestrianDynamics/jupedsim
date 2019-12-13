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

#include <map>
#include <queue>
#include <set>
#include <ctime>
#include <vector>

class Building;

class NavLine;

class Router;

class Knowledge;

class WalkingSpeed;

class Pedestrian {
private:
    /**
     * Unique id of pedestrian, starting at 1.
     */
    int _id;

    /**
     * Current desired exit index.
     */
    int _exitIndex;

    /**
     * ID of group to which the pedestrian belongs.
     */
    int _group;

    /**
     * Desired final goal, -1 for outside.
     * TODO rename, also intermediate goals are saved here
     */
    int _desiredFinalDestination;

    /**
     * Height of pedestrian. Never used.
     */
    double _height;

    /**
     * Age of pedestrian. Never used.
     */
    double _age;

    /**
     * Time till the pedestrian starts its initial motion.
     */
    double _premovement = 0;

    /**
     * Risk tolerance of the pedestrian, defines the probabilt that the ped will go through smoked areas.
     * @todo only used in smoke router, move it there
     */
    double _riskTolerance = 0;

    /**
     * Gender of ped. Never used.
     * @todo better as enum
     */
    std::string _gender;

    //gcfm specific parameters
    /**
     * Mass of pedestrian?
     * @todo unit
     */
    double _mass; // Mass: 1

    /**
     * Reaction time?
     * @todo unit
     */
    double _tau; // Reaction time: 0.5

    /**
     *
     */
    double _T; //TODO remove only used in Velocity Model // OV function

    /**
     * Time step size
     * @todo why needed in ped?
     */
    double _deltaT;

    /**
     * Shape of pedestrian, includes also the coordinates.
     * @todo may be move to model and keep only position
     */
    JEllipse _ellipse;

    /**
     * Current velocity of ped. Point used as vector here.
     */
    Point _V0;

    /**
     * Velocity on upward stairs.
     */
    double _V0UpStairs;

    /**
     * Velocity on downward stairs.
     */
    double _V0DownStairs;
    double _EscalatorUpStairs;
    double _EscalatorDownStairs;
    double _V0IdleEscalatorUpStairs;
    double _V0IdleEscalatorDownStairs;

    //location parameters
    std::string _roomCaption; //TODO remove
    int _roomID;
    int _subRoomID;
    int _subRoomUID;
    int _oldRoomID;    //TODO remove
    int _oldSubRoomID; //TODO remove
    Point _lastE0;

    NavLine *_navLine;            // current exit line

    //TODO check why this is here, used by global and quickest router
    std::map<int, int> _mentalMap; // map the actual room to a destination

    Point _lastPosition; //TODO remove
    int _lastCellPosition;  //TODO remove

    ///state of doors with time stamps
    std::map<int, Knowledge> _knownDoors; //TODO remove only used in eventmanager

    /// distance to nearest obstacle that blocks the sight of ped.
    double _distToBlockade; // TODO remove only used in walking strategy 4
    //routing parameters
    /// new orientation after 10 seconds
    double _reroutingThreshold; //TODO remove
    /// a new orientation starts after this time
    double _timeBeforeRerouting; //TODO move to quickest router
    /// actual time im Jam
    double _timeInJam; //TODO move to quickest router
    /// time after which the ped feels to be in jam
    double _patienceTime; //TODO move to quickest router
    /// data from the last <_recordingTime> seconds will be kept
    double _recordingTime; // TODO seems to be only used in velocity model
    /// store the last positions
    std::queue<Point> _lastPositions; // TODO seems to be only used in velocity model
    /// store the last velocities
    std::queue<Point> _lastVelocites; // TODO remove, never read
    /// routing strategy followed
    RoutingStrategy _routingStrategy; // TODO discuss removal as only used as output

    //TODO meaning? Related to smooth rotating
    int _newOrientationDelay; //2 seconds, in steps

    /// necessary for smooth turning at sharp bend
    double _updateRate; //TODO remove, only set but never read
    double _turninAngle; //TODO remove, only set but never read
    bool _reroutingEnabled; //TODO remove, seems to be only used in global router
    bool _tmpFirstOrientation; //TODO remove, only set but never read // possibility to get rid of this variable
    bool _newOrientationFlag;  //TODO remove, this is used in the DirectionGeneral::GetTarget() strategy 4

    // the current time in the simulation
    static double _globalTime; //TODO remove, belongs more to building
    static double _minPremovementTime;
    static AgentColorMode _colorMode;
    bool _spotlight;

    /// the router responsible for this pedestrian
    Router *_router; //TODO remove, change responsibilities
    /// a pointer to the complete building
    Building *_building; //TODO remove

    static int _agentsCreated;

    double _FED_In; //TODO move to router
    double _FED_Heat; //TODO move to router

    std::shared_ptr<WalkingSpeed> _WalkingSpeed = nullptr;
    std::shared_ptr<ToxicityAnalysis> _ToxicityAnalysis = nullptr;

    int _lastGoalID = -1;
    bool _insideGoal = false;
    bool _waiting = false;
    Point _waitingPos;

public:
    // constructors
    Pedestrian();

    explicit Pedestrian(const StartDistribution &agentsParameters, Building &building);

    virtual ~Pedestrian();

    // Setter-Funktionen
    void SetID(int i);

    //TODO remove caption
    void SetRoomID(int i, std::string roomCaption);

    void SetSubRoomID(int i);

    void SetSubRoomUID(int i);

    void SetMass(double m);

    void SetTau(double tau);

    //TODO does it make sense to set the shape by the model? Ped Factory? Using two models would lead to problems
    void SetEllipse(const JEllipse &e);

    double GetT() const;

    void SetT(double T);

    //TODO: merge this two functions
    void SetExitIndex(int i);

    void SetExitLine(const NavLine *l);

    //TODO remove never used
    void SetFEDIn(double FED_In);

    //TODO remove never used
    double GetFEDIn();

    //TODO remove never used
    void SetFEDHeat(double FED_Heat);

    //TODO remove never used
    double GetFEDHeat();

    //TODO remove if _dt is not needed in ped
    void Setdt(double dt);

    //TODO remove if _dt is not needed in ped
    double Getdt();

    //TODO remove only used in walking strat 4
    void SetDistToBlockade(double dist);

    //TODO remove, never used
    double GetDistToBlockade();

    // Eigenschaften der Ellipse
    void SetPos(const Point &pos, bool initial = false); // setzt x und y-Koordinaten

    //TODO remove, never used
    void SetCellPos(int cp);

    void SetV(const Point &v); // setzt x und y-Koordinaten der Geschwindigkeit

    void SetV0Norm(
            double v0,
            double v0UpStairs,
            double v0DownStairs,
            double escalatorUp,
            double escalatorDown,
            double v0IdleEscalatorUp,
            double v0IdleEscalatorDown);

    void SetSmoothTurning(); // activate the smooth turning with a delay of 2 sec

    //TODO merge SetPos, SetV, SetPhiPed in one function, especially since SetPhiPed does not get any parameter
    void SetPhiPed();

    void SetFinalDestination(int UID);

    //TODO remove if router gets removed
    void SetRouter(Router *router);

    // Getter-Funktionen
    int GetID() const;

    int GetRoomID() const;

    int GetSubRoomID() const;

    int GetSubRoomUID() const;

    double GetMass() const;

    double GetTau() const;

    const JEllipse &GetEllipse() const;

    int GetExitIndex() const;

    Router *GetRouter() const;

    NavLine *GetExitLine() const;

    double GetUpdateRate() const; //TODO remove, never called
    Point GetLastE0() const;

    void SetLastE0(Point E0);

    // Eigenschaften der Ellipse
    const Point &GetPos() const;

    int GetCellPos() const; //TODO remove, never called
    const Point &GetV() const;

    const Point &GetV0() const;

    const Point &GetV0(const Point &target);

    void InitV0(const Point &target);

    /**
      * the desired speed is the projection of the speed on the horizontal plane.
      * @return the norm of the desired speed.
      */
    double GetV0Norm() const;

    ///get axis in the walking direction
    double GetLargerAxis() const; //TODO maybe remove only in walking strat 4 and output
    ///get axis in the shoulder direction = orthogonal to the walking direction
    double GetSmallerAxis() const;

    //TODO remove, only in velocity model used to delete peds (peds should not be deleted there anyways...)
    double GetTimeInJam() const;


    int GetFinalDestination() const;

    //TODO remove if not used by AI Router
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
    //TODO remove if not used by AI Router
    std::map<int, Knowledge> &GetKnownledge();

    /**
      * For convenience
      * @return a string representation of the knowledge
      */
    //TODO remove if not used by AI Router
    const std::string GetKnowledgeAsString() const;

    /**
      * clear all information related to the knowledge about closed doors
      */
    //TODO remove if not used by AI Router
    void ClearKnowledge();

    RoutingStrategy GetRoutingStrategy() const;

    //TODO remove, one get the unique ID via building, here magic numbers are used
    int GetUniqueRoomID() const;

    int GetNextDestination();

    //TODO remove, not responsibility of ped
    double GetDistanceToNextTarget() const;

    //TODO remove, never used
    double GetDisTanceToPreviousTarget() const;

    //TODO move to walking strat 4
    void SetNewOrientationFlag(bool flag);

    //TODO move to walking strat 4
    bool GetNewOrientationFlag();

    //TODO remove, never used
    bool GetNewEventFlag();

    //TODO remove, never used
    void SetNewEventFlag(bool flag);

    //TODO remove, never used
    bool ChangedSubRoom();

    //TODO remove, never used
    void RecordActualPosition();

    //TODO remove, never used
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
    //TODO remove when improving the router interface, does not make any sense here
    int FindRoute();

    ///write the pedestrian path (room and exit taken ) to file
    //TODO remove, never used
    void WritePath(std::ofstream &file, Building *building = nullptr);

    ///write the pedestrian path (room and exit taken ) to file
    /// in the format room1:exit1>room2:exit2
    //TODO remove, there seems to be an error in it anyways
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
    //TODO remove, never used
    bool Observe(double maxObservationTime = -1);

    /**
      * @return true, if reference pedestrian have been selected
      * and the observation process has started
      */
    //TODO remove, never used
    bool IsObserving();

    /**
      * return the observation data in an array
      *
      * @param exitID, the concerned exit
      * @param data, a float array to store the data
      */
    //TODO remove, never used
    void GetObservationData(int exitID, float *data);

    /**
      * @return true if the time spent in jam exceed the patience time
      * @see GetPatienceTime
      */
    //TODO move to quickest router
    bool IsFeelingLikeInJam();

    /**
      * Set/Get the patience time.
      * Higher value will cause the agent to almost never changes its current path.
      * Small values will increase the frequency of looking for alternative
      */
    //TODO remove, only in velocity model used to delete peds (peds should not be deleted there anyways...)
    double GetPatienceTime() const;

    /**
      * Set/Get the patience time.
      * Higher value will cause the agent to almost never changes its current path.
      * Small values will increase the frequency of looking for alternative
      */
    //TODO remove, only in velocity model used to delete peds (peds should not be deleted there anyways...)
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
    //TODO move to smoke router
    void SetRiskTolerance(double tol);

    /**
      * Set/Get the risk tolerance of a pedestrians.
      * The value should be in the interval [0 1].
      * It will be truncated accordingly if not in that interval.
      */
    //TODO move to smoke router
    double GetRiskTolerance() const;

    /**
      * return the pedestrian color used for visualiation.
      * Default mode is coded by velocity.
      * @return a value in [-1 255]
      */
    //TODO remove when we have updated visualisation
    int GetColor() const;


    void ResetTimeInJam();

    void UpdateTimeInJam();

    //TODO remove never used
    void UpdateJamData();

    //TODO move to quickest router
    void UpdateReroutingTime();

    //TODO remove never used
    double GetReroutingTime();

    //TODO move to quickest router, and check for magic number in global shortest
    void RerouteIn(double time);

    //TODO move to quickest router
    bool IsReadyForRerouting();
    /**
      * clear the parameter related to the re routing
      */
    //TODO move to quickest router
    void ResetRerouting();

    /**
      * Set/Get the time period for which the data of the pedestrian should be kept.
      * The results are used by the quickest path router
      */
    //TODO remove never used
    void SetRecordingTime(double timeInSec);

    /**
      * Set/Get the time period for which the data of the pedestrian should be kept
      * The results are used by the quickest path router
      */
    //TODO remove never used
    double GetRecordingTime() const;

    /**
      * @return the average velocity over the recording period
      */
    //TODO move to velocity model
    double GetMeanVelOverRecTime() const;

    //TODO remove then age is removed
    double GetAge() const;

    //TODO remove then age is removed
    void SetAge(double age);

    //TODO remove then gender is removed
    std::string GetGender() const;

    //TODO remove then gender is removed
    void SetGender(std::string gender);

    //TODO remove when height is removed
    double GetHeight() const;

    //TODO remove when height is removed
    void SetHeight(double height);

    int GetGroup() const;

    void SetGroup(int group);

    //TODO move to building
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
    //TODO remove when removing building
    const Building *GetBuilding();

    /**
      * Set/Get the Building object
      */
    //TODO remove when removing building
    void SetBuilding(Building *building);

    void SetWalkingSpeed(std::shared_ptr<WalkingSpeed> walkingSpeed);

    //TODO remove only declaration
    void
    WalkingUpstairs(double c, SubRoom *sub, double ped_elevation, double &walking_speed) const;

    //TODO remove only declaration
    void
    WalkingDownstairs(double &walking_speed, double c, SubRoom *sub, double ped_elevation) const;

    //TODO remove
    void SetTox(std::shared_ptr<ToxicityAnalysis> toxicityAnalysis);

    //TODO remove only declaration
    void ConductToxicityAnalysis();

    bool Relocate(std::function<void(const Pedestrian &)> flowupdater);

    //TODO remove only declaration
    const std::shared_ptr<ToxicityAnalysis> &getToxicityAnalysis();


    //TODO remove, only used by waiting strategies
    int GetLastGoalID() const;

    //TODO check if this really belongs here, more likely move functionality to goal manager
    void EnterGoal();

    void LeaveGoal();

    bool IsInsideGoal() const;

    bool IsInsideWaitingAreaWaiting() const;

    const Point &GetWaitingPos() const;

    void SetWaitingPos(const Point &waitingPos);

    bool IsWaiting() const;

    void StartWaiting();

    void EndWaiting();

    //TODO remove, never used
    bool IsOutside();
};
